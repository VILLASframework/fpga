/**
 * Author: Pascal Henry Bauer <pascal.bauer@rwth-aachen.de>
 * Based on the work of: Steffen Vogel <post@steffenvogel.de> and Daniel Krebs
 *<github@daniel-krebs.net>
 *
 * SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power
 *Systems, EONERC SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#include <villas/fpga/platform_card.hpp>

#include <iostream>
#include <memory>
#include <villas/exceptions.hpp>
#include <villas/fpga/core.hpp>
#include <villas/fpga/node.hpp>
#include <villas/utils.hpp>

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
  //! VFIO CONTAINER REPLACED BY DEVICES

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

std::list<std::shared_ptr<PlatformCard> >
PlatformCardFactory::make(json_t *json,
                          std::shared_ptr<kernel::vfio::Container> vc,
						  const std::filesystem::path& searchPath)			  
{
    std::list<std::shared_ptr<PlatformCard>> cards;
	auto logger = villas::logging.get("PlatformCard");

	const char *card_name;
	json_t *json_card;
	json_object_foreach(json, card_name, json_card) {
		logger->info("Found config for FPGA card {}", card_name);

		json_t*     json_ips = nullptr;
		json_t*     json_paths = nullptr;
		const char* pci_slot = nullptr;
		const char* pci_id   = nullptr;
		int         do_reset = 0;
		int         affinity = 0;
		int 	    polling  = 0;

		json_error_t err;
		int ret = json_unpack_ex(json_card, &err, 0, "{ s: o, s?: i, s?: b, s?: s, s?: s, s?: b, s?: o }",
			"ips",		&json_ips,
			"affinity", 	&affinity,
			"do_reset", 	&do_reset,
			"slot", 	&pci_slot,
		    "id", 		&pci_id,
			"polling", 	&polling,
			"paths", 	&json_paths);

		if (ret != 0)
			throw ConfigError(json_card, err, "", "Failed to parse card");

		//TODO: parse from json
		std::vector<std::string> device_names{"a0000000.dma", "a0010000.axis_switch"};

		auto card = std::make_unique<PlatformCard>(vc, device_names);

		// Populate generic properties
		card->name = std::string(card_name);
		card->vfioContainer = vc;
		card->affinity = affinity;
		card->doReset = do_reset != 0;
		card->polling = (polling != 0);

		// if (not card->init()) {
		// 	logger->warn("Cannot start FPGA card {}", card_name);
		// 	continue;
		// }

		// Load IPs from a separate json file
		if (!json_is_string(json_ips)) {
			logger->debug("FPGA IP cores config item is not a string.");
			throw ConfigError(json_ips, "node-config-fpga-ips", "FPGA IP cores config item is not a string.");
		}
		if (!searchPath.empty()) {
			std::filesystem::path json_ips_path = searchPath / json_string_value(json_ips);
			logger->debug("searching for FPGA IP cors config at {}", json_ips_path);
			json_ips = json_load_file(json_ips_path.c_str(), 0, nullptr);
		}
		if (json_ips == nullptr) {
			json_ips = json_load_file(json_string_value(json_ips), 0, nullptr);
			logger->debug("searching for FPGA IP cors config at {}", json_string_value(json_ips));
			if (json_ips == nullptr) {
				throw ConfigError(json_ips, "node-config-fpga-ips", "Failed to find FPGA IP cores config");
			}
		}

		if (not json_is_object(json_ips))
			throw ConfigError(json_ips, "node-config-fpga-ips", "FPGA IP core list must be an object!");

		card->ips = ip::CoreFactory::make(card.get(), json_ips);
		if (card->ips.empty())
			throw ConfigError(json_ips, "node-config-fpga-ips", "Cannot initialize IPs of FPGA card {}", card_name);

		// if (not card->check())
		// 	throw RuntimeError("Checking of FPGA card {} failed", card_name);

		// // Additional static paths for AXI-Steram switch
		// if (json_paths != nullptr) {
		// 	if (not json_is_array(json_paths))
		// 		throw ConfigError(json_paths, err, "", "Switch path configuration must be an array");

		// 	size_t i;
		// 	json_t *json_path;
		// 	json_array_foreach(json_paths, i, json_path) {
		// 		const char *from, *to;
		// 		int reverse = 0;

		// 		ret = json_unpack_ex(json_path, &err, 0, "{ s: s, s: s, s?: b }",
		// 			"from", &from,
		// 			"to", &to,
		// 			"reverse", &reverse
		// 		);
		// 		if (ret != 0)
		// 			throw ConfigError(json_path, err, "", "Cannot parse switch path config");

		// 		auto masterIpCore = card->lookupIp(from);
		// 		if (!masterIpCore)
		// 			throw ConfigError(json_path, "", "Unknown IP {}", from);

		// 		auto slaveIpCore = card->lookupIp(to);
		// 		if (!slaveIpCore)
		// 			throw ConfigError(json_path, "", "Unknown IP {}", to);

		// 		auto masterIpNode = std::dynamic_pointer_cast<ip::Node>(masterIpCore);
		// 		if (!masterIpNode)
		// 			throw ConfigError(json_path, "", "IP {} is not a streaming node", from);

		// 		auto slaveIpNode = std::dynamic_pointer_cast<ip::Node>(slaveIpCore);
		// 		if (!slaveIpNode)
		// 			throw ConfigError(json_path, "", "IP {} is not a streaming node", to);

		// 		if (not masterIpNode->connect(*slaveIpNode, reverse != 0))
		// 			throw ConfigError(json_path, "", "Failed to connect node {} to {}", from, to);
		// 	}
		// }

		cards.push_back(std::move(card));
	}
	return cards;
}
