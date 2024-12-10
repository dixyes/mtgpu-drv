/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_DEBUGFS_H__
#define __MTGPU_DRM_DEBUGFS_H__

struct mtgpu_dp;
struct mtgpu_hdmi;
struct mtgpu_dispc;

void mtgpu_dp_debugfs_create_files(struct mtgpu_dp *dp);
void mtgpu_dp_debugfs_remove_files(struct mtgpu_dp *dp);

void mtgpu_hdmi_debugfs_create_files(struct mtgpu_hdmi *hdmi);
void mtgpu_hdmi_debugfs_remove_files(struct mtgpu_hdmi *hdmi);

void mtgpu_dispc_debugfs_create_files(struct mtgpu_dispc *dispc);
void mtgpu_dispc_debugfs_remove_files(struct mtgpu_dispc *dispc);

#endif /* __MTGPU_DRM_DEBUGFS_H__ */
