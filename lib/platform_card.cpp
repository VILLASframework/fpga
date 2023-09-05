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
    std::shared_ptr<kernel::vfio::Container> vfioContainer)
{
        this->vfioContainer = vfioContainer;
        this->logger = villas::logging.get("PlatformCard");

		// VFIO Group
        const int IOMMU_GROUP = 2;
        auto group = std::make_shared<kernel::vfio::Group>(IOMMU_GROUP, true);
        vfioContainer->attachGroup(group);

		// VFIO Devices
		const char* DEVICES[] = {"a0000000.dma", "a0010000.axis_switch"};
		for(const char* DEVICE : DEVICES){
			auto vfioDevice = std::make_shared<kernel::vfio::Device>(
				DEVICE,
				group->getFileDescriptor());
			group->attachDevice(vfioDevice);
			this->devices.push_back(vfioDevice);
		}

		
		this->vfioDevice = nullptr; //TODO: refactor
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

		auto card = std::make_unique<PlatformCard>(vc);

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
