/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_PSTATE_H__
#define __MTGPU_PSTATE_H__

#include "mtgpu.h"

#define PSTATE_STATUS_OK		(0)
#define PSTATE_STATUS_ERROR		(1)  /* A generic error happens */
#define PSTATE_STATUS_ETIME		(2)  /* Timed out */
#define PSTATE_STATUS_EFULL		(3)  /* The resource is full */
#define PSTATE_STATUS_EEMPTY		(4)  /* The resource is empty */
#define PSTATE_STATUS_ENOMEM		(5)  /* Out of memory */
#define PSTATE_STATUS_EPERM		(6)  /* Operation not permitted */
#define PSTATE_STATUS_EBUSY		(7)  /* The resource is busy */
#define PSTATE_STATUS_EFAULT		(8)  /* Bad address */
#define PSTATE_STATUS_EAGAIN		(9)  /* Try again */
#define PSTATE_STATUS_EINVAL		(10) /* Invalid argument */
#define PSTATE_STATUS_EUNSUPPORT	(64) /* not support */

/**
 * enum pstate_lvl  - Describe the pstate level
 * @PSTATE_LVL_P0: graphic(full perf):PC gaming/maximum 3D
 * @PSTATE_LVL_P12: short idle(static screen)
 */
enum pstate_lvl {
	PSTATE_LVL_NULL     = -1,
	PSTATE_LVL_P0       = 0,
	PSTATE_LVL_P1       = 1,
	PSTATE_LVL_P2       = 2,
	PSTATE_LVL_P3       = 3,
	PSTATE_LVL_P4       = 4,
	PSTATE_LVL_P5       = 5,
	PSTATE_LVL_P6       = 6,
	PSTATE_LVL_P7       = 7,
	PSTATE_LVL_P8       = 8,
	PSTATE_LVL_P9       = 9,
	PSTATE_LVL_P10      = 10,
	PSTATE_LVL_P11      = 11,
	PSTATE_LVL_P12      = 12,
	PSTATE_LVL_P13      = 13,
	PSTATE_LVL_P14      = 14,
	PSTATE_LVL_P15      = 15,
	PSTATE_LVL_MAX      = 0xFFFF,
};

enum mtgpu_pstate_event {
	MTGPU_PSTATE_EVENT_NONE,
	MTGPU_PSTATE_EVENT_GPU_IDLE,
	MTGPU_PSTATE_EVENT_GPU_ACTIVE,
	MTGPU_PSTATE_EVENT_VIDEO_IDLE,
	MTGPU_PSTATE_EVENT_VIDEO_ACTIVE,
	MTGPU_PSTATE_EVENT_DISP_IDLE,
	MTGPU_PSTATE_EVENT_DISP_ACTIVE,
	MTGPU_PSTATE_EVENT_ALL_IDLE,
	MTGPU_PSTATE_EVENT_ALL_ACTIVE,
};

enum mtgpu_pstate_mode {
	MTGPU_PSTATE_MODE_PERF = 0,
	MTGPU_PSTATE_MODE_NORMAL,
	MTGPU_PSTATE_MODE_INVALID,
};

int mtgpu_pstate_update(struct mtgpu_device *mtdev);
int mtgpu_pstate_sync_with_smc(struct mtgpu_device *mtdev);
int mtgpu_pstate_notifier_call_chain(struct mtgpu_device *mtdev, unsigned long event, void *data);
void mtgpu_pstate_enable_timer(struct mtgpu_device *mtdev);
int mtgpu_pstate_init(struct mtgpu_device *mtdev);
void mtgpu_pstate_exit(struct mtgpu_device *mtdev);
int mtgpu_pstate_get_mode(struct mtgpu_device *mtdev);
void mtgpu_pstate_set_mode(struct mtgpu_device *mtdev, uint32_t pstate_mode);
u32 mtgpu_pstate_get_count(struct mtgpu_device *mtdev, enum pstate_lvl pstate_lvl);
int mtgpu_pstate_status_to_error(int pstate_status);

#endif /* __MTGPU_PSTATE_H__ */
