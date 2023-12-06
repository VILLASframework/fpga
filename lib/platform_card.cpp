/**
 * Author: Pascal Henry Bauer <pascal.bauer@rwth-aachen.de>
 * Based on the work of: Steffen Vogel <post@steffenvogel.de> and Daniel Krebs
 *<github@daniel-krebs.net>
 *
 * SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power
 *Systems, EONERC SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/
#include <jansson.h>
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
        const int IOMMU_GROUP = 2;
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

			IpLoader ipLoader(parser.json_ips, searchPath);

			card->ips = ip::CoreFactory::make(card.get(), parser.json_ips);
			if (card->ips.empty())
				throw ConfigError(parser.json_ips, "node-config-fpga-ips", "Cannot initialize IPs of FPGA card {}", card_name);

			cards.push_back(std::move(card));
		}
	return cards;
}
