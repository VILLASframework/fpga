/** AXI PCIe bridge
 *
 * Author: Daniel Krebs <github@daniel-krebs.net>
 * SPDX-FileCopyrightText: 2018 Institute for Automation of Complex Power Systems, EONERC
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#include <limits>
#include <jansson.h>

#include <villas/exceptions.hpp>
#include <villas/memory.hpp>

#include <villas/fpga/card.hpp>
#include <villas/fpga/ips/zynq.hpp>
#include <villas/fpga/pcie_card.hpp>

#include <villas/fpga/platform_card.hpp>


using namespace villas::fpga::ip;


bool
Zynq::init()
{
  auto &mm = MemoryManager::get();

	// IPs that can access this address space will know it via their memory view
	// const auto addrSpaceNameDeviceToHost =
	//         mm.getMasterAddrSpaceName("zynq_ultra_ps_e_0", "HPC1_DDR_LOW");
          //mm.getSlaveAddrSpaceName(getInstanceName(), pcieMemory);

	// Save ID in card so we can create mappings later when needed (e.g. when
	// allocating DMA memory in host RAM)
	card->addrSpaceIdDeviceToHost =
	        mm.getOrCreateAddressSpace("zynq_ultra_ps_e_0/HPC0_DDR_LOW");
  
  dynamic_cast<PlatformCard*>(card)->connectVFIOtoIPS();

  return true;
}

void
ZynqFactory::parse(Core &ip, json_t *cfg)
{
	CoreFactory::parse(ip, cfg);

	auto logger = getLogger();
	//auto &zynq = dynamic_cast<Zynq&>(ip);

	// for (auto barType : std::list<std::string>{
	// 	"axi_bars",
	// 	"pcie_bars"
	// }) {
	// 	json_t *json_bars = json_object_get(cfg, barType.c_str());
	// 	if (not json_is_object(json_bars))
	// 		throw ConfigError(cfg, "", "Missing BAR config: {}", barType);

	// 	json_t* json_bar;
	// 	const char* bar_name;
	// 	json_object_foreach(json_bars, bar_name, json_bar) {
	// 		unsigned int translation;

	// 		json_error_t err;
	// 		int ret = json_unpack_ex(json_bar, &err, 0, "{ s: i }",
	// 			"translation", &translation
	// 		);
	// 		if (ret != 0)
	// 			throw ConfigError(json_bar, err, "", "Cannot parse {}/{}", barType, bar_name);

	// 		if (barType == "axi_bars") {
	// 			json_int_t base, high, size;
	// 			int ret = json_unpack_ex(json_bar, &err, 0, "{ s: I, s: I, s: I }",
	// 			        "baseaddr", &base,
	// 			        "highaddr", &high,
	// 			        "size", &size
	// 			);
	// 			if (ret != 0)
	// 				throw ConfigError(json_bar, err, "", "Cannot parse {}/{}", barType, bar_name);

	// 			pcie.axiToPcieTranslations[bar_name] = {
	// 			    .base = static_cast<uintptr_t>(base),
	// 			    .size = static_cast<size_t>(size),
	// 			    .translation = translation
	// 			};
	// 		} else
	// 			pcie.pcieToAxiTranslations[bar_name] = {
	// 			    .translation = translation
	// 			};
	// 	}
	// }
}

static ZynqFactory p;
