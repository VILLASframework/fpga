/** 
 * Author: Pascal Henry Bauer <pascal.bauer@rwth-aachen.de>
 * Based on the work of: Steffen Vogel <post@steffenvogel.de> and Daniel Krebs <github@daniel-krebs.net>
 *
 * SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power Systems, EONERC
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#include <villas/fpga/platform_card.hpp>

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
            { IMPORTANT two implementations exist with overloaded parameters, PCI dev and some generic
                >>> investigation <<<
            }
            vfioDevice->pciEnable()
        }
        card->ips = ip::CoreFactory::make(card.get(), json_ips);
        + some messing with cores through getIp func
    */
   
        auto vfioContainer = std::make_shared<kernel::vfio::Container>();
        // DMESG: xilinx-vdma a0000000.dma: Adding to iommu group 2
        const int IOMMU_GROUP = 2;
        unique_ptr<kernel::vfio::Group> group = make_unique(IOMMU_GROUP, true);
        //vfioContainer -> attachGroup()
        //vfioContainer -> attachDevice("xilinx-vdma a0000000.dma", IOMMU_GROUP);


        return 0;
}

PlatformCard::PlatformCard(std::shared_ptr<kernel::vfio::Container> vfioContainer)
{
        this -> vfioContainer = vfioContainer;

        logger = villas::logging.get("PlatformCard");
        
}
