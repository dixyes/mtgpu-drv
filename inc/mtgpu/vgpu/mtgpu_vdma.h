/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_VDMA_H__
#define __MTGPU_VDMA_H__

#include "os-interface.h"

#include "mtgpu.h"
#include "mtgpu_dma.h"

#define VDMA_MAX_BATCH_COMMAND_COUNT	1
#define VDMA_MAX_DESCS_PER_CHAN		0x7F
#define DEFAULT_VDMA_HW_DESC_COUNT	10240

struct mtgpu_vdma_xfer_desc {
	struct mtgpu_dma_xfer_desc *dma_desc;
	u32 desc_cnt;
	u32 size;
	u32 type;
	u32 direction;
	u32 process_id;
};

struct mtgpu_vdma_command_info {
	u32 size;
	u32 direction;
	u32 process_id;
	u32 cnt;
	u32 type;
	struct mtgpu_dma_xfer_desc dma_descs[VDMA_MAX_DESCS_PER_CHAN];
};

struct mtgpu_vdma_chan_data {
	u64 vdma_interrupt_write_count;
	u64 vdma_interrupt_read_count;
	s32 int_status;

	struct mtgpu_vdma_command_info vdma_command_info;
};

struct mtgpu_vdma_shared_buffer {
	volatile u32 vdma_enable;
	volatile struct mtgpu_vdma_chan_data chan_data[VDMA_GUEST_CHAN_MAX_NUM];
};

struct mtgpu_vdma_pref {
	u64 start_time;
	u32 perf_state;
	u32 vdma_count[VDMA_GUEST_CHAN_MAX_NUM];
	u32 hw_dma_desc_count[VDMA_GUEST_CHAN_MAX_NUM];
	u64 vdma_size[VDMA_GUEST_CHAN_MAX_NUM];
	u64 vdma_hw_time[VDMA_GUEST_CHAN_MAX_NUM];
	u64 vdma_prepare_time[VDMA_GUEST_CHAN_MAX_NUM];
};

#define VDMA_PERF_DISABLE 0
#define VDMA_PERF_START 1
#define VDMA_PERF_END 2
#define VDMA_PERF_LOG 3
#define VDMA_INFO_LOG 4

struct mtgpu_vdma {
	struct mtgpu_device *mtdev;
	u32 osid;
	u32 mpc_id;
	u32 instance_id;
	u32 last_chan;
	struct kvm *kvm;
	spinlock_t *vdma_status_lock;
	struct mtgpu_vdma_pref vdma_perf;

	int chan_status[VDMA_GUEST_CHAN_MAX_NUM];
	struct mtgpu_vdma_xfer_desc xfer_descs[VDMA_GUEST_CHAN_MAX_NUM];

	struct mtgpu_vdma_shared_buffer *vdma_buffer;

	struct work_struct *transfer_work;
};

/* do something for vdma initialize */
int mtgpu_vdma_init(struct mtgpu_vdma *vdma);

/* do something for vdma deinitialize */
void mtgpu_vdma_deinit(struct mtgpu_vdma *vdma);

/* set vdma command info address */
void mtgpu_vdma_set_buffer(struct mtgpu_vdma *vdma, u64 command_info_gpa);

/* unset vdma command info address */
void mtgpu_vdma_unset_buffer(struct mtgpu_vdma *vdma);

/* parse vdma command and prepare vdma description */
int mtgpu_push_vdma_desc(struct mtgpu_vdma *vdma, int chan);

#endif /* __MTGPU_VDMA_H__ */
