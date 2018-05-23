// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2017.3
// Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
// 
// ==============================================================

// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of baseaddr
//        bit 31~0 - baseaddr[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of data_offset
//        bit 31~0 - data_offset[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of doorbell_offset
//        bit 31~0 - doorbell_offset[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of status_i
//        bit 31~0 - status_i[31:0] (Read/Write)
// 0x2c : reserved
// 0x30 : Data signal of status_o
//        bit 31~0 - status_o[31:0] (Read)
// 0x34 : reserved
// 0x38 : Data signal of frame_size
//        bit 31~0 - frame_size[31:0] (Read/Write)
// 0x3c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XRTDS2GPU_CTRL_ADDR_AP_CTRL              0x00
#define XRTDS2GPU_CTRL_ADDR_GIE                  0x04
#define XRTDS2GPU_CTRL_ADDR_IER                  0x08
#define XRTDS2GPU_CTRL_ADDR_ISR                  0x0c
#define XRTDS2GPU_CTRL_ADDR_BASEADDR_DATA        0x10
#define XRTDS2GPU_CTRL_BITS_BASEADDR_DATA        32
#define XRTDS2GPU_CTRL_ADDR_DATA_OFFSET_DATA     0x18
#define XRTDS2GPU_CTRL_BITS_DATA_OFFSET_DATA     32
#define XRTDS2GPU_CTRL_ADDR_DOORBELL_OFFSET_DATA 0x20
#define XRTDS2GPU_CTRL_BITS_DOORBELL_OFFSET_DATA 32
#define XRTDS2GPU_CTRL_ADDR_STATUS_I_DATA        0x28
#define XRTDS2GPU_CTRL_BITS_STATUS_I_DATA        32
#define XRTDS2GPU_CTRL_ADDR_STATUS_O_DATA        0x30
#define XRTDS2GPU_CTRL_BITS_STATUS_O_DATA        32
#define XRTDS2GPU_CTRL_ADDR_FRAME_SIZE_DATA      0x38
#define XRTDS2GPU_CTRL_BITS_FRAME_SIZE_DATA      32

