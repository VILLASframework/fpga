/** FPGA card
 *
 * This class represents a FPGA device.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2017-2022, Institute for Automation of Complex Power Systems,
 *EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASfpga
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

/** @addtogroup fpga VILLASfpga
 * @{
 */

#pragma once

#include <jansson.h>
#include <list>
#include <set>
#include <string>

#include <villas/memory.hpp>
#include <villas/plugin.hpp>

#include <villas/kernel/pci.hpp>
#include <villas/kernel/vfio.hpp>

#include <villas/fpga/config.h>
#include <villas/fpga/core.hpp>

namespace villas
{
namespace fpga
{

class FpgaDevice
{
    public:
        ip::Core::List ips;

        // The VFIO container that this card is part of
        std::shared_ptr<kernel::vfio::Container> vfioContainer;

        // The VFIO device that represents this card
        kernel::vfio::Device* vfioDevice;

		// Slave address space ID to access the PCIe address space from the FPGA
		MemoryManager::AddressSpaceId addrSpaceIdDeviceToHost;

        // Address space identifier of the master address space of this FPGA
        // card. This will be used for address resolution of all IPs on this
        // card.
        MemoryManager::AddressSpaceId addrSpaceIdHostToDevice;

		//kernel::vfio::Device* vfioDevice;

        ip::Core::Ptr lookupIp(const std::string &name) const;
};

class FpgaDeviceFactory : public plugin::Plugin
{
    public:
};
}
}

/** @} */
