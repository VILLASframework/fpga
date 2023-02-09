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
        auto vfioContainer = std::make_shared<kernel::vfio::Container>();

        //const int IOMMU_GROUP = 2;
        PlatformCard card = PlatformCard(vfioContainer);

        return 0;
}

PlatformCard::PlatformCard(std::shared_ptr<kernel::vfio::Container> vfioContainer)
{
        this -> vfioContainer = vfioContainer;

        logger = villas::logging.get("PlatformCard");
        // logger->info("Initializing FPGA card {}", DEV_NAME);

        // Attach PCIe card to VFIO container
        // vfioDevice = vfioContainer->attachDevice(DEV_NAME, DEV_GROUP);

        // // Enable memory access and PCI bus mastering for DMA
        // if(not vfioDevice->pciEnable()) {
        //         logger->error("Failed to enable PCI device");
        //         return false;
        // }
}
