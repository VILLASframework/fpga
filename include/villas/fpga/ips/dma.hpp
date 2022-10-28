/** DMA driver
 *
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2018-2022, Institute for Automation of Complex Power Systems, EONERC
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
 ******************************************************************************/

#pragma once

#include <xilinx/xaxidma.h>

#include <villas/memory.hpp>
#include <villas/fpga/node.hpp>

namespace villas {
namespace fpga {
namespace ip {

class Dma : public Node
{
public:
	friend class DmaFactory;

	bool init();
	bool reset();

	// Memory-mapped to stream (MM2S)
	bool write(const MemoryBlock &mem, size_t len);

	// Stream to memory-mapped (S2MM)
	bool read(const MemoryBlock &mem, size_t len);

	size_t writeComplete()
	{
		return hasScatterGather() ? writeCompleteSG() : writeCompleteSimple();
	}

	size_t readComplete()
	{
		return hasScatterGather() ? readCompleteSG() : readCompleteSimple();
	}

	bool memcpy(const MemoryBlock &src, const MemoryBlock &dst, size_t len);

	void makeAccesibleFromVA(const MemoryBlock &mem);
	bool makeInaccesibleFromVA(const MemoryBlock &mem);

	inline bool
	hasScatterGather() const
	{
		return hasSG;
	}

	const StreamVertex&
	getDefaultSlavePort() const
	{
		return getSlavePort(s2mmPort);
	}

	const StreamVertex&
	getDefaultMasterPort() const
	{
		return getMasterPort(mm2sPort);
	}

private:
	bool writeSG(const void* buf, size_t len);
	bool readSG(void* buf, size_t len);
	size_t writeCompleteSG();
	size_t readCompleteSG();

	bool writeSimple(const void* buf, size_t len);
	bool readSimple(void* buf, size_t len);
	size_t writeCompleteSimple();
	size_t readCompleteSimple();

	void setupRingRx();
	void setupRingTx();

public:
	static constexpr const char* s2mmPort = "S2MM";
	static constexpr const char* mm2sPort = "MM2S";

	bool isMemoryBlockAccesible(const MemoryBlock &mem, const std::string &interface);

	virtual void dump();

private:
	static constexpr char registerMemory[] = "Reg";

	static constexpr char mm2sInterrupt[] = "mm2s_introut";
	static constexpr char mm2sInterface[] = "M_AXI_MM2S";

	static constexpr char s2mmInterrupt[] = "s2mm_introut";
	static constexpr char s2mmInterface[] = "M_AXI_S2MM";

	// Optional Scatter-Gather interface to access descriptors
	static constexpr char sgInterface[] = "M_AXI_SG";

	std::list<MemoryBlockName> getMemoryBlocks() const
	{
		return {
			registerMemory
		};
	}

	XAxiDma xDma;
	bool hasSG;

	MemoryBlock sgRings;
};

class DmaFactory : public NodeFactory {
public:

	Core* create()
	{
		return new Dma;
	}

	virtual std::string
	getName() const
	{
		return "Dma";
	}

	virtual std::string
	getDescription() const
	{
		return "Xilinx's AXI4 Direct Memory Access Controller";
	}

	virtual Vlnv
	getCompatibleVlnv() const
	{
		return {
			"xilinx.com:ip:axi_dma:"
		};
	}
};

} /* namespace ip */
} /* namespace fpga */
} /* namespace villas */
