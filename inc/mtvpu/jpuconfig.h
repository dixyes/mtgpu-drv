//-----------------------------------------------------------------------------
// COPYRIGHT (C) 2020   CHIPS&MEDIA INC. ALL RIGHTS RESERVED
// 
// This file is distributed under BSD 3 clause and LGPL2.1 (dual license)
// SPDX License Identifier: BSD-3-Clause
// SPDX License Identifier: LGPL-2.1-only
// 
// The entire notice above must be reproduced on all authorized copies.
// 
// Description  : 
//-----------------------------------------------------------------------------
#ifndef _JPU_CONFIG_H_
#define _JPU_CONFIG_H_

#include "../config.h"

#define MAX_NUM_JPU_INSTANCE            8
#define MAX_FRAME                       1
#define MAX_JPU_BUFFER_POOL             32
#define MAX_NUM_REGISTER_SET            4 /* fixed value(do not modified) */
#define MAX_JPU_INST_HANDLE_SIZE        48

#define JPU_FRAME_ENDIAN                JDI_LITTLE_ENDIAN
#define JPU_STREAM_ENDIAN               JDI_LITTLE_ENDIAN
#define JDI_SYSTEM_ENDIAN 				JDI_LITTLE_ENDIAN

/* 0 (chroma separate mode), 1 (cbcr interleave mode), 2 (crcb interleave mode) */
#define JPU_CHROMA_INTERLEAVE           1

/* 0 : ON ("0xFF"), 1 : OFF ("0x00") for stuffing */
#define JPU_STUFFING_BYTE_FF            0

#define MAX_MJPG_PIC_WIDTH              32768
#define MAX_MJPG_PIC_HEIGHT             32768
#define STREAM_FILL_SIZE                0x10000
#define STREAM_END_SIZE                 0
#define JPU_GBU_SIZE                    256
#define STREAM_BUF_SIZE                 (2 << 20)
#define JPU_CHECK_WRITE_RESPONSE_BVALID_SIGNAL 0
#define JPU_INTERRUPT_TIMEOUT_MS        5000
#define JPU_INST_CTRL_TIMEOUT_MS        5000

#endif    /* __JPU_CONFIG_H__ */
