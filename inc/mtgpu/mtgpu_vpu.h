/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_VPU_H__
#define __MTGPU_VPU_H__

#ifdef VPU_ENABLE
#include "mtvpu_api.h"
#include "mtjpu_drv.h"
#else
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

static inline void mtvpu_gem_free_obj(void *arg1)
{
}
#endif

#endif /* __MTGPU_VPU_H__ */
