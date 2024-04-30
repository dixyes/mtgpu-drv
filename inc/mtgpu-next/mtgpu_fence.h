/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_FENCE_H_
#define _MTGPU_FENCE_H_

#include "os-interface.h"

struct drm_device;
struct drm_file;
struct _CONNECTION_DATA_;
struct drm_mtgpu_fence;
struct _HASH_TABLE_;

//#define MTGPU_FENCE_DEBUG

#if defined(MTGPU_FENCE_DEBUG)
#define MTGPU_FENCE_CTX_TRACE(c, fmt, ...)							\
	do {											\
		struct mtgpu_fence_context *__fctx = (c);					\
		os_pr_err("c %llu: (MTGPU) " fmt, (u64)__fctx->fence_context, ##__VA_ARGS__);	\
	} while (0)
#else
#define MTGPU_FENCE_CTX_TRACE(c, fmt, ...)
#endif

#if defined(MTGPU_FENCE_DEBUG)
#define MTGPU_FENCE_TRACE(f, fmt, args...) \
	do {								\
		struct dma_fence *__ff = (f);				\
		os_pr_err("f %llu#%llu: " fmt,				\
			  os_get_dma_fence_context(__ff),		\
			  os_get_dma_fence_seqno(__ff),			\
			  ##args);					\
	} while (0)
#else
#define MTGPU_FENCE_TRACE(f, fmt, ...)
#endif

struct mtgpu_fence {
	struct dma_fence *base;
	struct mtgpu_fence_context *fctx;
	char name[32];

	struct dma_fence *fence;
	struct SYNC_CHECKPOINT_TAG *sync_checkpoint;

	u32 dep_checkpoint_count;
	struct SYNC_CHECKPOINT_TAG **dep_checkpoints;

	struct list_head fence_head;
	struct list_head signal_head;
	struct dma_fence_cb *cb;
	struct rcu_head rcu;
};

struct mtgpu_fence_context {
	spinlock_t *lock;
	char name[32];
	/* TODO: debug request */
	// void *dbg_request_handle;
	u64 fence_context;
	atomic64_t fence_seqno;

	struct workqueue_struct *fence_wq;
	struct work_struct *check_status_work;

	void *cmd_complete_handle;

	spinlock_t *list_lock;
	/* TODO: in debug request */
	struct list_head fence_list;
	struct list_head signal_list;
	struct list_head deferred_free_list;

	/*
	 * Pointer to handle hash table.
	 *  The hash table is used to do reverse lookups, converting data
	 *  pointers to handles.
	 */
	struct _HASH_TABLE_ *hash_table;
	struct SYNC_CHECKPOINT_TAG *shared_checkpoint;
	atomic64_t latest_seqno;

	mt_kref kref;
	struct work_struct *destroy_work;
};

int mtgpu_fence_ops_init(void);
void mtgpu_fence_ops_deinit(void);
void mtgpu_fence_context_free_deferred(struct mtgpu_fence_context *fctx);
void mtgpu_fence_context_destroy(struct mtgpu_fence_context *fctx);
struct mtgpu_fence_context *
mtgpu_fence_context_create(struct workqueue_struct *fence_status_wq,
			   const char *name);
void mtgpu_fence_prepare_name(char *fence_name, size_t fence_name_size,
			      const char *name, u64 seqno);
struct mtgpu_fence *mtgpu_fence_create(struct mtgpu_fence_context *fctx,
				       struct SYNC_CHECKPOINT_CONTEXT_TAG *sync_checkpoint_ctx,
				       int timeline_fd, const char *name);
void mtgpu_fence_destroy(struct mtgpu_fence *mtgpu_fence);
bool is_mtgpu_local_fence(struct dma_fence *fence);
struct mtgpu_fence *mtgpu_fence_get(struct _CONNECTION_DATA_ *conn, struct drm_mtgpu_fence *fence);
int mtgpu_fence_free(struct _CONNECTION_DATA_ *conn, struct drm_mtgpu_fence *fence);
int mtgpu_fence_get_checkpoints(struct mtgpu_fence **mtgpu_fences, u32 nr_fences,
				struct SYNC_CHECKPOINT_TAG **fence_checkpoints);
struct mtgpu_fence *
mtgpu_create_fence_from_fence(struct mtgpu_fence_context *fctx,
			      struct SYNC_CHECKPOINT_CONTEXT_TAG *sync_checkpoint_ctx,
			      struct dma_fence *fence, PVRSRV_FENCE fence_fd, const char *name);

int mtgpu_fence_wait_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_fence_to_fd_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_timeline_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_timeline_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_timeline_read_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_FENCE_H_ */
