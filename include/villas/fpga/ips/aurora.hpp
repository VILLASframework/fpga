/** Driver for wrapper around Aurora (acs.eonerc.rwth-aachen.de:user:aurora)
 *
 * @file
 * @author Hatim Kanchwala <hatim@hatimak.me>
 * @copyright 2020-2022, Institute for Automation of Complex Power Systems, EONERC
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

#include <villas/fpga/node.hpp>

namespace villas {
namespace fpga {
namespace ip {

class Aurora : public Node {
public:
	static constexpr const char* masterPort = "m_axis";
	static constexpr const char* slavePort = "s_axis";

	virtual
	void dump() override;

	std::list<std::string> getMemoryBlocks() const
	{
		return {
			registerMemory
		};
	}

	const StreamVertex&
	getDefaultSlavePort() const
	{
		return getSlavePort(slavePort);
	}

	const StreamVertex&
	getDefaultMasterPort() const
	{
		return getMasterPort(masterPort);
	}

	void
	setLoopback(bool state);

	void
	resetFrameCounters();

private:
	static constexpr const char registerMemory[] = "reg0";
};

class AuroraFactory : public NodeFactory {
public:

	Core* create()
	{
		return new Aurora;
	}

	virtual std::string
	getName() const
	{
		return "Aurora";
	}

	virtual std::string
	getDescription() const
	{
		return "Aurora 8B/10B and additional support modules, like an AXI4-Lite register interface.";
	}

	virtual Vlnv
	getCompatibleVlnv() const
	{
		return Vlnv("acs.eonerc.rwth-aachen.de:user:aurora_axis:");
	}
};

} /* namespace ip */
} /* namespace fpga */
} /* namespace villas */
