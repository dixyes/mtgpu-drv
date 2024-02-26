/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_DMA_NEXT_H
#define _MTGPU_DMA_NEXT_H

struct drm_device;
struct drm_file;

int mtgpu_dma_transfer_ioctl(struct drm_device *drm, void *data,
			     struct drm_file *file_priv);

#endif /* _MTGPU_DMA_NEXT_H */
