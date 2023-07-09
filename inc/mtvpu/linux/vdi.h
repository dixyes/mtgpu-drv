/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _VDI_H_
#define _VDI_H_

/************************************************************************/
/* COMMON REGISTERS                                                     */
/************************************************************************/

#define VPU_PRODUCT_NAME_REGISTER                  (0x1040)
#define VPU_PRODUCT_CODE_REGISTER                  (0x1044)

#define VpuWriteReg(CORE, ADDR, DATA)              vdi_write_register(CORE, ADDR, DATA)
#define VpuReadReg(CORE, ADDR)                     vdi_read_register(CORE, ADDR)
#define VpuWriteMem(CORE, ADDR, DATA, LEN, ENDIAN) vdi_write_memory(CORE, ADDR, DATA, LEN, ENDIAN)
#define VpuReadMem(CORE, ADDR, DATA, LEN, ENDIAN)  vdi_read_memory(CORE, ADDR, DATA, LEN, ENDIAN)

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

#define VDI_128BIT_ENDIAN_MASK 0xf

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
    MEM_TYPE_MAX
} MemTypes;

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

int vdi_init(int coreIdx);
int vdi_release(int coreIdx);

int vdi_lock(int coreIdx);
int vdi_unlock(int coreIdx);
int vdi_disp_lock(int coreIdx);
int vdi_disp_unlock(int coreIdx);

int vdi_hw_reset(int coreIdx);

int vdi_get_instance_num(int coreIdx);
vpu_instance_pool_t *vdi_get_instance_pool(int coreIdx);

int vdi_open_instance(int coreIdx, int inst);
int vdi_close_instance(int coreIdx, int inst);

void vdi_write_register(int coreIdx, Uint32 addr, Uint32 data);
Uint32 vdi_read_register(int coreIdx, Uint32 addr);

int vdi_convert_endian(int coreIdx, int endian);
int vdi_write_memory(int coreIdx, PhysicalAddress addr, Uint8 *data, int len, int endian);
int vdi_read_memory(int coreIdx, PhysicalAddress addr, Uint8 *data, int len, int endian);

void vdi_fio_write_register(int coreIdx, Uint32 addr, Uint32 data);
Uint32 vdi_fio_read_register(int coreIdx, Uint32 addr);

int vdi_get_high_addr(int coreIdx);
int vdi_set_clock_gate(int coreIdx, int enable);

int vdi_get_common_memory(int coreIdx, vpu_buffer_t *vb);
int vdi_get_debug_log_addr(int coreIdx, vpu_buffer_t* vb);
int vdi_get_sram_memory(int coreIdx, vpu_buffer_t *vb);
int vdi_clear_memory(int coreIdx, vpu_buffer_t *vb);

int vdi_allocate_dma_memory(int coreIdx, vpu_buffer_t *vb, int memTypes, int inst);
void vdi_free_dma_memory(int coreIdx, vpu_buffer_t *vb, int memTypes, int inst);
int vdi_attach_dma_memory(int coreIdx, vpu_buffer_t *vb);
int vdi_dettach_dma_memory(int coreIdx, vpu_buffer_t *vb);

int vdi_wait_vpu_busy(int coreIdx, int timeout, Uint32 gdi_busy_flag);
int vdi_wait_bus_busy(int coreIdx, int timeout, Uint32 gdi_busy_flag);
int vdi_wait_vcpu_bus_busy(int coreIdx, int timeout, Uint32 gdi_busy_flag);

void vdi_wait_send_cmd(int coreIdx);
int vdi_wait_interrupt(int coreIdx, int inst, int timeout);
int vdi_wait_interrupt_poll(int coreIdx, int instIdx, int timeout);
int vdi_set_bit_firmware_to_pm(int coreIdx, const Uint16 *code);

void vdi_log(int coreIdx, int inst, int cmd, int step);

#endif
