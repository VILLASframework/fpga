/**
 * SPDX-FileCopyrightText: 2018 Institute for Automation of Complex Power Systems, EONERC
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#include "villas/fpga/ips/switch.hpp"
#include <exception>
#include <jansson.h>
#include <unistd.h>
#include <villas/exceptions.hpp>
#include <villas/fpga/ips/dma.hpp>
#include <villas/fpga/platform_card.hpp>
#include <villas/utils.hpp>
#include <villas/fpga/utils.hpp>

using namespace villas;
using namespace fpga;

static auto logger = villas::logging.get("PLATFORM CTRL");

void writeToDmaFromStdIn(std::shared_ptr<villas::fpga::ip::Dma> dma)
{
        auto &alloc = villas::HostRam::getAllocator();
        const std::shared_ptr<villas::MemoryBlock> block
            = alloc.allocateBlock(0x200 * sizeof(float));
        villas::MemoryAccessor<float> mem = *block;
        dma->makeAccesibleFromVA(block);

        logger->info(
            "Please enter values to write to the device, separated by ';'");


        // Read values from stdin
        std::string line;
        std::getline(std::cin, line);
        auto values = villas::utils::tokenize(line, ";");

        size_t i = 0;
        for(auto &value : values) {
                if(value.empty())
                        continue;

                const float number = std::stof(value);
                mem[i++] = number;
        }

        // Initiate write transfer
        bool state = dma->write(*block, i * sizeof(float));
        if(!state)
                logger->error("Failed to write to device");

        auto writeComp = dma->writeComplete();

        // logger->debug("Wrote {} bytes", writeComp.bytes);
        
        // auto &alloc = villas::HostRam::getAllocator();

        // const std::shared_ptr<villas::MemoryBlock> block[] = {
        // 	alloc.allocateBlock(0x200 * sizeof(uint32_t)),
        // 	alloc.allocateBlock(0x200 * sizeof(uint32_t))
        // };
        // villas::MemoryAccessor<int32_t> mem[] = {*block[0], *block[1]};

        // for (auto b : block) {
        // 	dma->makeAccesibleFromVA(b);
        // }

        // size_t cur = 0, next = 1;
        // std::ios::sync_with_stdio(false);
        // std::string line;
        // bool firstXfer = true;

        // while(true) {
        // 	// Read values from stdin

        // 	std::getline(std::cin, line);
        // 	auto values = villas::utils::tokenize(line, ";");

        // 	size_t i = 0;
        // 	for (auto &value: values) {
        // 		if (value.empty()) continue;

        // 		const float number = std::stof(value);
        // 		mem[cur][i++] = number;
        // 	}

        // 	// Initiate write transfer
        // 	bool state = dma->write(*block[cur], i * sizeof(float));
        // 	if (!state)
        // 		logger->error("Failed to write to device");

        // 	if (!firstXfer) {
        // 		auto bytesWritten = dma->writeComplete();
        // 		logger->debug("Wrote {} bytes", bytesWritten.bytes);
        // 	} else {
        // 		firstXfer = false;
        // 	}

        // 	cur = next;
        // 	next = (next + 1) % (sizeof(mem) / sizeof(mem[0]));
        // }
}

void readFromDmaToStdOut(
    std::shared_ptr<villas::fpga::ip::Dma> dma,
    std::unique_ptr<fpga::BufferedSampleFormatter> formatter)
{
        auto &alloc = villas::HostRam::getAllocator();

        const std::shared_ptr<villas::MemoryBlock> block[]
            = { alloc.allocateBlock(0x200 * sizeof(uint32_t)),
                alloc.allocateBlock(0x200 * sizeof(uint32_t)) };
        villas::MemoryAccessor<int32_t> mem[] = { *block[0], *block[1] };

        for(auto b : block) {
                dma->makeAccesibleFromVA(b);
        }

        size_t cur = 0, next = 1;
        std::ios::sync_with_stdio(false);

        // Setup read transfer
        dma->read(*block[0], block[0]->getSize());

        while(true) {
                logger->trace("Read from stream and write to address {}:{:p}",
                              block[next]->getAddrSpaceId(),
                              block[next]->getOffset());
                // We could use the number of interrupts to determine if we
                // missed a chunk of data
                dma->read(*block[next], block[next]->getSize());
                auto c = dma->readComplete();

                if(c.interrupts > 1) {
                        logger->warn("Missed {} interrupts", c.interrupts - 1);
                }

                logger->debug("bytes: {}, intrs: {}, bds: {}",
                              c.bytes,
                              c.interrupts,
                              c.bds);
                try {
                        for(size_t i = 0; i * 4 < c.bytes; i++) {
                                int32_t ival = mem[cur][i];
                                float fval = *(
                                    (float *) (&ival)); // cppcheck-suppress
                                                        // invalidPointerCast
                                formatter->format(fval);
                                printf("%#x\n", ival);
                        }
                        formatter->output(std::cout);
                } catch(const std::exception &e) {
                        logger->warn("Failed to output data: {}", e.what());
                }

                cur = next;
                next = (next + 1) % (sizeof(mem) / sizeof(mem[0]));
        }
}



std::shared_ptr<fpga::Card>
setupCard(const std::string &configFilePath, const std::string &fpgaName)
{
        auto configDir = std::filesystem::path(configFilePath).parent_path();
        std::vector<std::string>  modules {"vfio"};
	auto vfioContainer = std::make_shared<kernel::vfio::Container>(modules);

	// Parse FPGA configuration
	FILE* f = fopen(configFilePath.c_str(), "r");
	if (!f)
		throw RuntimeError("Cannot open config file: {}", configFilePath);

	json_t* json = json_loadf(f, 0, nullptr);
	if (!json) {
		logger->error("Cannot parse JSON config");
		fclose(f);
		throw RuntimeError("Cannot parse JSON config");
	}

	fclose(f);

	json_t* fpgas = json_object_get(json, "fpgas");
	if (fpgas == nullptr) {
		logger->error("No section 'fpgas' found in config");
		exit(1);
	}

	// // Get the FPGA card plugin
	// auto fpgaCardFactory = plugin::registry->lookup<fpga::PCIeCardFactory>("pcie");
	// if (fpgaCardFactory == nullptr) {
	// 	logger->error("No FPGA plugin found");
	// 	exit(1);
	// }

	// Create all FPGA card instances using the corresponding plugin
	auto cards = PlatformCardFactory::make(fpgas, vfioContainer, configDir);

	std::shared_ptr<fpga::Card> card;
	for (auto &fpgaCard : cards) {
		if (fpgaCard->name == fpgaName) {
			return fpgaCard;
		}
	}

	// Deallocate JSON config
	json_decref(json);

	if (!card)
		throw RuntimeError("FPGA card {} not found in config or not working", fpgaName);

	return card;
}

int writeTest(std::shared_ptr<fpga::ip::Dma> dma){
        auto &alloc = villas::HostRam::getAllocator();
        const std::shared_ptr<villas::MemoryBlock> block
                = alloc.allocateBlock(0x200 * sizeof(float));
        villas::MemoryAccessor<float> mem = *block;
        dma->makeAccesibleFromVA(block);

        logger->info("Trying to write Values");

        mem[0] = 1337;

        // Itiate write transfer
        bool state = dma->write(*block, 1 * sizeof(float));
        if(!state)
                logger->error("Failed to write to device");

        volatile auto writeComp = dma->writeComplete();
}

int readTest(std::shared_ptr<fpga::ip::Dma> dma){
        auto &alloc = villas::HostRam::getAllocator();

        const std::shared_ptr<villas::MemoryBlock> block[]
            = { alloc.allocateBlock(0x200 * sizeof(uint32_t)),
                alloc.allocateBlock(0x200 * sizeof(uint32_t)) };

        villas::MemoryAccessor<int32_t> mem[] = { *block[0], *block[1] };

        for(auto b : block) {
                dma->makeAccesibleFromVA(b);
        }

        // Setup read transfer
        dma->read(*block[0], block[0]->getSize());

        size_t cur = 0, next = 1;
        while(true) {
                logger->trace("Read from stream and write to address {}:{:p}",
                              block[next]->getAddrSpaceId(),
                              block[next]->getOffset());
         
                dma->read(*block[next], block[next]->getSize());

                auto c = dma->readComplete();

                logger->debug("bytes: {}, intrs: {}, bds: {}",
                              c.bytes,
                              c.interrupts,
                              c.bds);

                cur = next;
                next = (next + 1) % (sizeof(mem) / sizeof(mem[0]));
        }

}

int main()
{
        logging.setLevel(spdlog::level::debug);
        
        std::shared_ptr<Card> card = setupCard("/home/root/fpga/build/src/fpgas.json","zcu106");

        auto dma = std::dynamic_pointer_cast<fpga::ip::Dma>(
            card->lookupIp(fpga::Vlnv("xilinx.com:ip:axi_dma:")));

        auto axi_switch = std::dynamic_pointer_cast<fpga::ip::AxiStreamSwitch>(
            card->lookupIp(fpga::Vlnv("xilinx.com:ip:axis_switch:")));

        axi_switch->connectInternal("S00_AXIS", "M00_AXIS");

        writeTest(dma);
        //readTest(dma);

        return 0;
}