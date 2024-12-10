/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_CONTEXT_H_
#define _MTGPU_CONTEXT_H_

struct drm_device;
struct drm_file;
struct _CONNECTION_DATA_;
struct _PVRSRV_DEVICE_NODE_;
struct _PVRSRV_RGXDEV_INFO_;
struct drm_mtgpu_job_context_create;
struct mtgpu_job_context;
struct _RGX_SERVER_COMPUTE_CONTEXT_;

struct mtgpu_render_context {
	struct device *dev;
	struct _RGX_SERVER_RENDER_CONTEXT_ *base;
	struct mtgpu_render_csw_data *csw;
};

struct mtgpu_compute_context {
	struct device *dev;
	struct _RGX_SERVER_COMPUTE_CONTEXT_ *base;
	struct mtgpu_compute_csw_data *csw;
};

struct mtgpu_dma_context {
	u64 robustness_addr;
  	struct workqueue_struct *dma_workqueue;
};

struct _PVRSRV_RGXDEV_INFO_ *mtgpu_job_context_get_devinfo(struct mtgpu_job_context *job_ctx);
void mtgpu_job_context_set_priv(struct mtgpu_job_context *job_ctx, void *data);
void *mtgpu_job_context_get_priv(struct mtgpu_job_context *job_ctx);
int mtgpu_job_context_create(struct drm_mtgpu_job_context_create *args,
			     struct _CONNECTION_DATA_ *conn,
			     struct _PVRSRV_DEVICE_NODE_ *dev_node,
			     struct mtgpu_job_context **job_ctx_out);
void mtgpu_job_context_destroy(struct mtgpu_job_context *job_ctx);

int mtgpu_context_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_context_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_context_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_CONTEXT_H_ */
