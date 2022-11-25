/** FPGA Device
 *
 * This class represents a FPGA device.
 *
 * @file
 * @author Pascal Bauer <pascal.bauer@rwth-aachen.de>
  based on the work of: Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
                        Daniel Krebs <github@daniel-krebs.net>
                        
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

#pragma once

#include <jansson.h>
#include <list>
#include <set>
#include <string>

#include <villas/memory.hpp>
#include <villas/plugin.hpp>

#include <villas/kernel/vfio.hpp>

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

        std::string name;
        // ToDo: Refactor with interrupt class
        bool polling;

        // This should be private but the core directly access it
        ip::Core::List ips;
        std::shared_ptr<kernel::vfio::Container> vfioContainer;
        kernel::vfio::Device *vfioDevice;

        FpgaDevice(){}; // for compatibility with card.hpp
        virtual ~FpgaDevice() = 0;

        ip::Core::Ptr lookupIp(const std::string &name) const;
        ip::Core::Ptr lookupIp(const Vlnv &vlnv) const;

        bool mapMemoryBlock(const MemoryBlock &block);

    private:
        // Address space identifier of the master address space of this
        // FPGA card.
        // This will be used for address resolution of all IPs on this
        MemoryManager::AddressSpaceId addrSpaceIdHostToDevice;
        MemoryManager::AddressSpaceId addrSpaceIdDeviceToHost;

        // Cache a set of already mapped memory blocks
        std::set<MemoryManager::AddressSpaceId> memoryBlocksMapped;

    protected:
        Logger logger;

        FpgaDevice(std::string name,
                   std::shared_ptr<kernel::vfio::Container> vc);
};

class FpgaDeviceFactory : public plugin::Plugin
{
    public:
        virtual FpgaDevice::List
        make(std::shared_ptr<kernel::vfio::Container> vc, json_t *json) const = 0;

        static Logger getStaticLogger()
        {
                // ToDo: make a proper logger
                return villas::logging.get("pcie:card:factory");
        }
};

}// namespace fpga
}// namespace villas
