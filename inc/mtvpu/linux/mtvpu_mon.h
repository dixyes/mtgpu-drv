/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTVPU_MON_H__
#define __MTVPU_MON_H__

struct timer_list;

struct mt_inst_info {
	u32 pid;           /* Process Id, 0 represents an idle session */
	u32 hori_res;      /* Horizontal Resolution (pixel) */
	u32 vert_res;      /* Vertical Resolution (pixel) */
	u32 frame_rate;    /* Number of frames per second */
	u32 bit_rate;      /* Number of bits per second */
	u32 latency;       /* Codec latency, in microsecond */
	CodStd stream_type;/* Type of stream, e.g. H.265/AV1/etc. */
	u64 vcore_mem_base;/* Each instance has a 4G group base for some instance memory */
};

struct mt_inst_extra {
	u64 frames;
	u64 frame_cycle;
	u32 stream_size;
	u32 max_cycle;
	u32 min_cycle;
	u32 mem_alloc;
	u32 mem_free;
};

void vpu_monitor(struct timer_list *timer_list);
#endif
