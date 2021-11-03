/** FPGA card
 *
 * This class represents a FPGA device.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
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

/** @addtogroup fpga VILLASfpga
 * @{
 */

#pragma once

#include <villas/fpga/card.hpp>

namespace villas {
namespace fpga {

/* Forward declarations */
class PCIeCardFactory;

class PCIeCard : public Card {

	friend PCIeCardFactory;

public:
	PCIeCard();
	virtual ~PCIeCard();

public:	// TODO: make this private

	std::shared_ptr<kernel::pci::Device> pdev;	/**< PCI device handle */
};

class PCIeCardFactory : public CardFactory {

public:

	static Card::List
	make(json_t *json, std::shared_ptr<kernel::pci::DeviceList> pci, std::shared_ptr<kernel::vfio::Container> vc);

	static PCIeCard*
	create()
	{ return new PCIeCard(); }

	static Logger
	getStaticLogger()
	{ return villas::logging.get("PCIeCardFactory"); }

	virtual std::string
	getName() const
	{ return "pcie"; }

	virtual std::string
	getDescription() const
	{ return "Xilinx PCIe FPGA cards"; } 
};

} /* namespace fpga */
} /* namespace villas */

/** @} */
