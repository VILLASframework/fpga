/** 
 * Author: Pascal Henry Bauer <pascal.bauer@rwth-aachen.de>
 * Based on the work of: Steffen Vogel <post@steffenvogel.de> and Daniel Krebs <github@daniel-krebs.net>
 *
 * SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power Systems, EONERC
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#pragma once

#include <villas/fpga/card.hpp>
#include <filesystem>

namespace villas {
namespace fpga {

class PlatformCard : public Card
{
public:
        PlatformCard(std::shared_ptr<kernel::vfio::Container> vfioContainer);
        ~PlatformCard(){};

        void connect();

private:
};

class PlatformCardFactory
{
public:
        static std::list<std::shared_ptr<PlatformCard>> make(json_t *json, 
                                        std::shared_ptr<kernel::vfio::Container> vc,
                                        const std::filesystem::path& searchPath);
};

} /* namespace fpga */
} /* namespace villas */