#include <villas/fpga/ips/platform_intc.hpp>

bool PlatformInterruptController::enableInterrupt(
    InterruptController::IrqMaskType mask, bool polling) {
  logger->debug("Enabling Platform Interrupt");
};

bool PlatformInterruptController::enableInterrupt(IrqPort irq, bool polling) {
  logger->debug("Enabling Platform Interrupt");
};
