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
    /*  Factory
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
            vfioDevice->pciEnable()
        }
        card->ips = ip::CoreFactory::make(card.get(), json_ips);
        + some messing with cores through getIp func
    */
        auto vfioContainer = std::make_shared<kernel::vfio::Container>();

        /* PCI System (finds Device for vfio)
        
        */
        //const int IOMMU_GROUP = 2;

        

        //PlatformCard card = PlatformCard(vfioContainer);

        return 0;
}

PlatformCard::PlatformCard(std::shared_ptr<kernel::vfio::Container> vfioContainer)
{
        this -> vfioContainer = vfioContainer;

        logger = villas::logging.get("PlatformCard");
        
}
