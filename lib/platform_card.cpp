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
#include <jansson.h>
#include <villas/exceptions.hpp>
#include <villas/fpga/core.hpp>
#include <villas/fpga/node.hpp>

using namespace villas;
using namespace villas::fpga;

int PlatformCard::Test()
{
        /* To investigate
           -relation between vfio device and pci device
                - vfio dev uses pci device as member
                -> when is it accessed??

        */

        /*  Report of dependencies to produce a minimal card example

        Factory in pcieCard:
            card->name = std::string(card_name);
            card->vfioContainer = vc;
            card->affinity = affinity;
            card->doReset = do_reset != 0;
            card->polling = (polling != 0);
            card->pdev
            {
                pciDevice* lookupPci
                {
                    std::make_shared<Device>(id, slot)
                }
            }
            card -> init()
            {
                vfioDevice = vfioContainer->attachDevice(pdev);
                { IMPORTANT two implementations exist with overloaded
        parameters, PCI dev and some generic
                    >>> investigation <<<
                }
                vfioDevice->pciEnable()
            }
            card->ips = ip::CoreFactory::make(card.get(), json_ips);
            + some messing with cores through getIp func
        */

        // DMESG: xilinx-vdma a0000000.dma: Adding to iommu group 2

        // Parse FPGA configuration

        const std::string CONFIG_FILE_PATH = "config.json";
        FILE *f = fopen(CONFIG_FILE_PATH.c_str(), "r");

        if(!f)
                throw RuntimeError("Cannot open config file: {}",
                                   CONFIG_FILE_PATH);

        json_t *json = json_loadf(f, 0, nullptr);
        if(!json) {
                fclose(f);
                throw RuntimeError("Cannot parse JSON config");
        }

        fclose(f);

        json_t *fpgas = json_object_get(json, "fpgas");
        if(fpgas == nullptr) {
                exit(1);
        }

        auto vc = std::make_shared<kernel::vfio::Container>();
        PlatformCardFactory::make(json, vc);

        return 0;
}

PlatformCard::PlatformCard(
    std::shared_ptr<kernel::vfio::Container> vfioContainer)
{
        this->vfioContainer = vfioContainer;

        logger = villas::logging.get("PlatformCard");
}

void PlatformCard::connect()
{
        const int IOMMU_GROUP = 2;
        const char *DEV_NAME = "a0000000.dma";
        auto group = std::make_shared<kernel::vfio::Group>(IOMMU_GROUP, true);

        vfioContainer->attachGroup(group);

        auto device = std::make_shared<kernel::vfio::Device>(
            DEV_NAME,
            group->getFileDescriptor());
        group->attachDevice(device);
}

std::list<std::shared_ptr<PlatformCard> >
PlatformCardFactory::make(json_t *json,
                          std::shared_ptr<kernel::vfio::Container> vc)
{
        std::list<std::shared_ptr<PlatformCard> > cards;

        const char *card_name;
        json_t *json_card;
        json_object_foreach(json, card_name, json_card)
        {
                json_t *json_ips = nullptr;
                json_t *json_paths = nullptr;
                const char *pci_slot = nullptr;
                const char *pci_id = nullptr;
                int do_reset = 0;
                int affinity = 0;
                int polling = 0;

                json_error_t err;
                int ret = json_unpack_ex(
                    json_card,
                    &err,
                    0,
                    "{ s: o, s?: i, s?: b, s?: s, s?: s, s?: b, s?: o }",
                    "ips",
                    &json_ips,
                    "affinity",
                    &affinity,
                    "do_reset",
                    &do_reset,
                    "slot",
                    &pci_slot,
                    "id",
                    &pci_id,
                    "polling",
                    &polling,
                    "paths",
                    &json_paths);

                if(ret != 0) {
                        throw ConfigError(json_card,
                                          err,
                                          "",
                                          "Failed to parse card");
                }

                auto card = std::make_unique<PlatformCard>(vc);
                card->polling = (polling != 0);

                card->connect();

                // Load IPs from a separate json file
                if(json_is_string(json_ips)) {
                        auto json_ips_fn = json_string_value(json_ips);
                        json_ips = json_load_file(json_ips_fn, 0, nullptr);
                        if(json_ips == nullptr)
                                throw ConfigError(
                                    json_ips,
                                    "node-config-fpga-ips",
                                    "Failed to load FPGA IP cores from {}",
                                    json_ips_fn);
                }

                if(not json_is_object(json_ips))
                        throw ConfigError(
                            json_ips,
                            "node-config-fpga-ips",
                            "FPGA IP core list must be an object!");

                card->ips = ip::CoreFactory::make(card.get(), json_ips);
                if(card->ips.empty())
                        throw ConfigError(
                            json_ips,
                            "node-config-fpga-ips",
                            "Cannot initialize IPs of FPGA card {}",
                            card_name);

                // Additional static paths for AXI-Steram switch
                if(json_paths != nullptr) {
                        if(not json_is_array(json_paths))
                                throw ConfigError(json_paths,
                                                  err,
                                                  "",
                                                  "Switch path configuration "
                                                  "must be an array");

                        size_t i;
                        json_t *json_path;
                        json_array_foreach(json_paths, i, json_path)
                        {
                                const char *from, *to;
                                int reverse = 0;

                                ret = json_unpack_ex(json_path,
                                                     &err,
                                                     0,
                                                     "{ s: s, s: s, s?: b }",
                                                     "from",
                                                     &from,
                                                     "to",
                                                     &to,
                                                     "reverse",
                                                     &reverse);
                                if(ret != 0)
                                        throw ConfigError(
                                            json_path,
                                            err,
                                            "",
                                            "Cannot parse switch path config");

                                auto masterIpCore = card->lookupIp(from);
                                if(!masterIpCore)
                                        throw ConfigError(json_path,
                                                          "",
                                                          "Unknown IP {}",
                                                          from);

                                auto slaveIpCore = card->lookupIp(to);
                                if(!slaveIpCore)
                                        throw ConfigError(json_path,
                                                          "",
                                                          "Unknown IP {}",
                                                          to);

                                auto masterIpNode
                                    = std::dynamic_pointer_cast<ip::Node>(
                                        masterIpCore);
                                if(!masterIpNode)
                                        throw ConfigError(
                                            json_path,
                                            "",
                                            "IP {} is not a streaming node",
                                            from);

                                auto slaveIpNode
                                    = std::dynamic_pointer_cast<ip::Node>(
                                        slaveIpCore);
                                if(!slaveIpNode)
                                        throw ConfigError(
                                            json_path,
                                            "",
                                            "IP {} is not a streaming node",
                                            to);

                                if(not masterIpNode->connect(*slaveIpNode,
                                                             reverse != 0))
                                        throw ConfigError(
                                            json_path,
                                            "",
                                            "Failed to connect node {} to {}",
                                            from,
                                            to);
                        }
                }
        }

        return cards;
}
