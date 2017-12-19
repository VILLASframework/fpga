/** Timer related helper functions
 *
 * These functions present a simpler interface to Xilinx' Timer Counter driver (XTmrCtr_*)
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2017, Steffen Vogel
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

#include "fpga/ip.hpp"
#include <xilinx/xtmrctr.h>

namespace villas {
namespace fpga {
namespace ip {


class Timer : public IpCore
{
public:
//	~Timer();

	bool start();

private:
	XTmrCtr xtmr;
};



class TimerFactory : public IpCoreFactory {
public:

	TimerFactory() :
	    IpCoreFactory(getName())
	{}

	IpCore* create()
	{ return new Timer; }

	std::string
	getName() const
	{ return "Timer"; }

	std::string
	getDescription() const
	{ return "Xilinx's programmable timer / counter"; }

	Vlnv getCompatibleVlnv() const
	{ return {"xilinx.com:ip:axi_timer:"}; }

	std::list<IpDependency> getDependencies() const
	{ return { {"intc", Vlnv("acs.eonerc.rwth-aachen.de:user:axi_pcie_intc:") } }; }
};

} // namespace ip
} // namespace fpga
} // namespace villas

/** @} */
