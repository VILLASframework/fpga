#include <memory>
#include <villas/exceptions.hpp>
#include <villas/fpga/fpgaDevice.hpp>

using namespace villas;
using namespace villas::fpga;

FpgaDevice::~FpgaDevice()
{
}

ip::Core::Ptr
FpgaDevice::lookupIp(const std::string &name) const
{
	for (auto &ip : ips) {
		if (*ip == name) {
			return ip;
		}
	}

	return nullptr;
}

ip::Core::Ptr
FpgaDevice::lookupIp(const Vlnv &vlnv) const
{
	for (auto &ip : ips) {
		if (*ip == vlnv) {
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

FpgaDevice::List
FpgaDeviceFactory::make(json_t *json,
                        std::shared_ptr<kernel::vfio::Container> vc)
{
        FpgaDevice::List cards;
        auto logger = getStaticLogger();

        const char *card_name;
        json_t *json_card;
        json_object_foreach(json, card_name, json_card)
        {
                logger->info("Found config for FPGA card {}", card_name);

                json_t *json_ips = nullptr;
                const char *pci_slot = nullptr;
                const char *pci_id = nullptr;
                int do_reset = 0;
                int affinity = 0;

                int ret = json_unpack(json_card,
                                      "{ s: o, s?: i, s?: b, s?: s, s?: s }",
                                      "ips",
                                      &json_ips,
                                      "affinity",
                                      &affinity,
                                      "do_reset",
                                      &do_reset,
                                      "slot",
                                      &pci_slot,
                                      "id",
                                      &pci_id);

                if(ret != 0) {
                        logger->warn("Cannot parse JSON config");
                        continue;
                }

                auto card = std::make_unique<FpgaDevice>();

                // Populate generic properties
                card->name = std::string(card_name);
                card->vfioContainer = std::move(vc);
                card->affinity = affinity;

                // kernel::pci::Device filter = defaultFilter;

                // if(pci_id)
                //         filter.id = kernel::pci::Id(pci_id);
                // if(pci_slot)
                //         filter.slot = kernel::pci::Slot(pci_slot);

                // // Search for FPGA card
                // card->pdev = pci->lookupDevice(filter);
                // if(!card->pdev) {
                //         logger->warn("Failed to find PCI device");
                //         continue;
                // }

                // if(not card->init()) {
                //         logger->warn("Cannot start FPGA card {}",
                //         card_name); continue;
                // }

                // Load IPs from a separate json file
                if(json_is_string(json_ips)) {
                        auto json_ips_fn = json_string_value(json_ips);
                        json_ips = json_load_file(json_ips_fn, 0, nullptr);
                        if(json_ips == nullptr)
                                throw ConfigError(
                                    json_ips,
                                    "node-config-fpga-ips",
                                    "Failed to load FPGA IP cores from {}",
                                    json_ips_fn);
                }

                if(not json_is_object(json_ips))
                        throw ConfigError(
                            json_ips,
                            "node-config-fpga-ips",
                            "FPGA IP core list must be an object!");

                card->ips = ip::CoreFactory::make(card.get(), json_ips);
                if(card->ips.empty())
                        throw ConfigError(
                            json_ips,
                            "node-config-fpga-ips",
                            "Cannot initialize IPs of FPGA card {}",
                            card_name);

                cards.push_back(std::move(card));
        }

        return cards;
}