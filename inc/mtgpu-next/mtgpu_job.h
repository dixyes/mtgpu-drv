/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_JOB_H_
#define _MTGPU_JOB_H_

#include "mtgpu_drm.h"

/* redefine them to compatible with dkms */
#define MTFW_DEPEND_Q_SEM_NUM	(32)
#define MTFW_FINISH_Q_SEM_NUM	(8)

struct drm_device;
struct drm_file;
struct _PVRSRV_DEVICE_NODE_;
struct _PVRSRV_RGXDEV_INFO_;
struct _CONNECTION_DATA_;
struct SERVER_MMU_CONTEXT_TAG;
struct mtgpu_fw_info;

struct mtgpu_job_item {
	/* job_item_type must be the first member of mtgpu_job_item */
	u32 job_item_type;

	u64 pool_index;

	struct mtgpu_syncobj *check_syncobjs[MTFW_DEPEND_Q_SEM_NUM];
	u32 check_syncobj_count;
	struct mtgpu_syncobj *update_syncobjs[MTFW_FINISH_Q_SEM_NUM];
	u32 update_syncobj_count;

	struct mtgpu_syncobj *cpu_signal_syncobj;
};

struct mtgpu_job_context {
	PVRSRV_DEVICE_NODE *dev_node;
	u32 type;
	u32 priority;
	struct mutex *lock;
	struct SERVER_MMU_CONTEXT_TAG *server_mmu_context;
	u64 page_table_root_addr;
};

int mtgpu_job_context_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_context_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_submit_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_submit_ioctl_v3(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_append_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_item_cleanup(struct _PVRSRV_RGXDEV_INFO_ *dev_info, u32 *job_item_type);
int mtgpu_notify_queue_update_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_JOB_H_ */
