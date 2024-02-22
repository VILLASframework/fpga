#include <exception>
#include <memory>
#include <stdexcept>
#include <villas/fpga/ips/platform_intc.hpp>
#include <villas/fpga/platform_card.hpp>

bool PlatformInterruptController::init(){
  auto platform_card = dynamic_cast<villas::fpga::PlatformCard*>(card);

  std::shared_ptr<villas::kernel::vfio::Device> dma_vfio_device;
  for (auto device : platform_card->devices)
  {
    if (device->getName().find("dma") != std::string::npos){
      dma_vfio_device = device;
      break;
    }
  }

  if(dma_vfio_device == nullptr) throw std::logic_error("No vfio device with name containing dma found.");

  dma_vfio_device->platformInit(efds);
}

bool PlatformInterruptController::enableInterrupt(
    InterruptController::IrqMaskType mask, bool polling) {
  logger->debug("Enabling Platform Interrupt");
};

bool PlatformInterruptController::enableInterrupt(IrqPort irq, bool polling) {
  logger->debug("Enabling Platform Interrupt");
};
