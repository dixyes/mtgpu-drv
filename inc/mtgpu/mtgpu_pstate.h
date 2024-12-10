/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_PSTATE_H__
#define __MTGPU_PSTATE_H__

#include "mtgpu.h"

enum mtgpu_pstate_event {
	MTGPU_PSTATE_EVENT_NONE,
	MTGPU_PSTATE_EVENT_GFX_IDLE,
	MTGPU_PSTATE_EVENT_GFX_ACTIVE,
	MTGPU_PSTATE_EVENT_VIDEO_IDLE,
	MTGPU_PSTATE_EVENT_VIDEO_ACTIVE,
	MTGPU_PSTATE_EVENT_DISP_IDLE,
	MTGPU_PSTATE_EVENT_DISP_ACTIVE,
	MTGPU_PSTATE_EVENT_ALL_IDLE,
	MTGPU_PSTATE_EVENT_ALL_ACTIVE,
};

enum mtgpu_pstate_mode {
	MTGPU_PSTATE_MODE_PERF,
	MTGPU_PSTATE_MODE_NORMAL,
};

int mtgpu_pstate_update(struct mtgpu_device *mtdev);
int mtgpu_pstate_notifier_call_chain(struct mtgpu_device *mtdev,
				     unsigned long event, void *data);
void mtgpu_pstate_enable_timer(struct mtgpu_device *mtdev);
void mtgpu_pstate_init(struct mtgpu_device *mtdev);
void mtgpu_pstate_exit(struct mtgpu_device *mtdev);
void mtgpu_pstate_set_mode(struct mtgpu_device *mtdev, uint32_t pstate_mode);

struct kernel_param;

#endif /* __MTGPU_PSTATE_H__ */
