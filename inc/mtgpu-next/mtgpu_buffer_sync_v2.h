/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_BUFFER_SYNC_V2_H__

#define MTGPU_BUFFER_SYNC_FENCE_SIGNAL_VALUE	(0x519)
#define MTGPU_BUFFER_SYNC_FENCE_TIMELINE_NAME	"buffer_sync"

struct mtgpu_buffer_sync_data;
struct mtgpu_buffer_sync_context_v2;
struct mtgpu_syncobj;
struct mtgpu_sched_job;
struct _PVRSRV_DEVICE_NODE_;

bool mtgpu_is_buffer_sync_fence(struct dma_fence *fence);
int mtgpu_buffer_sync_context_create_v2(struct mtgpu_buffer_sync_context_v2 **cxt_out);
void mtgpu_buffer_sync_context_destroy_v2(struct mtgpu_buffer_sync_context_v2 *ctx);
int mtgpu_buffer_sync_create_syncobj(struct _PVRSRV_DEVICE_NODE_ *dev_node,
				     struct mtgpu_buffer_sync_context_v2 *ctx,
				     bool use_dmabuf_fd,
				     void *buf_sync_handles,
				     u32 *buf_sync_flags,
				     u32 buf_sync_count,
				     struct mtgpu_syncobj **check_syncobjs_out,
				     u32 *check_syncobjs_count_out,
				     struct mtgpu_syncobj **update_syncobj_out,
				     struct mtgpu_buffer_sync_data **data_out);
void mtgpu_buffer_sync_submit_succeed(struct mtgpu_buffer_sync_data *data);
void mtgpu_buffer_sync_submit_fail(struct mtgpu_buffer_sync_data *data);
void mtgpu_buffer_sync_fence_signal(struct mtgpu_syncobj *syncobj);
void mtgpu_buffer_sync_fence_set_sched_job(struct device *dev, struct dma_fence *dma_fence, struct mtgpu_sched_job *sched_job);
struct mtgpu_sched_job *mtgpu_buffer_sync_fence_get_sched_job(struct device *dev, struct dma_fence *dma_fence);

int mtgpu_buffer_fence_ops_init(void);
void mtgpu_buffer_fence_ops_deinit(void);

#endif /* __MTGPU_BUFFER_SYNC_V2_H__ */
