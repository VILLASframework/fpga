/** Interface to Xilinx System Generator Models via PCIe
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017-2018, Steffen Vogel
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

#include <stdlib.h>
#include <stdint.h>

#include "list.h"

#define XSG_MAPLEN		0x1000
#define XSG_MAGIC		0xDEADBABE

// Forward declaration
struct ip;

enum model_type {
	MODEL_TYPE_HLS,
	MODEL_TYPE_XSG
};

enum model_xsg_block_type {
	XSG_BLOCK_GATEWAY_IN	= 0x1000,
	XSG_BLOCK_GATEWAY_OUT	= 0x1001,
	XSG_BLOCK_INFO		= 0x2000
};

enum model_parameter_type {
	MODEL_PARAMETER_TYPE_UFIX,
	MODEL_PARAMETER_TYPE_FIX,
	MODEL_PARAMETER_TYPE_FLOAT,
	MODEL_PARAMETER_TYPE_BOOLEAN
};

enum model_parameter_direction {
	MODEL_PARAMETER_IN,
	MODEL_PARAMETER_OUT,
	MODEL_PARAMETER_INOUT
};

union model_parameter_value {
	uint32_t ufix;
	int32_t  fix;
	float    flt;
	bool     bol;
};

struct xsg_model {
	uint32_t *map;
	ssize_t maplen;
};

struct hls_model {

};

struct model {
	enum model_type type;			// Either HLS or XSG model

	struct list parameters;			// List of model parameters.
	struct list infos;			// A list of key / value pairs with model details

	union {
		struct xsg_model xsg;		// XSG specific model data
		struct hls_model hls;		// HLS specific model data
	};
};

struct model_info {
	char *field;
	char *value;
};

struct model_parameter {
	// Name of the parameter
	char *name;

	// Read / Write / Read-write?
	enum model_parameter_direction direction;
	// Data type. Integers are represented by MODEL_GW_TYPE_(U)FIX with model_gw::binpt == 0
	enum model_parameter_type type;

	// Binary point for type == MODEL_GW_TYPE_(U)FIX
	int binpt;
	// Register offset to model::baseaddress
	uintptr_t offset;

	union model_parameter_value default_value;

	// A pointer to the model structure to which this parameters belongs to.
	struct fpga_ip *ip;
};

// Initialize a model
int model_init(struct fpga_ip *c);

// Parse model
int model_parse(struct fpga_ip *c, json_t *cfg);

// Destroy a model
int model_destroy(struct fpga_ip *c);

// Print detailed information about the model to the screen.
void model_dump(struct fpga_ip *c);

// Add a new parameter to the model
void model_parameter_add(struct fpga_ip *c, const char *name, enum model_parameter_direction dir, enum model_parameter_type type);

// Remove an existing parameter by its name
int model_parameter_remove(struct fpga_ip *c, const char *name);

/** Read a model parameter.
 *
 * Note: the data type of the register is taken into account.
 * All datatypes are converted to double.
 */
int model_parameter_read(struct model_parameter *p, double *v);

/** Update a model parameter.
 *
 * Note: the data type of the register is taken into account.
 * The double argument will be converted to the respective data type of the
 * GatewayIn/Out block.
 */
int model_parameter_write(struct model_parameter *p, double v);

int model_parameter_update(struct model_parameter *p, struct model_parameter *u);

/** @} */
