/** Jitter benchmarks.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017-2018, Steffen Vogel
 * @license GNU General Public License (version 3)
 *
 * VILLASfpga
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <villas/utils.hpp>

#include <villas/fpga/card.h>
#include <villas/fpga/ip.h>

#include <villas/fpga/ips/timer.h>

#include "bench.h"

int fpga_benchmark_jitter(struct fpga_card *c)
{
	int ret;

	struct fpga_ip *ip = list_lookup(&c->ips, "timer_0");
	if (!ip || !c->intc)
		return -1;

	struct timer *tmr = (struct timer *) ip->_vd;

	XTmrCtr *xtmr = &tmr->inst;

	ret = intc_enable(c->intc, (1 << ip->irq), intc_flags);
	if (ret)
		error("Failed to enable interrupt");

	float period = 50e-6;
	int runs = 300.0 / period;

	int *hist = alloc(8 << 20);

	XTmrCtr_SetOptions(xtmr, 0, XTC_INT_MODE_OPTION | XTC_EXT_COMPARE_OPTION | XTC_DOWN_COUNT_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(xtmr, 0, period * FPGA_AXI_HZ);
	XTmrCtr_Start(xtmr, 0);

	uint64_t end, start = rdtsc();
	for (int i = 0; i < runs; i++) {
		uint64_t cnt = intc_wait(c->intc, ip->irq);
		if (cnt != 1)
			warn("fail");

		// Ackowledge IRQ
		XTmrCtr_WriteReg((uintptr_t) c->map + ip->baseaddr, 0, XTC_TCSR_OFFSET, XTmrCtr_ReadReg((uintptr_t) c->map + ip->baseaddr, 0, XTC_TCSR_OFFSET));

		end = rdtsc();
		hist[i] = end - start;
		start = end;
	}

	XTmrCtr_Stop(xtmr, 0);

	char fn[256];
	snprintf(fn, sizeof(fn), "results/jitter_%s_%s.dat", intc_flags & INTC_POLLING ? "polling" : "irq", uts.release);
	FILE *g = fopen(fn, "w");
	for (int i = 0; i < runs; i++)
		fprintf(g, "%u\n", hist[i]);
	fclose(g);

	free(hist);

	ret = intc_disable(c->intc, (1 << ip->irq));
	if (ret)
		error("Failed to disable interrupt");

	return 0;
}
