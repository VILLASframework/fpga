#pragma once

#include <villas/memory.hpp>
#include <villas/fpga/ip_node.hpp>

#include "rtds2gpu/xrtds2gpu.h"
#include "rtds2gpu/register_types.hpp"

namespace villas {
namespace fpga {
namespace ip {


class Rtds2Gpu : public IpNode
{
public:
	friend class Rtds2GpuFactory;

	bool init();

	bool start();

	void dump(spdlog::level::level_enum logLevel = spdlog::level::info);

	bool startOnce(const MemoryBlock& mem, size_t frameSize, size_t dataOffset, size_t doorbellOffset);

	bool isFinished();

	bool isReady();

	size_t getMaxFrameSize();

	void dumpDoorbell(uint32_t doorbellRegister) const;

private:
	bool updateStatus();

private:
	static constexpr const char* registerMemory = "Reg";
	static constexpr const char* axiInterface = "m_axi_axi_mm";
	static constexpr const char* streamInterface = "rtds_input";

	std::list<MemoryBlockName> getMemoryBlocks() const
	{ return { registerMemory }; }

	XRtds2gpu xInstance;

	axilite_reg_status_t status;
	size_t maxFrameSize;

	bool started;
};


class Rtds2GpuFactory : public IpNodeFactory {
public:
	Rtds2GpuFactory();

	IpCore* create()
	{ return new Rtds2Gpu; }

	std::string
	getName() const
	{ return "Rtds2Gpu"; }

	std::string
	getDescription() const
	{ return "HLS RTDS2GPU IP"; }

	Vlnv getCompatibleVlnv() const
	{ return {"acs.eonerc.rwth-aachen.de:hls:rtds2gpu:"}; }
};

} // namespace ip
} // namespace fpga
} // namespace villas