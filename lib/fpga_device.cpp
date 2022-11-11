#include <villas/fpga/fpga_device.hpp>

using namespace villas;
using namespace villas::fpga;

FpgaDevice::FpgaDevice(std::string name,
                       std::shared_ptr<kernel::vfio::Container> vc)
    : name(name), vfioContainer(vc), logger(villas::logging.get(name))
{
}

FpgaDevice::~FpgaDevice()
{
}

ip::Core::Ptr FpgaDevice::lookupIp(const std::string &name) const
{
        for(auto &ip : ips) {
                if(*ip == name) {
                        return ip;
                }
        }

        return nullptr;
}

ip::Core::Ptr FpgaDevice::lookupIp(const Vlnv &vlnv) const
{
        for(auto &ip : ips) {
                if(*ip == vlnv) {
                        return ip;
                }
        }

        return nullptr;
}

bool FpgaDevice::mapMemoryBlock(const MemoryBlock &block)
{
        if(not vfioContainer->isIommuEnabled()) {
                logger->warn("VFIO mapping not supported without IOMMU");
                return false;
        }

        auto &mm = MemoryManager::get();
        const auto &addrSpaceId = block.getAddrSpaceId();

        if(memoryBlocksMapped.find(addrSpaceId) != memoryBlocksMapped.end())
                // Block already mapped
                return true;
        else
                logger->debug("Create VFIO mapping for {}", addrSpaceId);

        auto translationFromProcess
            = mm.getTranslationFromProcess(addrSpaceId);
        uintptr_t processBaseAddr = translationFromProcess.getLocalAddr(0);
        uintptr_t iovaAddr = vfioContainer->memoryMap(processBaseAddr,
                                                      UINTPTR_MAX,
                                                      block.getSize());

        if(iovaAddr == UINTPTR_MAX) {
                logger->error("Cannot map memory at {:#x} of size {:#x}",
                              processBaseAddr,
                              block.getSize());
                return false;
        }

        mm.createMapping(iovaAddr,
                         0,
                         block.getSize(),
                         "VFIO-D2H",
                         this->addrSpaceIdDeviceToHost,
                         addrSpaceId);

        // Remember that this block has already been mapped for later
        memoryBlocksMapped.insert(addrSpaceId);

        return true;
}
