/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_IOCTL_H__
#define __MTGPU_IOCTL_H__

struct drm_device;
struct drm_file;
struct _PVRSRV_RGXDEV_INFO_;

typedef int (*mtgpu_ioctl_func_t)(struct drm_device *drm_dev, void *data, struct drm_file *file);

struct mtgpu_ioctl_desc {
	const char *name;
	mtgpu_ioctl_func_t func;
        uint64_t checksum;
};

extern const struct mtgpu_ioctl_desc core_cmd_ioctl_desc[MTGPU_CORE_CMD_MAX];
extern const struct mtgpu_ioctl_desc query_cmd_ioctl_desc[MTGPU_QUERY_CMD_MAX];
extern const struct mtgpu_ioctl_desc bo_cmd_ioctl_desc[MTGPU_BO_CMD_MAX];
extern const struct mtgpu_ioctl_desc vm_cmd_ioctl_desc[MTGPU_VM_CMD_MAX];
extern const struct mtgpu_ioctl_desc sync_cmd_ioctl_desc[MTGPU_SYNC_CMD_MAX];
extern const struct mtgpu_ioctl_desc job_cmd_ioctl_desc[MTGPU_JOB_CMD_MAX];
extern const struct mtgpu_ioctl_desc perf_cmd_ioctl_desc[MTGPU_PERF_CMD_MAX];

/* FIXME: workaround for KMD1.5 compilation error */
#define MTGPU_SYNC_CMD_TIMELINE_CREATE_CHECKSUM
#define MTGPU_SYNC_CMD_TIMELINE_DESTROY_CHECKSUM
#define MTGPU_SYNC_CMD_TIMELINE_READ_CHECKSUM
#define MTGPU_SYNC_CMD_FENCE_WAIT_CHECKSUM
#define MTGPU_SYNC_CMD_FENCE_TO_FD_CHECKSUM

#define MTGPU_JOB_CMD_CONTEXT_CREATE_CHECKSUM
#define MTGPU_JOB_CMD_CONTEXT_DESTROY_CHECKSUM
#define MTGPU_JOB_CMD_SUBMIT_CHECKSUM
#define MTGPU_JOB_CMD_DMA_TRANSFER_CHECKSUM
#define MTGPU_JOB_CMD_OBJECT_CREATE_CHECKSUM
#define MTGPU_JOB_CMD_OBJECT_DESTROY_CHECKSUM
#define MTGPU_JOB_CMD_NOTIFY_QUEUE_UPDATE_CHECKSUM
#define MTGPU_JOB_CMD_GET_LLC_PERSISTENCE_CHECKSUM
#define MTGPU_JOB_CMD_SET_LLC_PERSISTENCE_CHECKSUM
#define MTGPU_JOB_CMD_RESET_LLC_PERSISTENCE_CHECKSUM

#define MTGPU_IOCTL_DESC(cmd, func) \
	[cmd] = {#cmd, func, cmd##_##CHECKSUM}

int mtgpu_ioctl(struct drm_device *drm_dev, void *data, struct drm_file *file);
void mtgpu_ioctl_disable_access(void);
void mtgpu_ioctl_enable_access(void);
void mtgpu_ioctl_block_access(struct _PVRSRV_DEVICE_NODE_ *dev_node);
void mtgpu_ioctl_unblock_access(struct _PVRSRV_DEVICE_NODE_ *dev_node);
int mtgpu_ioctl_access_check(struct _PVRSRV_DEVICE_NODE_ *dev_node);

#endif /* __MTGPU_IOCTL_H__ */
