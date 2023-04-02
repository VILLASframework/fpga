
#include <exception>
#include <jansson.h>
#include <unistd.h>
#include <villas/exceptions.hpp>
#include <villas/fpga/ips/dma.hpp>
#include <villas/fpga/platform_card.hpp>
#include <villas/utils.hpp>
#include <villas/fpga/utils.hpp>

#define SEC_IN_USEC 1000 * 1000

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

        while(true) {
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
                logger->debug("Wrote {} bytes", writeComp.bytes);
        }
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

int main()
{
        // Parse FPGA configuration
        const std::string CONFIG_FILE_PATH = "config.json";
        FILE *f = fopen(CONFIG_FILE_PATH.c_str(), "r");

        if(!f)
                throw RuntimeError("Cannot open config file: {}",
                                   CONFIG_FILE_PATH);

        json_t *json = json_loadf(f, 0, nullptr);
        if(!json) {
                fclose(f);
                throw RuntimeError("Cannot parse JSON config");
        }

        fclose(f);

        auto vc = std::make_shared<kernel::vfio::Container>();
        std::shared_ptr<PlatformCard> card
            = PlatformCardFactory::make(json, vc).front();

        auto dma = std::dynamic_pointer_cast<fpga::ip::Dma>(
            card->lookupIp(fpga::Vlnv("xilinx.com:ip:axi_dma:")));

        writeToDmaFromStdIn(dma);
        usleep(5 * SEC_IN_USEC); // some magic numbers

        return 0;
}