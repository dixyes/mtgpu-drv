/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_JOB_H_
#define _MTGPU_JOB_H_

struct drm_device;
struct drm_file;

int mtgpu_job_submit_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_submit2_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_JOB_H_ */
