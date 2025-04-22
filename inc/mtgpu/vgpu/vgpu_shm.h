/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __VGPU_SHM_H__
#define __VGPU_SHM_H__

#include "vchannel.h"

enum {
	VGPU_SHM_TYPE_SHARED_STATE,
	VGPU_SHM_TYPE_IPC_RINGBUF,
	VGPU_SHM_TYPE_BS_BUF_GUEST_TO_HOST_RD,
	VGPU_SHM_TYPE_BS_BUF_HOST_TO_GUEST_RD,

	VGPU_SHM_TYPE_MAX,
};

union vgpu_shm_update_cmd {
	uint64_t cmd;

	struct {
		uint64_t attach : 1;
		uint64_t type : 8;
		uint64_t size : 32;
		uint64_t __reserved : 23;
	};
};

struct vgpu_shm_ops {
	void (*init)(union vgpu_shm_update_cmd *cmd, uint64_t shm_type);
	bool (*attach)(void *data, void *priv, uint64_t shm_type);
	void (*detach)(void *priv, uint64_t shm_type);
};

struct vgpu_shm_region {
	void *data;
	uint32_t size;
	bool initialized;
	struct vgpu_shm_ops ops;
#ifdef __linux__
	/* for host only */
	int host_status;
	atomic_t refcount;
	struct completion *free;
#endif
};

#define VGPU_NODE_NUM_MAX 8
#define VPU_NODE_NUM_MAX  2

typedef enum _VGPU_CMD_TYPE_ {
    VGPU_CMD_TYPE_NORMAL = 0,
    VGPU_CMD_TYPE_PREEMPTION,
    VGPU_CMD_TYPE_MAX,
} VGPU_CMD_TYPE;

struct exec_info {
	uint64_t cmd_count;
	uint64_t max_exec_time;
	uint64_t averg_exec_time;
	uint64_t total_exec_time;
};

struct tdr_node_info {
	uint32_t node;
	uint64_t execTime;
};

struct vpu_cmd_info {
	struct tdr_node_info tdr_node[VPU_NODE_NUM_MAX];
	struct exec_info vpu_node_exec_info[VPU_NODE_NUM_MAX];
};

struct vgpu_cmd_exec_info {
	/* cause windows tdr node info */
	uint32_t idx;
	struct tdr_node_info tdr_node[VGPU_NODE_NUM_MAX];

	/* command executute time */
	struct exec_info vgpu_node_exec_info[VGPU_NODE_NUM_MAX][VGPU_CMD_TYPE_MAX];

	struct vpu_cmd_info vpu_cmd_exec_info;
};

/* shared struct definition between host and guest START */
/* VGPU_SHM_TYPE_SHARED_STATE */
struct vgpu_shared_state {
	volatile bool gpu_normal;

	/* interrupt status */
	volatile uint32_t vintr_status;
	volatile uint64_t vintr_ack_cnt;

	/* for vgpu live migration */
	/* guest read, host write */
	volatile uint64_t mig_host_stage;
	/* guest write, host read */
	volatile uint64_t mig_can_stop_guest;

	/* gpu used memory size */
	volatile uint64_t gpu_used_mem_size;
	/* vpu used memory size */
	volatile uint64_t vpu_enc_used_mem_size;
	volatile uint64_t vpu_dec_used_mem_size;

	volatile bool is_mtml_get_sysmem;

	struct vgpu_cmd_exec_info vgpu_cmd_exec_info;

	volatile uint32_t vsync_status;

	volatile uint32_t linux_host_hwr_stage;

	/* for fw hot upgrade */
	volatile uint32_t fw_hot_upgrade_stage;
};

/* VGPU_SHM_TYPE_IPC_RINGBUF */
struct vgpu_ipc_ringbuf {
	struct vqueue vqs[VQUEUE_DIRECTION_MAX_COUNT];
};

#define VGPU_BYTE_STREAM_BUF_SIZE (4096)
/* byte stream buffer shm definition */
struct vgpu_bs_buf {
	uint8_t data[VGPU_BYTE_STREAM_BUF_SIZE];
};

enum {
	VGPU_COMPAT_CHECK_MODE_DISABLE,
	VGPU_COMPAT_CHECK_MODE_VERSION_LIST,
	VGPU_COMPAT_CHECK_MODE_STRICTLY_MATCH,

	VGPU_COMPAT_CHECKE_MAX,
};

