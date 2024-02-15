/* AXI-PCIe Interrupt controller
 *
 * Author: Steffen Vogel <post@steffenvogel.de>
 * Author: Daniel Krebs <github@daniel-krebs.net>
 * SPDX-FileCopyrightText: 2017 Steffen Vogel <post@steffenvogel.de>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <xilinx/xintc.h>

#include <villas/fpga/core.hpp>

namespace villas {
namespace fpga {
namespace ip {

class InterruptController : public Core {
public:
	using IrqMaskType = uint32_t;
	static constexpr int maxIrqs = 32;

	virtual
	~InterruptController();

	virtual
	bool init() override;

	virtual bool enableInterrupt(IrqMaskType mask, bool polling);
	virtual bool enableInterrupt(IrqPort irq, bool polling)
	{
		return enableInterrupt(1 << irq.num, polling);
	}

	virtual bool disableInterrupt(IrqMaskType mask);
	virtual bool disableInterrupt(IrqPort irq)
	{
		return disableInterrupt(1 << irq.num);
	}

	virtual int waitForInterrupt(int irq);
	virtual int waitForInterrupt(IrqPort irq)
	{
		return waitForInterrupt(irq.num);
	}

protected:

	static constexpr char registerMemory[] = "reg0";

	std::list<MemoryBlockName> getMemoryBlocks() const
	{
		return {
			registerMemory
		};
	}

	struct Interrupt {
		int eventFd;			// Event file descriptor
		int number;			// Interrupt number from /proc/interrupts
		bool polling;			// Polled or not
	};

	int num_irqs;				// Number of available MSI vectors
	int efds[maxIrqs];
	int nos[maxIrqs];
	bool polling[maxIrqs];
};

} // namespace ip
} // namespace fpga
} // namespace villas
