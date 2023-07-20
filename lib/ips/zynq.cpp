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


using namespace villas::fpga::ip;


bool
Zynq::init()
{
	auto &mm = MemoryManager::get();

	// Throw an exception if the is no bus master interface and thus no
	// address space we can use for translation -> error

	card->addrSpaceIdHostToDevice = mm.findAddressSpace("zynq_ultra_ps_e_0:M_AXI_HPM0_FPD");

	// Map PCIe BAR0 via VFIO
	const void* bar0_mapped = card->vfioDevice->regionMap(0);
	if (bar0_mapped == MAP_FAILED) {
		logger->error("Failed to mmap() BAR0");
		return false;
	}

	const size_t mem_size = UINT64_MAX;

	// Create a mapping from process address space to the FPGA card via vfio
	size_t srcVertexId = mm.getProcessAddressSpace();
	size_t targetVertexId = card->addrSpaceIdHostToDevice;
	mm.createMapping(reinterpret_cast<uintptr_t>(bar0_mapped),
					 0, mem_size, "vfio-h2d", srcVertexId, targetVertexId);


	//! Dev
	// Make PCIe (IOVA) address space available to FPGA via BAR0

	// // IPs that can access this address space will know it via their memory view
	// const auto addrSpaceNameDeviceToHost =
	//         mm.getSlaveAddrSpaceName(getInstanceName(), pcieMemory);

	// // Save ID in card so we can create mappings later when needed (e.g. when
	// // allocating DMA memory in host RAM)
	// card->addrSpaceIdDeviceToHost =
	//         mm.getOrCreateAddressSpace(addrSpaceNameDeviceToHost);

	// auto pciAddrSpaceId = mm.getPciAddressSpace();
	// auto region = card->vfioDevice->regions[0];
	// int region_num = 0

	// auto pciBar = pcieToAxiTranslations.at("bar0");

	// mm.createMapping(region.start, pciBar.translation, bar0_size,
	// 					std::string("PCI-") + barName,
	// 					pciAddrSpaceId, card->addrSpaceIdHostToDevice);


	// for (auto& [barName, axiBar] : axiToPcieTranslations) {
	// 	logger->info("AXI-{}: bus addr={:#x} size={:#x}",
	// 	             barName, axiBar.base, axiBar.size);
	// 	logger->info("AXI-{}: PCI translation offset: {:#x}",
	// 	             barName, axiBar.translation);

	// 	auto barXAddrSpaceName = mm.getSlaveAddrSpaceName(getInstanceName(), barName);
	// 	auto barXAddrSpaceId = mm.getOrCreateAddressSpace(barXAddrSpaceName);

	// 	// Base is already incorporated into mapping of each IP by Vivado, so
	// 	// the mapping src has to be 0
	// 	mm.createMapping(0, axiBar.translation, axiBar.size,
	// 	                 std::string("AXI-") + barName,
	// 	                 barXAddrSpaceId, pciAddrSpaceId);

	// 	i++;
	// }
	
	// auto const baseaddr = 2684354560;
    // auto const size = 65536;
	// srcVertexId = mm.getProcessAddressSpace();
	// targetVertexId = mm.findAddressSpace("zynq_ultra_ps_e_0/HPC1_DDR_LOW");
	// mm.createMapping(baseaddr, 0, size,
	// 	                 std::string("AXI-") + "BAR0",
	// 	                 targetVertexId, srcVertexId);
	
	// auto const highaddr =  2684420095;
	// srcVertexId = mm.getProcessAddressSpace();
	// targetVertexId = mm.findAddressSpace("zynq_ultra_ps_e_0/HPC1_DDR_HIGH");
	// mm.createMapping(highaddr, 0, size,
	// 	                 std::string("AXI-") + "BAR0",
	// 	                 targetVertexId, srcVertexId);

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
