/** Driver for wrapper around standard Xilinx Aurora (xilinx.com:ip:aurora_8b10b)
 *
 * @author Steffen Vogel <post@steffenvogel.de>
 * @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
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

#include <villas/fpga/node.hpp>

namespace villas {
namespace fpga {
namespace ip {

class AuroraXilinx : public Node {
public:
	static constexpr const char* masterPort = "m_axis";
	static constexpr const char* slavePort = "s_axis";

	const StreamVertex&
	getDefaultSlavePort() const
	{ return getSlavePort(slavePort); }

	const StreamVertex&
	getDefaultMasterPort() const
	{ return getMasterPort(masterPort); }
};


class AuroraXilinxFactory : public NodeFactory {
public:

	Core* create()
	{ return new AuroraXilinx; }

	virtual std::string
	getName() const
	{ return "Aurora"; }

	virtual std::string
	getDescription() const
	{ return "Xilinx Aurora 8B/10B."; }

	virtual Vlnv
	getCompatibleVlnv() const
	{ return {"xilinx.com:ip:aurora_8b10b:"}; }

};

} /* namespace ip */
} /* namespace fpga */
} /* namespace villas */

/** @} */
