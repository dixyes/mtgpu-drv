/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     GPLv2
 */

#ifndef _MTGPU_DRM_DSC_H_
#define _MTGPU_DRM_DSC_H_

#include <drm/drm_crtc.h>
#include <drm/drm_encoder.h>

extern bool dsc_enable;

void mtgpu_compute_dsc_params(struct drm_encoder *encoder, struct drm_crtc *crtc);
void mtgpu_dsc_param_restore(struct drm_encoder *encoder, struct drm_crtc *crtc);

#endif /* _MTGPU_DRM_COMMON_H_ */

