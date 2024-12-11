/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef MTGPU_CODEC_H
#define MTGPU_CODEC_H

struct drm_file;
struct drm_device;
struct mtgpu_job_context;
struct drm_mtgpu_job_context_create;
struct _CONNECTION_DATA_;
struct _PVRSRV_DEVICE_NODE_;

struct mtgpu_codec_priv_data {
	u64 page_table_root_addr;
	u32 page_size;
};

int mtgpu_codec_context_create(struct drm_mtgpu_job_context_create *args,
			       struct _CONNECTION_DATA_ *conn,
			       struct _PVRSRV_DEVICE_NODE_ *dev_node);
int mtgpu_codec_wait_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_codec_job_submit(struct drm_device *drm,
			   void *data,
			   struct drm_file *file_priv,
			   struct mtgpu_job_context *job_ctx);

#endif /* MTGPU_CODEC_H */
