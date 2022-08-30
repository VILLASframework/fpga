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

#include <cstdint>

#include <villas/utils.hpp>

#include <villas/fpga/card.hpp>
#include <villas/fpga/ips/aurora_xilinx.hpp>

using namespace villas::fpga::ip;

static AuroraXilinxFactory auroraFactoryInstance;
