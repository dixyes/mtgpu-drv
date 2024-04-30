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
#ifndef _JDI_HPI_H_
#define _JDI_HPI_H_

#include "vputypes.h"
#include "jpuconfig.h"
#include "jpuapi.h"
#include "jpuapifunc.h"

int jdi_write_register(int coreIdx, Uint32 addr, Uint32 data);
Uint32 jdi_read_register(int coreIdx, Uint32 addr);

#define JpuWriteInstReg(CORE_IDX, INST_REG_IDX, ADDR, DATA) \
	jdi_write_register(CORE_IDX, (((Uint32)INST_REG_IDX * NPT_REG_SIZE) + ADDR), DATA)

#define JpuReadInstReg(CORE_IDX, INST_REG_IDX, ADDR) \
	jdi_read_register(CORE_IDX, ((Uint32)INST_REG_IDX * NPT_REG_SIZE) + ADDR)

#define JpuWriteReg(CORE_IDX, ADDR, DATA) \
	jdi_write_register(CORE_IDX, ADDR, DATA)

#define JpuReadReg(CORE_IDX, ADDR) \
	jdi_read_register(CORE_IDX, ADDR)

#define JpuWriteMem(CORE_IDX, ADDR, DATA, LEN, ENDIAN) \
	jdi_write_memory(CORE_IDX, ADDR, DATA, LEN, ENDIAN)

#define JpuReadMem(CORE_IDX, ADDR, DATA, LEN, ENDIAN) \
	jdi_read_memory(CORE_IDX, ADDR, DATA, LEN, ENDIAN)

typedef struct jpu_buffer {
	unsigned int gem_handle;
	unsigned int mem_group_id;
	unsigned int mem_types;
	unsigned long long dev_addr;
	unsigned long long cpu_addr;
	unsigned long long kernel_addr;
	unsigned long long user_addr;
	unsigned int size;
} jpu_buffer_t;

typedef struct jpu_instance_pool {
	unsigned char   jpgInstPool[MAX_NUM_JPU_INSTANCE][MAX_JPU_INST_HANDLE_SIZE];
	Int32           jpu_instance_num;
	BOOL            instance_pool_inited;
	void *instPendingInst;
} jpu_instance_pool_t;

#ifdef CNM_SIM_PLATFORM
typedef struct jpu_sim_context_t {
	Uint32           frameIdx;
	Uint32           instIdx;
} jpu_sim_context_t;
#endif

typedef enum {
    JDI_LITTLE_ENDIAN = 0,
    JDI_BIG_ENDIAN,
    JDI_32BIT_LITTLE_ENDIAN,
    JDI_32BIT_BIG_ENDIAN,
} JpuEndianMode;

typedef enum {
    JDI_LOG_CMD_PICRUN  = 0,
    JDI_LOG_CMD_INIT  = 1,
    JDI_LOG_CMD_RESET  = 2,
    JDI_LOG_CMD_PAUSE_INST_CTRL = 3,
    JDI_LOG_CMD_MAX
} jdi_log_cmd;

void jdi_print_reg_all(int coreIdx);
void jdi_print_reg(int coreIdx, int addr);
int jdi_get_task_num(int coreIdx);
int jdi_init(int coreIdx);
int jdi_release(int coreIdx);
jpu_instance_pool_t *jdi_get_instance_pool(int coreIdx);
int jdi_allocate_dma_memory(int inst_idx, struct jpu_buffer *vb);
void jdi_free_dma_memory(int inst_idx, struct jpu_buffer *vb);
int jdi_wait_interrupt(int timeout, int coreIdx);
int jdi_wait_inst_ctrl_busy(int coreIdx, int timeout, Uint32 addr_flag_reg, Uint32 flag);
int jdi_set_clock_gate(int enable);
int jdi_get_clock_gate(void);
int jdi_hw_reset(void);

#ifdef SUPPORT_STOP_CHECKING
int jdi_wait_pic_status(int coreIdx, int timeout, Uint32 inst_reg_idx, Uint32 flag);
#endif

int jdi_open_instance(int coreIdx, Uint32 inst_idx);
int jdi_close_instance(int coreIdx, Uint32 inst_idx);
int jdi_get_instance_num(int coreIdx);

int jdi_write_memory(int coreIdx, Uint32 addr, Uint8 *data, int len, int endian);
int jdi_read_memory(int coreIdx, Uint32 addr, Uint8 *data, int len, int endian);

int jdi_lock(int coreIdx);
void jdi_unlock(int coreIdx);
void jdi_log(int cmd, int step, int inst);
void jdi_delay_us(Uint32 us);

#endif //#ifndef _JDI_HPI_H_
