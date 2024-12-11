//-----------------------------------------------------------------------------
// COPYRIGHT (C) 2023 Moore Threads Ltd. ALL RIGHTS RESERVED
//
// This file is distributed under BSD 3 clause and LGPL2.1 (dual license)
// SPDX License Identifier: BSD-3-Clause
// SPDX License Identifier: LGPL-2.1-only
//
// The entire notice above must be reproduced on all authorized copies.
//
// Description  :
//-----------------------------------------------------------------------------

#ifndef _VDI_H_
#define _VDI_H_

#include "vputypes.h"
#include "vpuconfig.h"
#include "vpudbg.h"
/************************************************************************/
/* COMMON REGISTERS                                                     */
/************************************************************************/
#define VPU_PRODUCT_NAME_REGISTER                 (0x1040)
#define VPU_PRODUCT_CODE_REGISTER                 (0x1044)
#define VPU_PRODUCT_NAME                          (0x090)
#define VPU_PRODUCT_VERSION                       (0x094)
#define VDI_128BIT_ENDIAN_MASK                    (0xf)
#define VDI_LOW_16BIT_MASK                        (0xFFFF)
#define VDI_HIGH_16BIT_MASK                       (0xFFFF0000)
#define VDI_LOW_32BIT_MASK                        (0xFFFFFFFFULL)
#define VDI_HIGH_32BIT_MASK                       (0xFFFFFFFF00000000ULL)

#define VDI_VPU_READY                             (0)
#define VDI_MCU_READY                             (1)

#define VpuWriteReg(CORE, ADDR, DATA)              vdi_write_register(CORE, (Uint32)(ADDR), (Uint32)(DATA))
#define VpuReadReg(CORE, ADDR)                     vdi_read_register(CORE, (Uint32)(ADDR))
#define VpuWriteMem(CORE, VPU_BUFFER, OFFSET, DATA, LEN, ENDIAN)    \
        vdi_write_memory(CORE, VPU_BUFFER, OFFSET, DATA, LEN, ENDIAN)
#define VpuReadMem(CORE, VPU_BUFFER, OFFSET, DATA, LEN, ENDIAN)     \
        vdi_read_memory(CORE, VPU_BUFFER, OFFSET, DATA, LEN, ENDIAN)

#define GET_HIGH_16BITS(addr) ((unsigned short)(((unsigned int)(addr) >> 16) & 0xFFFF))
#define GET_LOW_16BITS(addr) ((unsigned short)((unsigned int)(addr) & 0xFFFF))
#define ABS_DIFF(x, y) ((x) > (y) ? (x) - (y) : (y) - (x))

typedef enum {
    VDI_LITTLE_ENDIAN = 0,      /* 64bit LE */
    VDI_BIG_ENDIAN,             /* 64bit BE */
    VDI_32BIT_LITTLE_ENDIAN,
    VDI_32BIT_BIG_ENDIAN,
    /* WAVE PRODUCTS */
    VDI_128BIT_LITTLE_ENDIAN    = 16,
    VDI_128BIT_LE_BYTE_SWAP,
    VDI_128BIT_LE_WORD_SWAP,
    VDI_128BIT_LE_WORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_SWAP,
    VDI_128BIT_LE_DWORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_WORD_SWAP,
    VDI_128BIT_LE_DWORD_WORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_WORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_WORD_SWAP,
    VDI_128BIT_BE_DWORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_SWAP,
    VDI_128BIT_BE_WORD_BYTE_SWAP,
    VDI_128BIT_BE_WORD_SWAP,
    VDI_128BIT_BE_BYTE_SWAP,
    VDI_128BIT_BIG_ENDIAN        = 31,
    VDI_ENDIAN_MAX
} EndianMode;

typedef enum {
    DEC_TASK      = 0,
    DEC_WORK      = 1,
    DEC_FBC       = 2,
    DEC_FBCY_TBL  = 3,
    DEC_FBCC_TBL  = 4,
    DEC_BS        = 5,
    DEC_FB_LINEAR = 6,
    DEC_MV        = 7,
    DEC_ETC       = 8,
    DEC_COMMON    = 9,
    DEC_VA_PARAM  = 10,
    ENC_TASK      = 50,
    ENC_WORK      = 51,
    ENC_FBC       = 52,
    ENC_FBCY_TBL  = 53,
    ENC_FBCC_TBL  = 54,
    ENC_BS        = 55,
    ENC_SRC       = 56,
    ENC_MV        = 57,
    ENC_DEF_CDF   = 58,
    ENC_SUBSAMBUF = 59,
    ENC_ETC       = 60,
    FW_BUFFER     = 61,
    MEM_TYPE_MAX
} MemTypes;

enum {
	VDI_TYPE_SUDI,
	VDI_TYPE_QUYU1,
	VDI_TYPE_QUYU2,
	VDI_TYPE_QUYU1_GUEST,
	VDI_TYPE_QUYU2_GUEST,
	VDI_TYPE_APOLLO,
	VDI_TYPE_PIHU1,
	VDI_TYPE_PIHU2,
};

