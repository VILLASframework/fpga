#include <villas/fpga/ips/platform_intc.hpp>
#include <villas/fpga/platform_card.hpp>

bool PlatformInterruptController::init(){
  auto platform_card = dynamic_cast<villas::fpga::PlatformCard*>(card);
  platform_card->devices[0]->platformInit(efds);
}

bool PlatformInterruptController::enableInterrupt(
    InterruptController::IrqMaskType mask, bool polling) {
  logger->debug("Enabling Platform Interrupt");
};

bool PlatformInterruptController::enableInterrupt(IrqPort irq, bool polling) {
  logger->debug("Enabling Platform Interrupt");
};
