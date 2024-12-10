/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_DMA_NEXT_H
#define _MTGPU_DMA_NEXT_H

struct drm_device;
struct drm_file;
struct _CONNECTION_DATA_;
struct drm_mtgpu_semaphore_submit;

struct mtgpu_dma_work_data {
	struct drm_file *file_priv;
	struct device *dev;
	struct dma_xfer_block **block_data;
	u32 *block_index;
	struct mtgpu_fence *mtgpu_fence;
	struct drm_mtgpu_dma_cmd * dma_cmds;
	int dma_cmd_count;
	u8 *args_buffer;
	struct drm_mtgpu_semaphore *update_sema;
};

int mtgpu_dma_transfer_ioctl(struct drm_device *drm, void *data,
			     struct drm_file *file_priv);
int mtgpu_dma_job_data_process(struct drm_file *file_priv,
			       u32 dma_cmd_count,
			       struct drm_mtgpu_dma_cmd * dma_cmds,
			       struct dma_xfer_block **xfer_block_data,
			       u32 *xfer_block_index);
int mtgpu_dma_create_fence(struct _CONNECTION_DATA_ *conn,
			   struct mtgpu_fence **mtgpu_fence,
			   struct drm_mtgpu_fence *update_fence,
			   char *update_fence_name);
int mtgpu_dma_semaphore_submit(struct drm_mtgpu_semaphore_submit *args,
			       struct _CONNECTION_DATA_ *conn,
			       struct SYNC_CHECKPOINT_TAG *sync_checkpoint);
void mtgpu_dma_job_submit_work(struct work_struct *work);

#endif /* _MTGPU_DMA_NEXT_H */
