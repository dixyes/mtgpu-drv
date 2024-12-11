/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_SEMAPHORE_H_
#define _MTGPU_SEMAPHORE_H_

#include "lock_types.h"

struct drm_device;
struct drm_file;
struct _PVRSRV_DEVICE_NODE_;
struct _RGX_SERVER_COMMON_CONTEXT_;
struct _SYNC_ADDR_LIST_;
struct _CONNECTION_DATA_;
struct SYNC_CHECKPOINT_TAG;
struct mtgpu_syncobj;

struct common_context_data {
	RGXFWIF_DM dm;
	RGX_HWPERF_KICK_TYPE kick_type;
	struct _PVRSRV_DEVICE_NODE_ *dev_node;
	struct _RGX_SERVER_COMMON_CONTEXT_ *server_common_context;
	struct _SYNC_ADDR_LIST_ *update_sync_list;
	struct _SYNC_ADDR_LIST_ *check_sync_list;
	POS_LOCK lock;
};

int mtgpu_semaphore_get_fwaddr(struct _CONNECTION_DATA_ *conn, void *handle, u32 *fw_addr);
int mtgpu_semaphore_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_semaphore_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_semaphore_submit_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_semaphore_cpu_signal_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_semaphore_export_fd_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_semaphore_import_fd_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
void mtgpu_semaphore_signal_fences(void *data);
int mtgpu_semaphore_signal_callback_register(void);
void mtgpu_semaphore_signal_callback_unregister(void);

PVRSRV_ERROR mtgpu_semaphore_resolve(struct _CONNECTION_DATA_ *conn,
				     struct drm_mtgpu_semaphore *check_semas,
				     u32 check_sema_count,
				     u32 *checkpoint_count,
				     PSYNC_CHECKPOINT **checkpoints_out,
				     u32 **values_out,
				     u64 *sem_uid);
struct SYNC_CHECKPOINT_TAG *mtgpu_get_checkpoint_from_handle(CONNECTION_DATA *conn, void *handle);
int mtgpu_semaphore_cpu_signal_submit(struct drm_device *drm_dev,
				      struct mtgpu_syncobj *syncobj,
				      u64 page_table_root_addr,
				      u64 value);
int mtgpu_semaphore_wait_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
bool mtgpu_semaphore_is_signaled(PSYNC_CHECKPOINT checkpoint, u32 value);

#endif /* _MTGPU_SEMAPHORE_H_ */
