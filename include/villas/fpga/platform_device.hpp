/** Platform Device
 *
 * This class represents a FPGA platform device.
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

/** @addtogroup fpga VILLASfpga
 * @{
 */

#pragma once

#include "fpga_device.hpp"
#include <villas/fpga/fpga_device.hpp>

namespace villas
{
namespace fpga
{

class PlatformDevice : public FpgaDevice
{
    public:
        using Ptr = std::shared_ptr<PlatformDevice>;
        using List = std::list<Ptr>;

        const char *DEVICE_NAME;
        const int IOMMU_GROUP;

        PlatformDevice(std::string name,
                       std::shared_ptr<kernel::vfio::Container> vc,
                       const char *DEVICE_NAME,
                       const int IOMMU_GROUP);
        ~PlatformDevice(){};

    private:
        bool initVfio();
};

class PlatformDeviceFactory : public FpgaDeviceFactory
{
    public:
        PlatformDevice::List make(std::shared_ptr<kernel::vfio::Container> vc,
                                  json_t *json) const override;

        static Logger getStaticLogger()
        {
                // ToDo: make a proper logger
                return villas::logging.get("pcie:card:factory");
        }
};
}
}

/** @} */
