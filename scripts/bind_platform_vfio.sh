#!/bin/bash
#
# Bind Platform fpga to vfio
#
# Author: Pascal Bauer <pascal.bauer@rwth-aachen.de>
# SPDX-FileCopyrightText: 2017 Institute for Automation of Complex Power Systems, EONERC
# SPDX-License-Identifier: Apache-2.0
##################################################################################

modprobe vfio_platform reset_required=0

# Unbind Device from driver
echo a0000000.dma > /sys/bus/platform/drivers/xilinx-vdma/unbind
# Bind device
echo vfio-platform > /sys/bus/platform/devices/a0000000.dma/driver_override
echo a0000000.dma > /sys/bus/platform/drivers/vfio-platform/bind

# Other vfio devices without driver override
echo a0010000.axis_switch > /sys/bus/platform/drivers_probe
