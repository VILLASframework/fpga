/** 
 * Author: Pascal Henry Bauer <pascal.bauer@rwth-aachen.de>
 * Based on the work of: Steffen Vogel <post@steffenvogel.de> and Daniel Krebs <github@daniel-krebs.net>
 *
 * SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power Systems, EONERC
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#pragma once

#include <villas/fpga/card.hpp>

namespace villas {
namespace fpga {

class PlatformCard : public Card
{
public:
        PlatformCard(std::shared_ptr<kernel::vfio::Container> vfioContainer);
        ~PlatformCard(){};

        static int Test();

private:
};

} /* namespace fpga */
} /* namespace villas */