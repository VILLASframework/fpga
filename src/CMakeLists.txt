## CMakeLists.txt
#
# Author: Daniel Krebs <github@daniel-krebs.net>
# SPDX-FileCopyrightText: 2018 Institute for Automation of Complex Power Systems, RWTH Aachen University
# SPDX-License-Identifier: Apache-2.0

add_executable(villas-fpga-ctrl villas-fpga-ctrl.cpp)

target_link_libraries(villas-fpga-ctrl PUBLIC
	villas-fpga
)

add_executable(villas-fpga-pipe villas-fpga-pipe.cpp)

target_link_libraries(villas-fpga-pipe PUBLIC
	villas-fpga
)


add_executable(pcimem pcimem.c)
