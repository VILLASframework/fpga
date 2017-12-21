/** Interlectual Property component.
 *
 * This class represents a module within the FPGA.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
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

#ifndef VILLAS_IP_HPP
#define VILLAS_IP_HPP

#include "fpga/vlnv.hpp"
#include "plugin.hpp"

#include <map>
#include <list>
#include <memory>

#include <jansson.h>


namespace villas {
namespace fpga {

class PCIeCard;

namespace ip {


class IpIdentifier {
public:
	IpIdentifier(Vlnv vlnv = Vlnv::getWildcard(), std::string name = "") :
	    vlnv(vlnv), name(name) {}

	IpIdentifier(std::string vlnvString, std::string name = "") :
	    vlnv(vlnvString), name(name) {}

	friend std::ostream&
	operator<< (std::ostream& stream, const IpIdentifier& id)
	{ return stream << " Name: " << id.name << "(VLNV: " << id.vlnv << ")"; }

	Vlnv vlnv;
	std::string name;
};

using IpDependency = std::pair<std::string, Vlnv>;

// forward declarations
class IpCoreFactory;

class IpCore {
public:

	friend IpCoreFactory;

	IpCore() : card(nullptr), baseaddr(0), irq(-1) {}
	virtual ~IpCore() {}

	// IPs can implement this interface
	virtual bool check() { return true; }
	virtual bool start() { return true; }
	virtual bool stop()  { return true; }
	virtual bool reset() { return true; }
	virtual void dump();

	bool
	operator== (const IpIdentifier& otherId) {
		const bool vlnvMatch = id.vlnv == otherId.vlnv;
		const bool nameWildcard = id.name.empty() or otherId.name.empty();

		return vlnvMatch and (nameWildcard or id.name == otherId.name);
	}

	bool
	operator== (const Vlnv& otherVlnv)
	{ return id.vlnv == otherVlnv; }

	friend std::ostream&
	operator<< (std::ostream& stream, const IpCore& ip)
	{ return stream << ip.id; }

protected:
	uintptr_t
	getBaseaddr() const;

protected:
	// populated by FpgaIpFactory
	PCIeCard* card;		/**< FPGA card this IP is instantiated on */	
	IpIdentifier id;	/**< VLNV and name defined in JSON config */
	uintptr_t baseaddr;	/**< The baseadress of this FPGA IP component */
	int irq;			/**< The interrupt number of the FPGA IP component. */

	std::map<std::string, IpCore*> dependencies;
};


using IpCoreList = std::list<std::unique_ptr<IpCore>>;


class IpCoreFactory : public Plugin {
public:
	IpCoreFactory(std::string concreteName) :
	    Plugin(std::string("FPGA IpCore Factory: ") + concreteName)
	{ pluginType = Plugin::Type::FpgaIp; }

	/// Returns a running and checked FPGA IP
	static IpCoreList
	make(PCIeCard* card, json_t *json_ips);

private:
	/// Create a concrete IP instance
	virtual IpCore* create() = 0;

	/// Configure IP instance from JSON config
	virtual bool configureJson(const std::unique_ptr<IpCore>& ip, json_t *json)
	{ return true; }

	virtual Vlnv getCompatibleVlnv() const = 0;
	virtual std::string getName() const = 0;
	virtual std::string getDescription() const = 0;
	virtual std::list<IpDependency> getDependencies() const = 0;

private:
	static IpCoreFactory*
	lookup(const Vlnv& vlnv);
};

/** @} */

} // namespace ip
} // namespace fpga
} // namespace villas

#endif // VILLAS_IP_HPP
