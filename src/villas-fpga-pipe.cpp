/** Streaming data from STDIN/OUT to FPGA.
 *
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2017-2022, Steffen Vogel
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

#include <csignal>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <jansson.h>

#include <CLI11.hpp>
#include <rang.hpp>

#include <villas/exceptions.hpp>
#include <villas/log.hpp>
#include <villas/utils.hpp>
#include <villas/utils.hpp>

#include <villas/fpga/core.hpp>
#include <villas/fpga/card.hpp>
#include <villas/fpga/vlnv.hpp>
#include <villas/fpga/ips/dma.hpp>
#include <villas/fpga/ips/rtds.hpp>
#include <villas/fpga/ips/aurora_xilinx.hpp>
#include <villas/fpga/fpgaHelper.hpp>

using namespace villas;

static std::shared_ptr<kernel::pci::DeviceList> pciDevices;
static auto logger = villas::logging.get("streamer");

int main(int argc, char* argv[])
{
	// Command Line Parser
	CLI::App app{"VILLASfpga data streamer"};

	try {
		std::string configFile;
		app.add_option("-c,--config", configFile, "Configuration file")
				->check(CLI::ExistingFile);

		std::string fpgaName = "vc707";
		app.add_option("--fpga", fpgaName, "Which FPGA to use");
		app.parse(argc, argv);

		// Logging setup
		spdlog::set_level(spdlog::level::debug);
		fpga::setupColorHandling();

		if (configFile.empty()) {
			logger->error("No configuration file provided/ Please use -c/--config argument");
			return 1;
		}

		auto card = fpga::setupFpgaCard(configFile, fpgaName);

		std::vector<std::shared_ptr<fpga::ip::AuroraXilinx>> aurora_channels;
		for (int i = 0; i < 4; i++) {
			auto name = fmt::format("aurora_8b10b_ch{}", i);
			auto id = fpga::ip::IpIdentifier("xilinx.com:ip:aurora_8b10b:", name);
			auto aurora = std::dynamic_pointer_cast<fpga::ip::AuroraXilinx>(card->lookupIp(id));
			if (aurora == nullptr) {
				logger->error("No Aurora interface found on FPGA");
				return 1;
			}

			aurora_channels.push_back(aurora);
		}

		auto dma = std::dynamic_pointer_cast<fpga::ip::Dma>
					(card->lookupIp(fpga::Vlnv("xilinx.com:ip:axi_dma:")));
		if (dma == nullptr) {
			logger->error("No DMA found on FPGA ");
			return 1;
		}

		for (auto aurora : aurora_channels)
			aurora->dump();

		// Configure Crossbar switch
#if 1
		aurora_channels[2]->connect(aurora_channels[2]->getDefaultMasterPort(), dma->getDefaultSlavePort());
		dma->connect(dma->getDefaultMasterPort(), aurora_channels[3]->getDefaultSlavePort());
#else
		dma->connectLoopback();
#endif
		auto &alloc = villas::HostRam::getAllocator();
		const villas::MemoryBlock::Ptr block = std::move(alloc.allocateBlock(sizeof(int32_t)*0x100));
		int32_t* mem = reinterpret_cast<int32_t*>(block->getOffset());

		dma->makeAccesibleFromVA(block);

		auto &mm = MemoryManager::get();
		mm.getGraph().dump("graph.dot");

		while (true) {
			// Setup read transfer
			dma->read(*block, block->getSize());

			// Read values from stdin
			std::string line;
			std::getline(std::cin, line);
			auto values = villas::utils::tokenize(line, ";");

			size_t i = 0;
			for (auto &value: values) {
				if (value.empty()) continue;

				const int32_t number = std::stoi(value);
				mem[i++] = number;
			}

			// Initiate write transfer
			bool state = dma->write(*block, i * sizeof(int32_t));
			if (!state)
				logger->error("Failed to write to device");

			auto bytesWritten = dma->writeComplete();
			logger->info("Wrote {} bytes", bytesWritten);

			auto bytesRead = dma->readComplete();
			auto valuesRead = bytesRead / sizeof(int32_t);
			logger->info("Read {} bytes", bytesRead);

			for (size_t i = 0; i < valuesRead; i++)
				std::cerr << mem[i] << ";";
			std::cerr << std::endl;
		}
	} catch (const RuntimeError &e) {
		logger->error("Error: {}", e.what());
		return -1;
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}

	return 0;
}
