/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_TRANSPORT_LAYER_H_
#define _MTGPU_TRANSPORT_LAYER_H_

struct drm_device;
struct drm_file;

int mtgpu_transport_layer_ioctl(struct drm_device *drm_dev,
				void *data,
				struct drm_file *file);

#endif /* _MTGPU_TRANSPORT_LAYER_H_ */

