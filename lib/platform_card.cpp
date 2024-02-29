/**
 * Author: Pascal Henry Bauer <pascal.bauer@rwth-aachen.de>
 * Based on the work of: Steffen Vogel <post@steffenvogel.de> and Daniel Krebs
 *<github@daniel-krebs.net>
 *
 * SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power
 *Systems, EONERC SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/
#include "villas/fpga/ips/dma.hpp"
#include <jansson.h>
#include <string>
#include <villas/fpga/platform_card.hpp>

#include <iostream>
#include <memory>
#include <villas/exceptions.hpp>
#include <villas/fpga/core.hpp>
#include <villas/fpga/node.hpp>
#include <villas/utils.hpp>
#include <villas/fpga/card_parser.hpp>
#include <villas/fpga/ip_loader.hpp>

using namespace villas;
using namespace villas::fpga;


PlatformCard::PlatformCard(
    std::shared_ptr<kernel::vfio::Container> vfioContainer,
	std::vector<std::string> device_names
	)
{
        this->vfioContainer = vfioContainer;
        this->logger = villas::logging.get("PlatformCard");

		// Create VFIO Group
        const int IOMMU_GROUP = 2; //TODO: find Group
        auto group = std::make_shared<kernel::vfio::Group>(IOMMU_GROUP, true);
        vfioContainer->attachGroup(group);

		// Open VFIO Devices
		for(std::string device_name : device_names){
			auto vfioDevice = std::make_shared<kernel::vfio::Device>(
				device_name,
				group->getFileDescriptor());
			group->attachDevice(vfioDevice);
			this->devices.push_back(vfioDevice);
		}

		// Map all vfio devices in card to process
		std::map<std::shared_ptr<villas::kernel::vfio::Device>, const void*> mapped_memory;
		for (auto device : devices)
		{
			const void* mapping = device->regionMap(0);
			if (mapping == MAP_FAILED) {
				logger->error("Failed to mmap() device");
			}
			logger->debug("memory mapped: {}", device->getName());
			mapped_memory.insert({device, mapping});
		}

		// Create mappings from process space to vfio devices
		auto &mm = MemoryManager::get();
		size_t srcVertexId = mm.getProcessAddressSpace();
		for (auto pair : mapped_memory)
		{
			const size_t mem_size = pair.first->regionGetSize(0);
			size_t targetVertexId = mm.getOrCreateAddressSpace(pair.first->getName());
			mm.createMapping(reinterpret_cast<uintptr_t>(pair.second),
								0, mem_size, "process to vfio", srcVertexId, targetVertexId);
			logger->debug("create edge from process to {}", pair.first->getName());
		}
}

void PlatformCard::connectVFIOtoIPS()
{
	auto &mm = MemoryManager::get();
	const size_t ip_mem_size = 65536;
	size_t srcVertexId = mm.getOrCreateAddressSpace("a0000000.dma");
	size_t targetVertexId = mm.getOrCreateAddressSpace("axi_dma_0/Reg");
	mm.createMapping(0, 0, ip_mem_size, "vfio to ip", srcVertexId,
					targetVertexId);

	// Switch
	srcVertexId = mm.getOrCreateAddressSpace("a0010000.axis_switch");
	targetVertexId = mm.getOrCreateAddressSpace("axis_interconnect_0_xbar/Reg");
	mm.createMapping(0, 0, ip_mem_size, "vfio to ip", srcVertexId,
					targetVertexId);

	for(auto device : devices)
	{
		std::string addr;
		std::string name;

		std::istringstream iss(device->getName());
		std::getline(iss, addr, '.');
		std::getline(iss, name, '.');
	}
}

bool PlatformCard::mapMemoryBlock(const std::shared_ptr<MemoryBlock> block) {
  if (not vfioContainer->isIommuEnabled()) {
    logger->warn("VFIO mapping not supported without IOMMU");
    return false;
  }

  auto &mm = MemoryManager::get();
  const auto &addrSpaceId = block->getAddrSpaceId();

  if (memoryBlocksMapped.find(addrSpaceId) != memoryBlocksMapped.end())
    // Block already mapped
    return true;
  else
    logger->debug("Create VFIO-Platform mapping for {}", addrSpaceId);

  auto translationFromProcess = mm.getTranslationFromProcess(addrSpaceId);
  uintptr_t processBaseAddr = translationFromProcess.getLocalAddr(0);
  uintptr_t iovaAddr =
      vfioContainer->memoryMap(processBaseAddr, UINTPTR_MAX, block->getSize());

  if (iovaAddr == UINTPTR_MAX) {
    logger->error("Cannot map memory at {:#x} of size {:#x}", processBaseAddr,
                  block->getSize());
    return false;
  }

  mm.createMapping(iovaAddr, 0, block->getSize(), "VFIO-D2H",
                   this->addrSpaceIdDeviceToHost, addrSpaceId);

	auto space = mm.findAddressSpace("zynq_ultra_ps_e_0/HPC1_DDR_LOW");
	mm.createMapping(iovaAddr, 0, block->getSize(), "VFIO-D2H",
	space, addrSpaceId);


  // Remember that this block has already been mapped for later
  memoryBlocksMapped.insert({addrSpaceId, block});

  return true;
}

std::list<std::shared_ptr<Card>>
PlatformCardFactory::make(json_t *json,
                          std::shared_ptr<kernel::vfio::Container> vc,
						  const std::filesystem::path& searchPath)
{
		std::list<std::shared_ptr<Card>> cards;
		auto logger = villas::logging.get("PlatformCard");

		const char *card_name;
		json_t *json_card;
		json_object_foreach(json, card_name, json_card) {	
			logger->info("Found config for FPGA card {}", card_name);

			// Parse and create card
			CardParser parser(json_card);
			
			auto card = std::make_unique<PlatformCard>(vc, parser.device_names);
			card->name = std::string(card_name);
			card->affinity = parser.affinity;
			card->doReset = parser.do_reset != 0;
			card->polling = (parser.polling != 0);

			// if (not card->init()) {
			// 	logger->warn("Cannot start FPGA card {}", card_name);
			// 	continue;
			// }

			//IpLoader ipLoader(parser.json_ips, searchPath);

			//Load IPs from a separate json file
			if (!json_is_string(parser.json_ips)) {
			logger->debug("FPGA IP cores config item is not a string.");
			throw ConfigError(parser.json_ips, "node-config-fpga-ips",
								"FPGA IP cores config item is not a string.");
			}
			if (!searchPath.empty()) {
			std::filesystem::path json_ips_path =
				searchPath / json_string_value(parser.json_ips);
			logger->debug("searching for FPGA IP cors config at {}", json_ips_path);
			parser.json_ips = json_load_file(json_ips_path.c_str(), 0, nullptr);
			}
			if (parser.json_ips == nullptr) {
			parser.json_ips =
				json_load_file(json_string_value(parser.json_ips), 0, nullptr);
			logger->debug("searching for FPGA IP cors config at {}",
							json_string_value(parser.json_ips));
			if (parser.json_ips == nullptr) {
				throw ConfigError(parser.json_ips, "node-config-fpga-ips",
								"Failed to find FPGA IP cores config");
			}
			}

			if (not json_is_object(parser.json_ips))
			throw ConfigError(parser.json_ips, "node-config-fpga-ips",
								"FPGA IP core list must be an object!");

			card->ips = ip::CoreFactory::make(card.get(), parser.json_ips);
			if (card->ips.empty())
				throw ConfigError(parser.json_ips, "node-config-fpga-ips", "Cannot initialize IPs of FPGA card {}", card_name);

			cards.push_back(std::move(card));
		}
	return cards;
}
