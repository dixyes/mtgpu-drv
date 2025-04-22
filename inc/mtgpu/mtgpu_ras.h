/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_RAS_H__
#define __MTGPU_RAS_H__

#include "os-interface.h"

struct mtgpu_device;
struct _PVRSRV_DEVICE_NODE_;

#define RAS_INPUT_BUFFER_SIZE 128

typedef enum {
	RAS_MODULE_MTLINK       = 0x0,
	RAS_MODULE_GPU,
	RAS_MODULE_FW,
	RAS_MODULE_MEM,
	RAS_MODULE_MAX,
} RAS_MODULE;

typedef enum {
	RAS_ERR_MTLINK_LINK_DOWN = 0x0,
	RAS_ERR_GPU_OVERRUN,
	RAS_ERR_GPU_LOCKUP,
	RAS_ERR_GPU_PAGE_FAULT,
	RAS_ERR_FW_IPC_TIMEOUT,
	RAS_ERR_MEM_SBE,
	RAS_ERR_MEM_DBE,
	RAS_ERR_MAX,
} RAS_ERROR;

typedef struct {
	u32 link;
} ras_mtlink_link_down_param;

typedef struct {
	u64 addr;
	u32 val;
} ras_mem_ecc_param;

typedef union {
	ras_mtlink_link_down_param mtlink_link_down_param;
	ras_mem_ecc_param mem_ecc_param;
} mtgpu_ras_param;

typedef int (*param_parser_func)(const char *param_str, mtgpu_ras_param *ras_param);

struct mtgpu_ras_ops {
	int (*mtlink_err_inject)(struct mtgpu_device *mtdev, RAS_ERROR ras_err, mtgpu_ras_param ras_param);
	int (*gpu_err_inject)(struct mtgpu_device *mtdev, RAS_ERROR ras_err, mtgpu_ras_param ras_param);
	int (*fw_err_inject)(struct mtgpu_device *mtdev, RAS_ERROR ras_err, mtgpu_ras_param ras_param);
	int (*mem_err_inject)(struct mtgpu_device *mtdev, RAS_ERROR ras_err, mtgpu_ras_param ras_param);
};

struct mtgpu_ras_item {
	RAS_MODULE ras_module;
	RAS_ERROR ras_err;
	mtgpu_ras_param ras_param;
};

struct mtgpu_ras_ctrl {
	struct mtgpu_device *mtdev;
	struct mtgpu_ras_ops *ras_ops;
};

int mtgpu_ras_debugfs_show(struct _PVRSRV_DEVICE_NODE_ *dev_node, char **output_str);
int mtgpu_ras_debugfs_ctrl(const char *buffer, u64 count, struct _PVRSRV_DEVICE_NODE_ *dev_node);
int mtgpu_ras_init(struct mtgpu_device *mtdev);
void mtgpu_ras_deinit(struct mtgpu_device *mtdev);

#endif /* __MTGPU_RAS_H__ */
