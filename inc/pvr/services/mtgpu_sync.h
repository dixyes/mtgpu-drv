/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_SYNC_H__
#define __MTGPU_SYNC_H__

#include "opaque_types.h"

#define MTGPU_MAX_FENCE_COUNT_PER_CONTEXT	(1024)

struct drm_mtgpu_fence;
struct mtgpu_fence_context;
struct mtgpu_fence;
struct drm_file;

struct _CONNECTION_DATA_;

struct mtgpu_timeline {
	char name[32];
	struct mtgpu_fence_context *fence_context;
};

static inline u32 mtgpu_sync_seqno_to_index(u64 seqno)
{
	return (seqno - 1) % MTGPU_MAX_FENCE_COUNT_PER_CONTEXT;
}

int mtgpu_sync_init(void);
void mtgpu_sync_deinit(void);
struct mtgpu_fence_context *mtgpu_sync_get_foreign_fence_context(void);

PVRSRV_ERROR mtgpu_sync_create_context(PPVRSRV_DEVICE_NODE dev_node,
				       struct mtgpu_timeline *timeline);

PVRSRV_ERROR
mtgpu_sync_create_fence(PPVRSRV_DEVICE_NODE dev_node,
			struct _CONNECTION_DATA_ *conn,
			PSYNC_CHECKPOINT_CONTEXT sync_checkpoint_ctx,
			struct drm_mtgpu_fence *fence,
			const char *fence_name,
			u64 *fence_uid,
			PSYNC_CHECKPOINT *sync_checkpoint_out);
PVRSRV_ERROR
mtgpu_sync_resolve_fence(PSYNC_CHECKPOINT_CONTEXT sync_checkpoint_ctx,
			 struct _CONNECTION_DATA_ *conn,
			 struct drm_mtgpu_fence *update_fence,
			 struct drm_mtgpu_fence *check_fences,
			 u32 check_fence_count,
			 s32 foreign_fence_fd,
			 u32 *checkpoint_count,
			 PSYNC_CHECKPOINT **checkpoints_out,
			 u64 **seqnos_out,
			 u64 *fence_uid);

bool mtgpu_sync_fence_is_signaled(struct mtgpu_fence_context *fctx, u64 query_seqno);
void mtgpu_sync_context_signal_fences(void *data);
int mtgpu_sync_release_fence_dependecy(struct mtgpu_fence *mtgpu_fence);
int mtgpu_sync_add_fence_dependecy(struct mtgpu_fence *mtgpu_fence, PSYNC_CHECKPOINT *checkpoints,
				   u32 checkpoint_count);
struct SYNC_CHECKPOINT_TAG *
mtgpu_sync_create_checkpoint(struct SYNC_CHECKPOINT_CONTEXT_TAG *sync_checkpoint_ctx,
			     const char *name,
			     int timeline,
			     int fd);
int mtgpu_sync_create_bo_for_checkpoint(struct drm_device *drm,
					struct drm_file *file_priv,
					PSYNC_CHECKPOINT sync_checkpoint,
					u32 *handle_out,
					u64 *value_offset);

#endif /* __MTGPU_SYNC_H__ */
