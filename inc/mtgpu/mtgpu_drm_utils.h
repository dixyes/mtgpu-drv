/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_UTILS_H__
#define __MTGPU_DRM_UTILS_H__

#define EDID_LOCK_PATH	"/lib/firmware/mthreads"

#define FIXED_INVALID_STATUS	(0)
#define FIXED_INITED_STATUS	(1)
#define FIXED_EDID_STATUS	(2)

struct drm_connector;
struct edid;

void mtgpu_drm_get_edid_vendor(const struct edid *edid, char *edid_vendor);
u32 mtgpu_drm_get_edid_productid(const struct edid *edid);
u32 mtgpu_drm_edid_size(struct edid *edid);

bool mtgpu_drm_get_fixedflag_from_fs(void);
bool mtgpu_drm_is_fwpath_exist(const char *filename);
void *mtgpu_drm_get_fixed_edid(struct drm_connector *connector);
void mtgpu_drm_set_fixed_edid(struct drm_connector *connector, void *edid, size_t count);
bool mtgpu_drm_get_fixed_edid_flag(struct drm_connector *connector);
void mtgpu_drm_set_fixed_edid_flag(struct drm_connector *connector, bool enable);

#endif
