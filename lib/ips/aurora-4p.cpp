/** Driver for wrapper around Aurora (acs.eonerc.rwth-aachen.de:user:aurora_4p_axis)
 *
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

#include <cstdint>

#include <villas/utils.hpp>

#include <villas/fpga/card.hpp>
#include <villas/fpga/ips/aurora-4p.hpp>

using namespace villas::fpga::ip;

static AuroraFactory aurora4pFactoryInstance;

void Aurora4P::dump()
{
	for (auto &p : ports)
		p.dump();
}

void Aurora::setLoopback(int index, bool state)
{
	ports[index].setLoopback(state)
}

void Aurora::resetFrameCounters()
{
	for (auto &p : ports)
		p.resetFrameCounters();
}
