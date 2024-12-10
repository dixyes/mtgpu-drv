/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_ATOMIC_H__
#define __MTGPU_DRM_ATOMIC_H__

void mtgpu_atomic_helper_commit_modeset_enables(struct drm_device *dev,
						struct drm_atomic_state *old_state);
void mtgpu_atomic_helper_commit_modeset_disables(struct drm_device *dev,
						 struct drm_atomic_state *old_state);
int mtgpu_atomic_helper_async_check(struct drm_device *dev,
				    struct drm_atomic_state *state);

#endif /* __MTGPU_DRM_ATOMIC_H__ */
