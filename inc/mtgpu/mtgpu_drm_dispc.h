/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_DISPC_H__
#define __MTGPU_DRM_DISPC_H__

#include <drm/drm_crtc.h>

#include "mtgpu_dispc_common.h"

struct mtgpu_cursor_info {
	struct drm_gem_object *bo;
	dma_addr_t dev_addr;
	u32 width;
	u32 height;
	u32 x;
	u32 y;
};

struct mtgpu_dispc {
	struct drm_crtc crtc;
	struct mtgpu_dispc_ctx ctx;
	struct mtgpu_dispc_ops *core;
	struct mtgpu_dispc_glb_ops *glb;
	struct device *dev;
	struct mtgpu_cursor_info cursor_info;
	/* dispc debugfs */
	struct mtgpu_dispc_debugfs debugfs;
};

#endif /* __MTGPU_DRM_DISPC_H__ */
