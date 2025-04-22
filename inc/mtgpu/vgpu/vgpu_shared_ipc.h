/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __VGPU_SHARED_IPC_H__
#define __VGPU_SHARED_IPC_H__

#define VGPU_IPC_DEF(group, subtype) group##_##subtype

/* vGPU IPC message group */
enum {
    VGPU_IPC_GENERAL,
    VGPU_IPC_GUEST_STAT,
    VGPU_IPC_VCHANNEL,
    VGPU_IPC_VPU,
    VGPU_IPC_BS_TYPE_FILE,      /* for vchannel_bs_cmd_req_t, used to transfer disk file */
    VGPU_IPC_BS_TYPE_OBJECT,    /* for vchannel_bs_cmd_req_t, used to transfer object in sysmem: struct, union, string... */

    VGPU_IPC_MAX,
};

/* vGPU IPC message subtype for VGPU_IPC_GENERAL */
#define DEF_VGPU_IPC_GENERAL \
    X(GET_HOST_FW_VERSION)\
    X(GET_HOST_COMPAT_CHECK_MODE)\
    X(GET_HOST_VERSION_CHECK_RESULT)\
    X(NOTIFY_HOST_VERSION_CHECK_FAILED)\
    X(NOTIFY_HOST_GUEST_COMMIT_VERSION)\
    X(GET_UPGRADE_POLICY)\
    X(GET_UPGRADE_PACKAGE)

/* vGPU IPC message subtype for VGPU_IPC_GUEST_STAT */
#define DEF_VGPU_IPC_GUEST_STAT \
    X(GET_GPU_UTILIZATION)\
    X(GET_GPU_USED_MEM)\
    X(GET_GPU_USED_MEM_SIZE)

/* vGPU IPC message subtype for VGPU_IPC_VCHANNEL */
#define DEF_VGPU_IPC_VCHANNEL \
    X(CONNECT)\
    X(DISCONNECT)\
    X(HANDLE_BYTE_STREAM)

/* vGPU IPC message subtype for DEF_VGPU_IPC_VPU */
#define DEF_VGPU_IPC_VPU \
    X(GET_DEVMEM_DEV_ADDR)\
    X(GET_DEVMEM_CPU_ADDR)\
    X(GET_DEVMEM_SIZE)


/* vGPU IPC message subtype for VGPU_IPC_BS_TYPE_FILE */
#define DEF_VGPU_IPC_BS_TYPE_FILE \
    X(GET_WDDM_DRIVER)\
    X(GET_MUSA_DRIVER)\
    X(GET_UPGRADE_CONF)

/* vGPU IPC message subtype for VGPU_IPC_BS_TYPE_OBJECT */
#define DEF_VGPU_IPC_BS_TYPE_OBJECT \
    X(GET_VGPU_INFO)

enum {
#define X(subtype) VGPU_IPC_DEF(VGPU_IPC_GENERAL, subtype),
    DEF_VGPU_IPC_GENERAL
#undef X
};

enum {
#define X(subtype) VGPU_IPC_DEF(VGPU_IPC_GUEST_STAT, subtype),
    DEF_VGPU_IPC_GUEST_STAT
#undef X
};

enum {
#define X(subtype) VGPU_IPC_DEF(VGPU_IPC_VCHANNEL, subtype),
    DEF_VGPU_IPC_VCHANNEL
#undef X
};

enum {
#define X(subtype) VGPU_IPC_DEF(VGPU_IPC_VPU, subtype),
    DEF_VGPU_IPC_VPU
#undef X
};

enum {
#define X(subtype) VGPU_IPC_DEF(VGPU_IPC_BS_TYPE_FILE, subtype),
    DEF_VGPU_IPC_BS_TYPE_FILE
#undef X
};

enum {
#define X(subtype) VGPU_IPC_DEF(VGPU_IPC_BS_TYPE_OBJECT, subtype),
    DEF_VGPU_IPC_BS_TYPE_OBJECT
#undef X
};

#endif /* __VGPU_SHARED_IPC_H__ */
