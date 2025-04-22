/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_UTIL_H_
#define _MTGPU_UTIL_H_

#include "lock_types.h"
#include "img_types.h"
#include "pvrsrv_error.h"

struct _PVRSRV_DEVICE_NODE_;
struct _PVRSRV_RGXDEV_INFO_;
struct work_struct;

struct mtgpu_gpu_util_stats {
	bool	valid;			/* Is the data valid */
	u64	gpu_stat_active;	/* GPU active statistic */
	u64	gpu_stat_idle;		/* GPU idle statistic */
	u64	gpu_stat_cumulative;	/* Sum of active/idle stats */
	u64	timestamp;		/* Timestamp of the most recent sample of the GPU stats */
};

struct mtgpu_gpu_util_item_info {
	u32     dm;
	char *const name;
};

int mtgpu_util_stats_register(void **gpu_util_user);

void mtgpu_util_stats_unregister(void *gpu_util_user);

PVRSRV_ERROR mtgpu_util_config_apphint_query(const struct _PVRSRV_DEVICE_NODE_ *dev_node,
					     const void *private, IMG_UINT32 *value);

PVRSRV_ERROR mtgpu_util_config_apphint_set(const struct _PVRSRV_DEVICE_NODE_ *dev_node,
					   const void *private, IMG_UINT32 value);

void mtgpu_util_delayed_work_func(struct work_struct *work);

int mtgpu_util_info_reset(struct _PVRSRV_DEVICE_NODE_ *dev_node);

int mtgpu_util_stats_get(struct _PVRSRV_DEVICE_NODE_ *dev_node,
			 void *gpu_util_user,
			 struct mtgpu_gpu_util_stats *return_stats);

#endif /* _MTGPU_UTIL_H_ */
