#pragma once

#include <villas/memory.hpp>
#include <villas/fpga/ips/xmem.h>
#include "fpga/ip.hpp"

namespace villas {
namespace fpga {
namespace ip {


class HlsMem : public IpCore
{
public:
	friend class HlsMemFactory;

	bool init();

	bool testMemory(const MemoryBlock& mem);

private:
	static constexpr const char* registerMemory = "Reg";
	static constexpr const char* axiInterface = "m_axi_gmem";

	std::list<MemoryBlockName> getMemoryBlocks() const
	{ return { registerMemory }; }

	XMem xmem;
};


class HlsMemFactory : public IpCoreFactory {
public:
	HlsMemFactory();

	IpCore* create()
	{ return new HlsMem; }

	std::string
	getName() const
	{ return "HlsMem"; }

	std::string
	getDescription() const
	{ return "HLS memory test"; }

	Vlnv getCompatibleVlnv() const
	{ return {"xilinx.com:hls:mem:"}; }
};

} // namespace ip
} // namespace fpga
} // namespace villas
