/** Driver for wrapper around Quad Port Aurora (acs.eonerc.rwth-aachen.de:user:aurora_4p_axis)
 *
 * @file
 * @author Steffen Vogel <svogel2@eoenrc.rwth-aachen.de>
 * @copyright 2017-2020, Institute for Automation of Complex Power Systems, EONERC
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

class Aurora4P : public Node {
public:

	void dump();

	std::list<std::string> getMemoryBlocks() const
	{ return { registerMemory }; }

	const StreamVertex&
	getDefaultSlavePort() const
	{ return getSlavePort("s_axis0"); }

	const StreamVertex&
	getDefaultMasterPort() const
	{ return getMasterPort("m_axis0"); }

	void
	setLoopback(int index, bool state);

	void
	resetFrameCounters();

private:
	static constexpr const char registerMemory[] = "reg0";

	std::vector<Aurora *> ports;
};


class AuroraFactory : public NodeFactory {
public:

	Core* create()
	{
		auto *a =  new Aurora4P;

		for (int i = 0; i < 4; i++)
			a->ports.push_back(new Aurora);

		return a;
	}

	virtual std::string
	getName() const
	{ return "Aurora 4P"; }

	virtual std::string
	getDescription() const
	{ return "Aurora 8B/10B and additional support modules, like an AXI4-Lite register interface."; }

	virtual Vlnv
	getCompatibleVlnv() const
	{ return {"acs.eonerc.rwth-aachen.de:user:aurora_4p_axis:"}; }

};

} /* namespace ip */
} /* namespace fpga */
} /* namespace villas */

/** @} */