/**
 * For example version: "1.2.3[-001] <--> major.minor.patch[-update]"
 * 
 * @note field update is optional and default is 0.
 */
struct vgpu_release_version {
	uint64_t major : 16;
	uint64_t minor : 16;
	uint64_t patch : 16;
	uint64_t update : 8;

	uint64_t pass : 1;
	uint64_t __reserved : 7;
};
typedef struct vgpu_release_version vgpu_release_version_t;

union vgpu_compat_check_cmd {
	uint64_t cmd;
	vgpu_release_version_t ver;
};
typedef union vgpu_compat_check_cmd vgpu_compat_check_cmd_t;

enum {
	VGPU_DAEMON_UPGRADE_MODE_DISABLE,
	VGPU_DAEMON_UPGRADE_MODE_USER_INTERACTIVE,

	VGPU_COMPAT_UPGRADE_MAX,
};

union vgpu_upgrade_common {
	uint64_t cmd; 
	uint64_t daemon_upgrade_mode;
	uint64_t guest_pkg_version;
};
typedef union vgpu_upgrade_common vgpu_upgrade_common_t;

#define VGPU_RELEAE_VER_EQ(v1, v2) \
	(((v1).major == (v2).major) && \
	((v1).minor == (v2).minor) && \
	((v1).patch == (v2).patch) && \
	((v1).update == (v2).update))

#define VGPU_RELEAE_VER_GT(v1, v2) \
	(((v1).major > (v2).major) || \
	((v1).major == (v2).major && (v1).minor > (v2).minor) || \
	((v1).major == (v2).major && (v1).minor == (v2).minor && (v1).patch > (v2).patch) || \
	((v1).major == (v2).major && (v1).minor == (v2).minor && (v1).patch == (v2).patch && (v1).update > (v2).update))

#ifndef VGPU_RELEASE_VERSION_MAJOR
#define VGPU_RELEASE_VERSION_MAJOR 1
#define VGPU_RELEASE_VERSION_MINOR 2
#define VGPU_RELEASE_VERSION_PATCH 3
#define VGPU_RELEASE_VERSION_UPDATE 4
#endif

#define VGPU_RELEASE_VER_INIT() \
	{ VGPU_RELEASE_VERSION_MAJOR, VGPU_RELEASE_VERSION_MINOR, \
	  VGPU_RELEASE_VERSION_PATCH, VGPU_RELEASE_VERSION_UPDATE }

#ifdef VGPU_RELEASE_VERSION_DFT
#define VGPU_RELEASE_VERSION_LIST_DUMMY() \
	{ 2, 4, 1, 0 },	\
	{ 2, 4, 1, 1 },	\
	{ 2, 4, 1, 2 },	\
	{ 2, 4, 2, 0 },	\
	{ 2, 4, 2, 1 },	\
			\
	{ 2, 5, 0, 0 },	\
	{ 2, 5, 0, 1 },	\
	{ 2, 5, 1, 0 },	\
	{ 2, 5, 2, 0 },	\
			\
	{ 2, 6, 0, 0 },	\
	{ 2, 6, 1, 0 }, \
	{ 2, 6, 5, 0 }, \
	{ 2, 6, 5, 1 }, \
	{ 2, 6, 6, 0 }, \
	{ 2, 8, 0, 0 }, \
	{ 3, 8, 0, 0 },
#endif

/* shared struct definition between host and guest END */

#ifndef __linux__
typedef union  vgpu_shm_update_cmd      VGPU_SHM_UPDATE_CMD;
typedef struct vgpu_shm_ops             VGPU_SHM_CALLBACKS;
typedef struct vgpu_shm_region          VGPU_SHM_REGION;

typedef struct vgpu_shared_state        VGPU_SHARED_STATE;
typedef struct vgpu_ipc_ringbuf         VGPU_IPC_RINGBUF;

typedef struct vgpu_cmd_exec_info       VGPU_CMD_EXEC_INFO;
typedef struct vpu_cmd_info             VPU_CMD_INFO;
typedef struct exec_info                EXEC_INFO;
typedef struct tdr_node_info            TDR_NODE_INFO;
typedef struct vgpu_bs_buf              VGPU_BS_BUF;

typedef struct vgpu_release_version     VGPU_RELEASE_VERSION;
typedef union vgpu_compat_check_cmd     VGPU_COMPAT_CHECK_CMD;
typedef union vgpu_upgrade_common       VGPU_UPGRADE_COMMON;
#endif /* __linux__ */

#endif /* __VGPU_SHM_H__ */