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
        using Ptr = std::shared_ptr<FpgaDevice>;
        using List = std::list<Ptr>;

        virtual ~FpgaDevice();

        // bool init();

        // 	bool stop()
        // 	{
        // 		return true;
        // 	}

        // 	bool check()
        // 	{
        // 		return true;
        // 	}

        // 	bool reset()
        // 	{
        // 		// TODO: Try via sysfs?
        // 		// echo 1 > /sys/bus/pci/devices/0000\:88\:00.0/reset
        // 		return true;
        // 	}

        // 	void dump()
        // 	{ }

        virtual ip::Core::Ptr lookupIp(const std::string &name) const;

        ip::Core::Ptr lookupIp(const Vlnv &vlnv) const;

        // 	ip::Core::Ptr
        // 	lookupIp(const ip::IpIdentifier &id) const;

        bool mapMemoryBlock(const MemoryBlock &block);

        // private:
        // 	// Cache a set of already mapped memory blocks
        // 	std::set<MemoryManager::AddressSpaceId> memoryBlocksMapped;

        // public:	// TODO: make this private
        // 	ip::Core::List ips;				// IPs located
        // on this FPGA card

        // 	bool doReset;					// Reset
        // VILLASfpga during startup?
        int affinity; // Affinity for MSI interrupts

        std::string name;

        // 	std::shared_ptr<kernel::pci::Device> pdev;	// PCI device
        // handle

        // 	// The VFIO container that this card is part of
        // 	std::shared_ptr<kernel::vfio::Container> vfioContainer;

        // The VFIO device that represents this card
        kernel::vfio::Device *vfioDevice;

        // 	// Slave address space ID to access the PCIe address space from
        MemoryManager::AddressSpaceId addrSpaceIdDeviceToHost;

        // 	// Address space identifier of the master address space of this
        // FPGA card.
        // 	// This will be used for address resolution of all IPs on this
        MemoryManager::AddressSpaceId addrSpaceIdHostToDevice;

        // protected:
        // 	Logger
        // 	getLogger() const
        // 	{
        // 		return villas::logging.get(name);
        // 	}

        // 	Logger logger;
};

class FpgaDeviceFactory : public plugin::Plugin
{
    public:
        static FpgaDevice::List
        make(json_t *json,
             std::shared_ptr<kernel::pci::DeviceList> pci,
             std::shared_ptr<kernel::vfio::Container> vc);

        // static FpgaDevice *create()
        // {
        //         return new FpgaDevice();
        // }

        // static Logger getStaticLogger()
        // {
        //         return villas::logging.get("pcie:card:factory");
        // }

        // virtual std::string getName() const
        // {
        //         return "pcie";
        // }

        // virtual std::string getDescription() const
        // {
        //         return "Xilinx PCIe FPGA cards";
        // }

        // virtual std::string getType() const
        // {
        //         return "card";
        // }
};
}
}

/** @} */
