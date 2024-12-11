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
union _MTFW_CCB_ITEM_PARA_;
struct mtgpu_fw_info;
struct mtgpu_vm_context;

struct mtgpu_job_item_semaphore {
	struct mtgpu_syncobj **check_syncobjs;
	u32 check_syncobj_count;
	struct mtgpu_syncobj **update_syncobjs;
	u32 update_syncobj_count;
	struct mtgpu_syncobj *cpu_signal_syncobj;
};

struct mtgpu_job_item {
	/* job_item_type must be the first member of mtgpu_job_item */
	u32 job_item_type;

	u64 pool_index;
	int resp_value;
	bool need_sync;

	struct mtgpu_job_item_semaphore job_semaphore;

	struct list_head node;
	struct mtgpu_job_item_list *job_list;
};

struct mtgpu_job_context {
	PVRSRV_DEVICE_NODE *dev_node;
	u32 type;
	u32 priority;
	struct mutex *lock;
	struct SERVER_MMU_CONTEXT_TAG *server_mmu_context;
	u64 page_table_root_addr;
	struct mtgpu_job_item_list *job_list;
};

int mtgpu_job_submit_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_notify_queue_update_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

int mtgpu_job_context_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_context_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_submit_ioctl_v3(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_append_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_item_cleanup(struct _PVRSRV_RGXDEV_INFO_ *dev_info, u32 *job_item_type);
int mtgpu_job_item_add_semaphore(struct _CONNECTION_DATA_ *conn,
				 struct drm_mtgpu_semaphore *check_semaphores,
				 u32 check_semaphore_count,
				 struct drm_mtgpu_semaphore *update_semaphores,
				 u32 update_semaphore_count,
				 union _MTFW_CCB_ITEM_PARA_ *item_para,
				 struct mtgpu_syncobj ***check_syncobjs_out,
				 struct mtgpu_syncobj ***update_syncobjs_out);
void mtgpu_job_item_semaphore_wake_up(struct mtgpu_job_item_semaphore *job_semaphore);
int mtgpu_job_item_list_create(struct mtgpu_job_item_list **item_list_out);
void mtgpu_job_item_list_destory(struct mtgpu_job_item_list *job_list);
int mtgpu_job_item_list_cleanup_and_destory(struct mtgpu_vm_context *vm_ctx);
void mtgpu_job_item_list_add(struct mtgpu_job_item_list *job_list, struct mtgpu_job_item *job_item);

#endif /* _MTGPU_JOB_H_ */
