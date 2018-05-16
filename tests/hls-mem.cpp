/** FIFO unit test.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
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

#include <criterion/criterion.h>

#include <villas/log.hpp>
#include <villas/memory.hpp>
#include <villas/fpga/card.hpp>
#include <villas/fpga/ips/hls-mem.hpp>

#include "global.hpp"


Test(fpga, hls_mem, .description = "HlsMem")
{
	auto logger = loggerGetOrCreate("unittest:hls-mem");

	for(auto& ip : state.cards.front()->ips) {
		if(*ip != villas::fpga::Vlnv("xilinx.com:hls:mem:"))
			continue;

		logger->info("Testing {}", *ip);

		auto hlsMem = reinterpret_cast<villas::fpga::ip::HlsMem&>(*ip);

		auto dmaMem = villas::HostDmaRam::getAllocator(0).allocate<char>(4);

		cr_assert(hlsMem.testMemory(dmaMem.getMemoryBlock()), "Testing HlsMem failed");

		logger->info(TXT_GREEN("Passed"));
	}
}
