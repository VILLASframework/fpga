/** Helper function for directly using VILLASfpga outside of VILLASnode
 *
 * @file
 * Author: Niklas Eiling <niklas.eiling@eonerc.rwth-aachen.de>
 * SPDX-FileCopyrightText: 2022, Steffen Vogel, Niklas Eiling
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#pragma once

#include <string>
#include <villas/fpga/card.hpp>

namespace villas {
namespace fpga {

std::shared_ptr<fpga::PCIeCard>
setupFpgaCard(const std::string &configFile, const std::string &fpgaName);

void configCrossBarUsingConnectString(std::string connectString,
	std::shared_ptr<villas::fpga::ip::Dma> dma,
	std::vector<std::shared_ptr<fpga::ip::AuroraXilinx>>& aurora_channels);

void setupColorHandling();

} /* namespace fpga */
} /* namespace villas */
