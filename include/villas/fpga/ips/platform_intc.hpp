#pragma once

#include <villas/fpga/ips/intc.hpp>

class PlatformInterruptController
    : public villas::fpga::ip::InterruptController {
public:
  bool init() override;

  bool enableInterrupt(InterruptController::IrqMaskType mask,
                       bool polling) override;
  bool enableInterrupt(IrqPort irq, bool polling) override;
};
