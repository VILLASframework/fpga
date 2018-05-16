#include <unistd.h>

#include <villas/memory_manager.hpp>
#include <villas/fpga/ips/hls-mem.hpp>

#include <villas/fpga/ips/xmem.h>

#include "log.hpp"

namespace villas {
namespace fpga {
namespace ip {

static HlsMemFactory factory;

bool HlsMem::init()
{
	xmem.IsReady = XIL_COMPONENT_IS_READY;
	xmem.Ctrl_BaseAddress = getBaseAddr(registerMemory);

	return true;
}

bool HlsMem::testMemory(const MemoryBlock& mem)
{
	static constexpr uint8_t testValue = 42;

	auto& mm = MemoryManager::get();

	auto translationFromXMem = mm.getTranslation(
	                               getMasterAddrSpaceByInterface(axiInterface),
	                               mem.getAddrSpaceId());

	// set address of memory block in HLS IP
	XMem_Set_axi_master(&xmem, translationFromXMem.getLocalAddr(0));

	// prepare memory with a known value
	auto translationFromProcess = mm.getTranslationFromProcess(mem.getAddrSpaceId());
	auto memory = reinterpret_cast<volatile uint8_t*>(translationFromProcess.getLocalAddr(0));
	memory[0] = testValue;

	logger->debug("Memory before: {}", memory[0]);

	// start and wait until it is done
	XMem_Start(&xmem);
	while(!XMem_IsDone(&xmem));

	logger->debug("Memory after:  {}", memory[0]);

	const auto valueReadBack = XMem_Get_value_r(&xmem);
	if(valueReadBack != testValue) {
		logger->error("Read-back value is not equal to written value ({} != {})",
		              valueReadBack, testValue);
		return false;
	}

	const auto currentMemoryValue = memory[0];
	if(currentMemoryValue != 0x55) {
		logger->error("Memory value should have been overwritten to 0x55, but it is {}",
		              currentMemoryValue);
		return false;
	}

	return true;
}

HlsMemFactory::HlsMemFactory() :
    IpCoreFactory(getName())
{
}

} // namespace ip
} // namespace fpga
} // namespace villas
