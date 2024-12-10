/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRV_NEXT_H__
#define __MTGPU_DRV_NEXT_H__

struct drm_device;
struct drm_file;

int mtgpu_device_init_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_align_check_ioctl(struct drm_device *drm_dev, void *data, struct drm_file *file);
int mtgpu_version_check_ioctl(struct drm_device *drm_dev, void *data, struct drm_file *file);

#endif /* __MTGPU_DRV_NEXT_H__ */
