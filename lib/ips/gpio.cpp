/** AXI General Purpose IO (GPIO)
 *
 * @author Steffen Vogel <post@steffenvogel.de>
 * @copyright 2017-2020, Steffen Vogel
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

#include <villas/plugin.hpp>

#include <villas/fpga/ips/gpio.hpp>

using namespace villas::fpga::ip;

bool
Gpio::init()
{
	//const uintptr_t base = getBaseAddr(registerMemory);

	return true;
}

static char n[] = "gpio";
static char d[] = "Xilinx's AXI4 general purpose IO";
static char v[] = "xilinx.com:ip:axi_gpio:";
static CorePlugin<Gpio, n, d, v> f;
