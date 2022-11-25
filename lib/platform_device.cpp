#include <linux/vfio.h>
#include <villas/exceptions.hpp>
#include <villas/fpga/config.h>
#include <villas/fpga/platform_device.hpp>

using namespace villas;
using namespace villas::fpga;

PlatformDevice::PlatformDevice(std::string name,
                               std::shared_ptr<kernel::vfio::Container> vc,
                               const char *DEVICE_NAME,
                               const int IOMMU_GROUP)
    : FpgaDevice(name, vc), DEVICE_NAME(DEVICE_NAME), IOMMU_GROUP(IOMMU_GROUP)
{
        initVfio();
        // Enable memory access and PCI bus mastering for DMA
        // device.pciEnable();
}

bool PlatformDevice::initVfio()
{
        logger->info("Initializing FPGA card {}", name);

        // Attach PCIe card to VFIO container
        kernel::vfio::Device &device
            = vfioContainer->attachDevice(DEVICE_NAME, IOMMU_GROUP);
        this->vfioDevice = &device;

        this->vfioDevice->dump();

        return true;
}

FpgaDevice::List
PlatformDeviceFactory::make(std::shared_ptr<kernel::vfio::Container> vc,
                            json_t *json) const
{
        auto logger = getStaticLogger();
        FpgaDevice::List cards;

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

                const char * name = "READ FROM JSON";
                const int iommu_group = -1;
                auto card
                    = std::make_unique<PlatformDevice>(std::string(card_name),
                                                       std::move(vc),
                                                       name,
                                                       iommu_group);

                // card->affinity = affinity;

                // Search for FPGA card
                // card->pdev = pci->lookupDevice(filter);

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