typedef struct vpu_buffer {
    Uint32 size;
    Uint64 phys_addr;
    Uint64 base;
    Uint64 virt_addr;
} vpu_buffer_t;

typedef struct vpu_instance_pool {
    /* Since VDI don't know the size of CodecInst structure, VDI should have the enough space not to overflow. */
    Uint8        codecInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
    vpu_buffer_t vpu_common_buffer;
    int          vpu_instance_num;
    int          instance_pool_inited;
    void *       pendingInst;
    int          pendingInstIdxPlus1;
    Uint32       lastPerformanceCycles;
} vpu_instance_pool_t;

#if defined (__cplusplus)
extern "C" {
#endif
void vdi_open_prepare(Uint32 coreIdx, Uint32 instIdx, int drm_id, int pool_id, int vcore_base,
        		      void *mmu_ctx);
int vdi_init(Uint32 coreIdx);
int vdi_release(Uint32 coreIdx);

int vdi_lock(Uint32 coreIdx);
int vdi_trylock(Uint32 coreIdx);
void vdi_unlock(Uint32 coreIdx);
int vdi_disp_lock(Uint32 coreIdx);
void vdi_disp_unlock(Uint32 coreIdx);

int vdi_get_instance_num(Uint32 coreIdx);
vpu_instance_pool_t *vdi_get_instance_pool(Uint32 coreIdx);

int vdi_open_instance(Uint32 coreIdx, Uint32 instIdx);
int vdi_close_instance(Uint32 coreIdx, Uint32 instIdx);

Uint32 vdi_get_chip_type(Uint32 coreIdx);
Uint32 vdi_is_mcu_core(Uint32 coreIdx);

void vdi_write_register(Uint32 coreIdx, Uint32 addr, Uint32 data);
Uint32 vdi_read_register(Uint32 coreIdx, Uint32 addr);
void vdi_fio_write_register(Uint32 coreIdx, Uint32 addr, Uint32 data);
Uint32 vdi_fio_read_register(Uint32 coreIdx, Uint32 addr);

int vdi_convert_endian(Uint32 coreIdx, int endian);
int vdi_write_memory(Uint32 coreIdx, vpu_buffer_t *vb, Uint32 offset,  Uint8 *data, int len, int endian);
int vdi_read_memory(Uint32 coreIdx, vpu_buffer_t *vb, Uint32 offset, Uint8 *data, int len, int endian);
int vdi_clear_memory(Uint32 coreIdx, vpu_buffer_t *vb);
void vdi_reset_memory(Uint32 coreIdx, vpu_buffer_t *vb);

int vdi_get_common_memory(Uint32 coreIdx, vpu_buffer_t *vb);
int vdi_allocate_dma_memory(Uint32 coreIdx, vpu_buffer_t *vb, int memTypes, Uint32 instIdx);
int vdi_attach_dma_memory(Uint32 coreIdx, vpu_buffer_t *vb);
int vdi_dettach_dma_memory(Uint32 coreIdx, vpu_buffer_t *vb);
void vdi_free_dma_memory(Uint32 coreIdx, vpu_buffer_t *vb, int memTypes, Uint32 instIdx);

int vdi_get_sram_memory(Uint32 coreIdx, vpu_buffer_t *vb);
int vdi_get_debug_log_addr(Uint32 coreIdx, vpu_buffer_t* vb);

int vdi_hw_reset(Uint32 coreIdx);
int vdi_set_clock_gate(Uint32 coreIdx, int enable);
int vdi_get_vcpu_high_addr(Uint32 coreIdx);
int vdi_get_vcore_high_addr(Uint32 coreIdx, Uint32 instid);
int vdi_set_bit_firmware_to_pm(Uint32 coreIdx, const Uint16 *code);

int vdi_wait_interrupt(Uint32 coreIdx, Uint32 instIdx, int timeout);
int vdi_wait_interrupt_poll(Uint32 coreIdx, Uint32 instIdx, int timeout);
int vdi_wait_vpu_busy(Uint32 coreIdx, int timeout, Uint32 wait_addr, Uint32 wait_value);
void vdi_delayms(int timeout);
int vdi_wait_bus_busy(Uint32 coreIdx, int timeout, Uint32 busy_flag);
int vdi_wait_vcpu_bus_busy(Uint32 coreIdx, int timeout, Uint32 busy_flag);

void vdi_log(Uint32 coreIdx, Uint32 instIdx, int cmd, int step);
int vdi_check_inst_idx_valid(Uint32 coreIdx, Uint32 instIdx);

void vdi_byte_swap(unsigned char *data, int len);
void vdi_word_swap(unsigned char *data, int len);
void vdi_dword_swap(unsigned char* data, int len);
void vdi_lword_swap(unsigned char *data, int len);

void vdi_memset(void *addr, Uint8 data, Uint32 size);

#if defined(_MSC_VER)
/* todo: remove these APIs for windows internal use */
int vdi_clear(Uint32 coreIdx, Uint32 core_count);
int vdi_fix_vpu_40bit(Uint32 coreIdx);
#endif

#if defined (__cplusplus)
}
#endif

#endif //#ifndef _VDI_H_
