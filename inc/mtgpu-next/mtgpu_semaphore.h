/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_SEMAPHORE_H_
#define _MTGPU_SEMAPHORE_H_

#include "lock_types.h"

struct drm_device;
struct drm_file;
struct _CONNECTION_DATA_;
struct SYNC_CHECKPOINT_TAG;
struct mtgpu_syncobj;
struct mtgpu_vm_context;
struct mtgpu_semaphore_fence;
struct mtgpu_job_context;
struct drm_mtgpu_semaphore_submit;
struct mtgpu_job_item;

struct mtgpu_semaphore_fence {
	struct dma_fence *base;
	spinlock_t *lock;
	struct mtgpu_sched_job *sched_job;
};

struct mtgpu_semaphore_export_data {
	struct list_head node;
	struct dma_fence *fence;
	struct dma_fence_ops *ops;
	spinlock_t *lock;
	struct SYNC_CHECKPOINT_TAG *sync_checkpoint;
	struct mtgpu_syncobj *syncobj;
	u32 signal_value;
};

extern struct dma_fence_ops *mtgpu_semaphore_fence_ops_ptr;

u64 mtgpu_semaphore_get_gpu_addr(void *semaphore);
u64 mtgpu_semaphore_get_shadow_gpu_addr(void *semaphore);
void *mtgpu_get_semaphore_from_handle(struct _CONNECTION_DATA_ *conn, void *handle);
int mtgpu_semaphore_get_fwaddr(struct _CONNECTION_DATA_ *conn, void *handle, u32 *fw_addr);
void mtgpu_semaphore_signal_fences(void *data);
int mtgpu_semaphore_signal_callback_register(void);
void mtgpu_semaphore_signal_callback_unregister(void);

PVRSRV_ERROR mtgpu_semaphore_resolve(struct _CONNECTION_DATA_ *conn,
				     struct drm_mtgpu_semaphore *check_semas,
				     u32 check_sema_count,
				     u32 *checkpoint_count,
				     struct SYNC_CHECKPOINT_TAG ***checkpoints_out,
				     u32 **values_out,
				     u64 *sem_uid);
struct SYNC_CHECKPOINT_TAG *mtgpu_get_checkpoint_from_handle(struct _CONNECTION_DATA_ *conn, void *handle);
int mtgpu_semaphore_cpu_signal_submit(struct drm_device *drm_dev,
				      struct mtgpu_vm_context *vm_ctx,
				      struct mtgpu_syncobj *syncobj,
				      u64 value);
bool mtgpu_semaphore_is_signaled(struct SYNC_CHECKPOINT_TAG *checkpoint, u32 value);
bool mtgpu_semaphore_is_signaled_v3(struct mtgpu_syncobj *syncobj, u64 value);
int mtgpu_dma_semaphore_submit_v3(struct _CONNECTION_DATA_ *conn,
				  struct drm_mtgpu_semaphore_submit *args,
				  struct mtgpu_job_context *job_ctx);

struct dma_fence *mtgpu_semaphore_fence_get_base(struct mtgpu_semaphore_fence* sem_fence);
int mtgpu_semaphore_fence_ops_init(void);
void mtgpu_semaphore_fence_ops_deinit(void);
int mtgpu_semaphore_fence_acquire(struct mtgpu_syncobj *syncobj,
				  u64 value,
				  struct mtgpu_semaphore_fence **sem_fence_out);  
int mtgpu_semaphore_check_fence_create(struct device *dev,
				       struct mtgpu_syncobj *syncobj,
				       u64 value,
				       struct dma_fence **fence_out);
int mtgpu_semaphore_update_fence_create(struct device *dev,
					struct mtgpu_syncobj *syncobj,
					u64 value,
					struct mtgpu_semaphore_fence **sem_fence_out);
int mtgpu_semaphore_fence_signal(struct device *dev, struct mtgpu_syncobj *syncobj, u64 value,
				 bool cpu_signal, bool guilty);
void mtgpu_semaphore_global_export_fence_signal(struct device *dev,
						struct mtgpu_syncobj *export_syncobj,
						u64 update_value);
void mtgpu_semaphore_global_import_fence_signal(struct device *dev,
						struct mtgpu_syncobj *import_syncobj,
						u64 update_value);
int mtgpu_semaphore_fence_signal_hash_callback(uintptr_t k, uintptr_t v, void* private);

#endif /* _MTGPU_SEMAPHORE_H_ */
