/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_BO_H_
#define _MTGPU_BO_H_

struct drm_device;
struct drm_file;

int mtgpu_bo_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_bo_from_userptr_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_bo_get_mmap_offset_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_BO_H_ */
