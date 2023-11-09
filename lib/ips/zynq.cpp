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
  // auto platform_card = dynamic_cast<PlatformCard*>(card);

  //! Hardcoded edges vfios to ips

  // IPs that can access this address space will know it via their memory view
	const auto addrSpaceNameDeviceToHost =
	        mm.getMasterAddrSpaceName("axi_dma_0", "M_AXI_SG");

	// Save ID in card so we can create mappings later when needed (e.g. when
	// allocating DMA memory in host RAM)
	card->addrSpaceIdDeviceToHost =
	        mm.getOrCreateAddressSpace(addrSpaceNameDeviceToHost);

  /*
  auto platform_card = dynamic_cast<PlatformCard*>(card);
  for(auto vfio_device : platform_card->devices)
  {
    const size_t ip_mem_size = 65536;
    size_t srcVertexId = mm.getOrCreateAddressSpace(DEVICE_TREE_NAME);
    size_t targetVertexId = mm.getOrCreateAddressSpace(MEMORY_GRAPH_NAME/Reg);
    mm.createMapping(0, 0, ip_mem_size, "vfio to ip", srcVertexId,
                    targetVertexId);
  }
  */

  //? Solve Strat: search for name
  // DMA
  const size_t ip_mem_size = 65536;
  size_t srcVertexId = mm.getOrCreateAddressSpace("a0000000.dma");
  size_t targetVertexId = mm.getOrCreateAddressSpace("axi_dma_0/Reg");
  mm.createMapping(0, 0, ip_mem_size, "vfio to ip", srcVertexId,
                   targetVertexId);

  // Switch
  srcVertexId = mm.getOrCreateAddressSpace("a0010000.axis_switch");
  targetVertexId = mm.getOrCreateAddressSpace("axis_interconnect_0_xbar/Reg");
  mm.createMapping(0, 0, ip_mem_size, "vfio to ip", srcVertexId,
                   targetVertexId);

  //! Hardcoded end

  //mm.getGraph().removeVertex(mm.getOrCreateAddressSpace("axis_interconnect_0_xbar/Reg"));

  //! Dev
  // Make PCIe (IOVA) address space available to FPGA via BAR0

  // // IPs that can access this address space will know it via their memory
  // view const auto addrSpaceNameDeviceToHost =
  //         mm.getSlaveAddrSpaceName(getInstanceName(), pcieMemory);

  // // Save ID in card so we can create mappings later when needed (e.g. when
  // // allocating DMA memory in host RAM)
  // card->addrSpaceIdDeviceToHost =
  //         mm.getOrCreateAddressSpace(addrSpaceNameDeviceToHost);

  // auto pciAddrSpaceId = mm.getPciAddressSpace();

  // auto regions = dynamic_cast<PCIeCard*>(card)->pdev->getRegions();

  // int i = 0;
  // for (auto region : regions) {
  // 	const size_t region_size = region.end - region.start + 1;

  // 	char barName[] = "BARx";
  // 	barName[3] = '0' + region.num;
  // 	auto pciBar = pcieToAxiTranslations.at(barName);

  // 	logger->info("PCI-BAR{}: bus addr={:#x} size={:#x}",
  // 	             region.num, region.start, region_size);
  // 	logger->info("PCI-BAR{}: AXI translation offset {:#x}",
  // 	             i, pciBar.translation);

  // 	mm.createMapping(region.start, pciBar.translation, region_size,
  // 	                 std::string("PCI-") + barName,
  // 	                 pciAddrSpaceId, card->addrSpaceIdHostToDevice);
  // }

  // for (auto& [barName, axiBar] : axiToPcieTranslations) {
  // 	logger->info("AXI-{}: bus addr={:#x} size={:#x}",
  // 	             barName, axiBar.base, axiBar.size);
  // 	logger->info("AXI-{}: PCI translation offset: {:#x}",
  // 	             barName, axiBar.translation);

  // 	auto barXAddrSpaceName = mm.getSlaveAddrSpaceName(getInstanceName(),
  // barName); 	auto barXAddrSpaceId =
  // mm.getOrCreateAddressSpace(barXAddrSpaceName);

  // 	// Base is already incorporated into mapping of each IP by Vivado, so
  // 	// the mapping src has to be 0
  // 	mm.createMapping(0, axiBar.translation, axiBar.size,
  // 	                 std::string("AXI-") + barName,
  // 	                 barXAddrSpaceId, pciAddrSpaceId);

  // 	i++;
  // }

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
