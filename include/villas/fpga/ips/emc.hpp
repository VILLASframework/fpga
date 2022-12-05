/** AXI External Memory Controller (EMC)
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
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

#pragma once

#include <xilinx/xilflash.h>

#include <villas/fpga/core.hpp>

namespace villas {
namespace fpga {
namespace ip {

class EMC : public Core {
public:

	virtual
	bool init() override;

	bool flash(uint32_t offset, const std::string &filename);
	bool flash(uint32_t offset, uint32_t length, uint8_t *data);

	bool read(uint32_t offset, uint32_t length, uint8_t *data);

private:

	XFlash xflash;

	static constexpr char registerMemory[] = "Reg";

	std::list<MemoryBlockName> getMemoryBlocks() const
	{
		return {
			registerMemory
		};
	}
};

class EMCFactory : public CoreFactory {
public:

	Core* create()
	{
		return new EMC;
	}

	virtual std::string
	getName() const
	{
		return "ExternalMemoryController";
	}

	virtual std::string
	getDescription() const
	{
		return "Xilinx's AXI External Memory Controller (EMC) ";
	}

	virtual Vlnv
	getCompatibleVlnv() const
	{
		return Vlnv("xilinx.com:ip:axi_emc:");
	}
};

} /* namespace ip */
} /* namespace fpga */
} /* namespace villas */
