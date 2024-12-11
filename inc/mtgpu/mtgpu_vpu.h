/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_VPU_H__
#define __MTGPU_VPU_H__

#ifdef VPU_ENABLE
#include "mtvpu_api.h"
#else
struct mtgpu_codec_priv_data;

static inline int mtvpu_drm_open(void *arg1, void *arg2)
{
	return 0;
}

static inline void mtvpu_drm_release(void *arg1, void *arg2)
{
}

static inline int mtvpu_gem_mmap_obj(void *arg1, void *arg2)
{
	return 0;
}

static inline int mtvpu_gem_dmabuf_map(void *arg1, void *arg2)
{
	return 0;
}

static inline void mtvpu_gem_free_obj(void *arg1)
{
}

static inline int mtvpu_job_submit(struct drm_device *drm,
				   struct drm_file *file_priv,
				   void __user *data,
				   uint32_t size,
				   struct mtgpu_codec_priv_data *data_priv)
{
	return 0;
}

static inline int mtvpu_bo_wait(struct drm_device *drm,
				struct drm_file *file_priv,
				u64 bo_handle,
				u32 offset,
				u32 flags,
				s64 timeout_ns)
{
	return 0;
}
#endif

#endif /* __MTGPU_VPU_H__ */
