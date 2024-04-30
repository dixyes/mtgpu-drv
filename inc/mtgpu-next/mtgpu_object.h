/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_OBJECT_H_
#define _MTGPU_OBJECT_H_

struct drm_device;
struct drm_file;

int mtgpu_object_destroy_ioctl(struct drm_device *drm_dev, void *raw_args, struct drm_file *file);
int mtgpu_object_create_ioctl(struct drm_device *drm_dev, void *raw_args, struct drm_file *file);

#endif /* _MTGPU_OBJECT_H_ */
