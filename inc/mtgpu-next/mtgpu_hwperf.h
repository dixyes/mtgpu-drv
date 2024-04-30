/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_HWPERF_H_
#define _MTGPU_HWPERF_H_

struct drm_device;
struct drm_file;

int mtgpu_hwperf_ioctl(struct drm_device *drm_dev, void *data, struct drm_file *file);

#endif /* _MTGPU_HWPERF_H_ */

