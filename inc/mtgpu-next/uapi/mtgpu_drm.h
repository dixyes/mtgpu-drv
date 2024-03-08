/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_H__
#define __MTGPU_DRM_H__

#include "linux-types.h"

#define MTGPU_IOCTL_VERSION		(13)

#define DRM_MTGPU_DEVICE_INIT		0x10
#define DRM_MTGPU_QUERY_INFO		0x11
#define DRM_MTGPU_BO_CREATE		0x12
#define DRM_MTGPU_BO_GET_MMAP_OFFSET	0x13
#define DRM_MTGPU_BO_FROM_USERPTR	0x14
#define DRM_MTGPU_VM_CONTEXT_CREATE	0x15
#define DRM_MTGPU_VM_CONTEXT_DESTROY	0x16
#define DRM_MTGPU_VM_MAP		0x17
#define DRM_MTGPU_VM_UNMAP		0x18
#define DRM_MTGPU_TIMELINE_CREATE	0x19
#define DRM_MTGPU_TIMELINE_DESTROY	0x1A
#define DRM_MTGPU_TIMELINE_READ		0x1B
#define DRM_MTGPU_FENCE_WAIT		0x1C
#define DRM_MTGPU_FENCE_TO_FD		0x1D
#define DRM_MTGPU_SEMAPHORE_CREATE	0x1E
#define DRM_MTGPU_SEMAPHORE_DESTROY	0x1F
#define DRM_MTGPU_SEMAPHORE_SUBMIT	0x20
#define DRM_MTGPU_CONTEXT_CREATE	0x21
#define DRM_MTGPU_CONTEXT_DESTROY	0x22
#define DRM_MTGPU_JOB_SUBMIT		0x23
#define DRM_MTGPU_DMA_TRANSFER		0x24
#define DRM_MTGPU_OBJECT_CREATE		0x25
#define DRM_MTGPU_OBJECT_DESTROY	0x26
#define DRM_MTGPU_TRANSPORT_LAYER	0x27
#define DRM_MTGPU_HWPERF		0x28
#define DRM_MTGPU_CACHE_OP		0x29

#define DRM_MTGPU_JOB_SUBMIT2		0x30

#define DRM_IOCTL_MTGPU_DEVICE_INIT \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_DEVICE_INIT, \
		 struct drm_mtgpu_device_init)
#define DRM_IOCTL_MTGPU_QUERY_INFO \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_QUERY_INFO, \
		 struct drm_mtgpu_query_info)
#define DRM_IOCTL_MTGPU_BO_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_BO_CREATE, \
		 struct drm_mtgpu_bo_create)
#define DRM_IOCTL_MTGPU_BO_GET_MMAP_OFFSET \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_BO_GET_MMAP_OFFSET, \
		 struct drm_mtgpu_bo_get_mmap_offset)
#define DRM_IOCTL_MTGPU_BO_FROM_USERPTR \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_BO_FROM_USERPTR, \
		 struct drm_mtgpu_bo_from_userptr)
#define DRM_IOCTL_MTGPU_VM_CONTEXT_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_VM_CONTEXT_CREATE, \
		 struct drm_mtgpu_vm_context_create)
#define DRM_IOCTL_MTGPU_VM_CONTEXT_DESTROY \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_VM_CONTEXT_DESTROY, \
		 struct drm_mtgpu_vm_context_destroy)
#define DRM_IOCTL_MTGPU_VM_MAP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_VM_MAP, \
		 struct drm_mtgpu_vm_map)
#define DRM_IOCTL_MTGPU_VM_UNMAP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_VM_UNMAP, \
		 struct drm_mtgpu_vm_map)
#define DRM_IOCTL_MTGPU_TIMELINE_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_TIMELINE_CREATE, \
		 struct drm_mtgpu_timeline_create)
#define DRM_IOCTL_MTGPU_TIMELINE_DESTROY \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_TIMELINE_DESTROY, \
		 struct drm_mtgpu_timeline_destroy)
#define DRM_IOCTL_MTGPU_TIMELINE_READ \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_TIMELINE_READ, \
		 struct drm_mtgpu_timeline_read)
#define DRM_IOCTL_MTGPU_FENCE_WAIT \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_FENCE_WAIT, \
		 struct drm_mtgpu_fence_wait)
#define DRM_IOCTL_MTGPU_FENCE_TO_FD \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_FENCE_TO_FD, \
		 struct drm_mtgpu_fence_to_fd)
#define DRM_IOCTL_MTGPU_SEMAPHORE_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_SEMAPHORE_CREATE, \
		 struct drm_mtgpu_semaphore_create)
#define DRM_IOCTL_MTGPU_SEMAPHORE_DESTROY \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_SEMAPHORE_DESTROY, \
		 struct drm_mtgpu_semaphore_destroy)
#define DRM_IOCTL_MTGPU_SEMAPHORE_SUBMIT \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_SEMAPHORE_SUBMIT, \
		 struct drm_mtgpu_semaphore_submit)
#define DRM_IOCTL_MTGPU_CONTEXT_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_CONTEXT_CREATE, \
		 struct drm_mtgpu_context_create)
#define DRM_IOCTL_MTGPU_CONTEXT_DESTROY \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_CONTEXT_DESTROY, \
		 struct drm_mtgpu_context_destroy)
#define DRM_IOCTL_MTGPU_JOB_SUBMIT \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_JOB_SUBMIT, \
		 struct drm_mtgpu_job_submit)
#define DRM_IOCTL_MTGPU_DMA_TRANSFER \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_DMA_TRANSFER, \
		 struct drm_mtgpu_dma_transfer)
#define DRM_IOCTL_MTGPU_OBJECT_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_OBJECT_CREATE, \
		 struct drm_mtgpu_object_create)
#define DRM_IOCTL_MTGPU_OBJECT_DESTROY \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_OBJECT_DESTROY, \
		 struct drm_mtgpu_object_destroy)
#define DRM_IOCTL_MTGPU_TRANSPORT_LAYER \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_TRANSPORT_LAYER, \
		 struct drm_mtgpu_transport_layer)
#define DRM_IOCTL_MTGPU_HWPERF \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_HWPERF, \
		 struct drm_mtgpu_hwperf)
#define DRM_IOCTL_MTGPU_CACHE_OP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_CACHE_OP, \
		 struct drm_mtgpu_cache_op)

#define DRM_IOCTL_MTGPU_JOB_SUBMIT2 \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_JOB_SUBMIT2, \
		 struct drm_mtgpu_job_submit_v2)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                         Domain                         *
 *  *                                                        *
 *  **********************************************************
 */
#define MTGPU_BO_DOMAIN_CPU		BIT(0)
#define MTGPU_BO_DOMAIN_GTT		BIT(1)
#define MTGPU_BO_DOMAIN_VRAM		BIT(2)
#define MTGPU_BO_DOMAIN_MASK		(MTGPU_BO_DOMAIN_CPU | \
					 MTGPU_BO_DOMAIN_GTT | \
					 MTGPU_BO_DOMAIN_VRAM)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                        CPU domain                      *
 *  *                                                        *
 *  **********************************************************
 */
/* Which corresponds to PVRSRV_MEMALLOCFLAG_CPU_UNCACHED_WC in pvr */
#define MTGPU_BO_FLAGS_CPU_UNCACHED_WC			BIT(0)
/* Which corresponds to PVRSRV_MEMALLOCFLAG_CPU_CACHED in pvr */
#define MTGPU_BO_FLAGS_CPU_CACHED			BIT(1)
/* Which corresponds to PVRSRV_MEMALLOCFLAG_CPU_READABLE |
 * PVRSRV_MEMALLOCFLAG_CPU_WRITEABLE in pvr
 */
#define MTGPU_BO_FLAGS_CPU_MAPPABLE			BIT(2)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                        GPU domain                      *
 *  *                                                        *
 *  **********************************************************
 */
/* Which corresponds to PVRSRV_MEMALLOCFLAG_GPU_UNCACHED in pvr*/
#define MTGPU_BO_FLAGS_VRAM_UNCACHED			BIT(8)
/* Which corresponds to PVRSRV_MEMALLOCFLAG_GPU_UNCACHED_WC in pvr*/
#define MTGPU_BO_FLAGS_VRAM_UNCACHED_WC			BIT(9)
/* Which corresponds to PVRSRV_MEMALLOCFLAG_GPU_CACHED in pvr*/
#define MTGPU_BO_FLAGS_VRAM_CACHED			BIT(10)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                          Others                        *
 *  *                                                        *
 *  **********************************************************
 */
/* Which corresponds to PVRSRV_MEMALLOCFLAG_KERNEL_CPU_MAPPABLE |
 * PVRSRV_MEMALLOCFLAG_ZERO_ON_ALLOC in pvr
 */
#define MTGPU_BO_FLAGS_CLEARED				BIT(16)

struct drm_mtgpu_device_init {
	__u32 ioctl_version;
	__u32 pad;
};

/* Query information about device: rev id, family, etc. */
enum {
	MTGPU_INFO_TYPE_HEAP_COUNT = 0,
	MTGPU_INFO_TYPE_HEAP_DETAILS,
	MTGPU_INFO_TYPE_MEMORY,
	MTGPU_INFO_TYPE_BO,
	MTGPU_INFO_TYPE_DEV,
	MTGPU_INFO_TYPE_PCI,
	MTGPU_INFO_TYPE_HEAP_INVALID,
};

#define MTGPU_HEAPNAME_MAXLENGTH (128)

struct mtgpu_heap_info {
	/* ID of this heap */
	__u32 id;

	/* Name of this heap - for debug purposes, and perhaps for lookup by name */
	char name[MTGPU_HEAPNAME_MAXLENGTH];

	/* Data page size.  This is the page size that is going to get
	 * programmed into the MMU, so it needs to be a valid one for the
	 * device.  Importantly, the start address and length _must_ be
	 * multiples of this page size.  Note that the page size is
	 * specified as the log 2 relative to 1 byte (e.g. 12 indicates
	 * 4kB)
	 */
	__u64 base;

	/* Length of the heap. The heap length _must_ be a whole number
	 * of data pages. Again, the recommendation is that it ends on
	 * a 1GB boundary.
	 */
	__u64 length;

	/* Data page size.  This is the page size that is going to get
	 * programmed into the MMU, so it needs to be a valid one for the
	 * device.  Importantly, the start address and length _must_ be
	 * multiples of this page size.  Note that the page size is
	 * specified as the log 2 relative to 1 byte (e.g. 12 indicates
	 * 4kB)
	 */
	__u32 log2_page_size;
};

struct mtgpu_heap_detail_in {
	__u32 index;
};

struct mtgpu_heap_detail_out {
	struct mtgpu_heap_info info;
};

struct mtgpu_mem_info {
	struct {
		__u64 total_size;
		__u64 free_size;
	} system;
	struct {
		__u64 total_size;
		__u64 free_size;
	} vram;
};

struct drm_mtgpu_device_info {
	/** PCI Device ID */
	__u32 device_id;
};

struct mtgpu_bo_info_in {
	__u32 bo_handle;
	__u32 pad;
};

struct mtgpu_bo_info {
	__u64 size;
};

struct mtgpu_bo_info_out {
	struct mtgpu_bo_info info;
};

struct mtgpu_dev_info {
	__u32 dev_clock_speed;
	__u32 mem_clock_speed;
	__u32 mem_max_clock_speed;
	__u32 num_cores;
	__u8 uuid[16];
};

struct mtgpu_pci_info {
	__u32 pci_domain_number;
	__u32 pci_bus_number;
	__u32 pci_device_number;
	__u32 pci_function_number;
};

struct drm_mtgpu_query_info {
	struct {
		__u32 type;
		__u32 pad;
		__u64 data;
	} in;

	struct {
		__u64 data;
	} out;
};

struct drm_mtgpu_bo_create {
	struct {
		__u32 size;
		__u32 align;
		__u32 domains;
		__u32 flags;
	} in;

	struct {
		__u32 bo_handle;
		__u32 pad;
	} out;
};

struct drm_mtgpu_bo_from_userptr {
	struct {
		__u64 userptr;
		__u64 size;
		__u32 flags;
		__u32 pad;
	} in;

	struct {
		__u32 bo_handle;
		__u32 pad;
	} out;
};

struct drm_mtgpu_bo_get_mmap_offset {
	struct {
		__u32 bo_handle;
		__u32 pad;
	} in;

	struct {
		__u64 offset;
	} out;
};

struct drm_mtgpu_vm_context_create {
	/** @vm_ctx_handle: [OUT] Handle for new VM context. */
	__u64 vm_ctx_handle;
};

struct drm_mtgpu_vm_context_destroy {
	/**
	 * @vm_ctx_handle: [IN] Handle for VM context to be destroyed.
	 */
	__u64 vm_ctx_handle;
};

struct drm_mtgpu_vm_map {
	/**
	 * @vm_ctx_handle: [IN] Handle for VM context that this mapping
	 * exists in. This must be a valid handle returned by
	 * %DRM_IOCTL_MTGPU_VM_CONTEXT_CREATE.
	 */
	__u64 vm_ctx_handle;
	/**
	 * @va: [IN] Requested device-virtual address for the mapping.
	 * This must be non-zero and aligned to the device page size for the
	 * heap containing the requested address.
	 */
	__u64 va;
	/**
	 * @flags: [IN] Flags which affect this mapping. Currently always 0.
	 */
	__u64 mapping_flags;
	/**
	 * @handle: [IN] Handle of the target buffer object. This must be a
	 * valid handle returned by %DRM_IOCTL_MTGPU_BO_CREATE.
	 */
	__u32 bo_handle;
	/**
	 * @pad: [IN] for padding
	 */
	__u32 pad;
	/**
	 * @size: [IN] Size of the requested mapping. Must be aligned to
	 * the device page size for the heap containing the requested address,
	 * as well as the host page size.
	 */
	__u64 size;
};

struct drm_mtgpu_vm_unmap {
	/**
	 * @vm_ctx_handle: [IN] Handle for VM context that this mapping
	 * exists in. This must be a valid handle returned by
	 * %DRM_IOCTL_MTGPU_VM_CONTEXT_CREATE.
	 */
	__u64 vm_ctx_handle;
	/**
	 * @va: [IN] Requested device-virtual address for the mapping.
	 * This must be non-zero and aligned to the device page size for the
	 * heap containing the requested address.
	 */
	__u64 va;
};

struct drm_mtgpu_timeline_create {
	struct {
		/**
		 * @timeline_handle: [OUT] handle of fence timeline
		 */
		__u64 timeline_handle;
		/**
		 * @timeline_bo_handle: [OUT] bo handle of fence timeline
		 */
		__u64 timeline_bo_handle;
		/**
		 * @timeline_value_offset: [OUT] value offset of fence timeline
		 */
		__u64 timeline_value_offset;
	} out;
};

struct drm_mtgpu_timeline_destroy {
	struct {
		/**
		 * @timeline_handle: [IN] handle of fence timeline
		 */
		__u64 timeline_handle;
	} in;
};

struct drm_mtgpu_timeline_read {
	struct {
		/**
		 * @timeline_handle: [IN] handle of fence timeline
		 */
		__u64 timeline_handle;
	} in;
	struct {
		/**
		 * @timeline_value: [OUT] value of fence timeline
		 */
		__u64 timeline_value;
	} out;
};

struct drm_mtgpu_fence {
	/**
	 * @timeline_handle: [IN] handle of fence timeline
	 */
	__u64 timeline_handle;

	/**
	 * @seqno: [IN] fence sequence number
	 */
	__u64 seqno;
};

struct drm_mtgpu_fence_wait {
	struct {
		__u64 fences;
		__u32 seqno_count;
		__u32 wait_all;
		__u64 timeout_ns;
	} in;

	struct {
		/**
		 * @first_signaled: [OUT] Index of first signaled fence in fences[]
		 */
		__u32 first_signaled;
		__u32 pad;
	} out;
};

/* context related */
struct drm_mtgpu_context_create {
	struct {
		/**
		 * @type: [IN] Type of the context to be created
		 *
		 * This must be one of the values defined by &enum drm_mtgpu_job_type.
		 */
		__u32 type;

		/**
		 * @type: [IN] Flags of the context to be created
		 */
		__u32 flags;

		/**
		 * @ccbsize: [IN] ccbsize of the context to be created
		 */
		__u32 ccbsize;

		/**
		 * @pad: [IN] Priority of new context.
		 */
		__u32 priority;

		/**
		 * @vm_ctx_handle: [IN] handle of vm context.
		 */
		__u64 vm_ctx_handle;

		/**
		 * @data: [IN] Private data of the context to be created
		 */
		__u64 data;
	} in;

	struct {
		/**
		 * @ctx_handle: [OUT] handle of job context.
		 */
		__u64 ctx_handle;
	} out;
};

struct drm_mtgpu_context_destroy {
	/**
	 * @type: [IN] Type of the context to be created
	 *
	 * This must be one of the values defined by &enum drm_mtgpu_job_type.
	 */
	__u32 type;

	/**
	 * @pad: [IN] for padding
	 */
	__u32 pad;

	/** @ctx_handle: [IN] handle of job context. */
	__u64 ctx_handle;
};

struct drm_mtgpu_tq_context_data {
	/** @robustness_addr: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
};

struct drm_mtgpu_render_context_data {
	/** @static_context_state: [IN] Context static register controls for context switch. */
	__u64 static_context_state;
	/** @static_context_state: [IN] Length of context static register controls. */
	__u32 static_context_state_len;
	/** @framework_cmd: [IN] Framework command. */
	__u64 framework_cmd;
	/** @framework_cmd: [IN] Framework command size. */
	__u32 framework_cmd_size;
	/** @robustness_addr: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
	/** @max_3d_deadline_ms: [IN] Max 3D deadline limit in MS. */
	__u32 max_3d_deadline_ms;
	/** @max_ta_deadline_ms: [IN] Max TA deadline limit in MS. */
	__u32 max_ta_deadline_ms;
};

struct drm_mtgpu_compute_context_data {
	/** @static_context_state: [IN] Context static register controls for context switch. */
	__u64 static_context_state;
	/** @static_context_state: [IN] Length of context static register controls. */
	__u32 static_context_state_len;
	/** @robustness_address: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
	/** @robustness_address: [IN] Max deadline limit in MS. */
	__u32 max_deadline_ms;
};

struct drm_mtgpu_ce_context_data {
	/** @framework_cmd: [IN] Framework command. */
	__u64 framework_cmd;
	/** @framework_cmd: [IN] Framework command size. */
	__u32 framework_cmd_size;
	/** @robustness_addr: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
};

enum drm_mtgpu_job_type {
	MTGPU_JOB_NOP = 0,
	MTGPU_JOB_TQ,
	MTGPU_JOB_CE,
	MTGPU_JOB_RENDER,
	MTGPU_JOB_COMPUTE,
	MTGPU_JOB_INVALID,
};

struct drm_mtgpu_compute_job_data_v2 {
	__u32 num_of_workgroups;
	__u32 num_of_workitems;
};

struct drm_mtgpu_tq_job_data_v2 {
	__u32 characteristic1;
	__u32 characteristic2;
};

struct drm_mtgpu_ce_job_data_v2 {
	__u32 characteristic1;
	__u32 characteristic2;
};

struct drm_mtgpu_render_job_data_v2 {
	/**
	 * @frag_check_semaphores: [IN] check semaphore array for 3D
	 */
	__u64 frag_check_semaphores;
	/**
	 * @frag_check_semaphore_count: [IN] check semaphore count for 3D
	 */
	__u32 frag_check_semaphore_count;
	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad1;

	/**
	 * @frag_update_semaphores: [IN] update semaphore array for 3D
	 */
	__u64 frag_update_semaphores;
	/**
	 * @frag_update_semaphore_count: [IN] update semaphore count for 3D
	 */
	__u32 frag_update_semaphore_count;
	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad2;

	/**
	 * @frag_foreign_fence_fd: [IN] the fd of 3D foreign fence
	 */
	__s32 frag_foreign_fence_fd;
	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad3;

	/**
	 * @frag_update_fence: [IN] update fence
	 */
	__u64 frag_update_fence;
	/**
	 * @frag_update_fence_name: [IN] update fence name
	 */
	char *frag_update_fence_name;

	/**
	 * @frag_cmd_array: [IN] 3D dm command buffer array
	 */
	__u8 *frag_cmd_array;
	/**
	 * @frag_cmd_size: [IN] 3D dm command size
	 */
	__u32 frag_cmd_size;
	/**
	 * @frag_cmd_count: [IN] the count of 3D dm command buffer
	 */
	__u32 frag_cmd_count;

	__u64 hwrt_dataset_handle;
	__u64 msaa_scratch_buffer_handle;
	__u64 zs_buffer_handle;
	__u32 draw_calls_number;
	__u32 indices_number;
	__u32 mrts_number;
	__u32 render_target_size;

	__u32 abort;
	__u32 kick_frag;
	__u32 kick_pr;
	__u32 kick_geom;
};

struct drm_mtgpu_job_submit_v2 {
	struct {
		/**
		 * @type: [IN] Type of the job
		 * This must be one of the values defined by &enum drm_mtgpu_job_type.
		 */
		__u32 type;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad1;

		/**
		 * @vm_ctx_handle: [IN] handle of job context.
		 */
		__u64 ctx_handle;

		/**
		 * @check_semaphores: [IN] check semaphore array
		 */
		__u64 check_semaphores;
		/**
		 * @check_semaphore_count: [IN] check semaphore count
		 */
		__u32 check_semaphore_count;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad2;

		/**
		 * @update_semaphores: [IN] check semaphore array
		 */
		__u64 update_semaphores;
		/**
		 * @update_semaphore_count: [IN] check semaphore count
		 */
		__u32 update_semaphore_count;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad3;

		/**
		 * @check_fences: [IN] check fence array
		 */
		__u64 check_fences;
		/**
		 * @check_fence_count: [IN] check fence count
		 */
		__u32 check_fence_count;
		/**
		 * @foreign_fence_fd: [IN] the fd of foreign fence
		 */
		__s32 foreign_fence_fd;

		/**
		 * @sync_buf_fds: [IN] sync buf array
		 */
		__s32 *sync_buf_fds;
		/**
		 * @sync_buf_flags: [IN] array of sync buf flag
		 */
		__u32 *sync_buf_flags;
		/**
		 * @sync_buf_count: [IN] the count of sync buf
		 */
		__u32 sync_buf_count;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad4;

		/**
		 * @update_fence: [IN] update fence
		 */
		__u64 update_fence;
		/**
		 * @update_fence_name: [IN] update fence name
		 */
		char *update_fence_name;

		/**
		 * @dm_cmd_array: [IN] dm command buffer array
		 */
		__u8 *dm_cmd_array;
		/**
		 * @dm_cmd_size: [IN] dm command size
		 */
		__u32 dm_cmd_size;
		/**
		 * @dm_cmd_count: [IN] the count of dm command buffer
		 */
		__u32 dm_cmd_count;

		/**
		 * @deadline_us: [IN] deadline in us
		 */
		__u64 deadline_us;
		/**
		 * @ext_job_ref: [IN] external job reference
		 */
		__u32 ext_job_ref;
		/**
		 * @pdump_flags: [IN] pdump flags
		 */
		__u32 pdump_flags;

		/**
		 * @data: [IN] Private data of the specific job type
		 */
		__u64 data;
	} in;
};

struct drm_mtgpu_compute_job_data {
	__u64 deadline_us;
	__u32 *client_update_offset;
	__u32 *client_update_value;
	__u64 *client_update_ufo_sync_prim_block_handles;
	__u32 client_update_count;
	__u32 *sync_bo_flags;
	__u64 *sync_bo_handles;
	__u32 sync_bo_count;
	__s32 check_fence;
	char *update_fence_name;
	__u8 *dm_cmd;
	__u32 command_size;
	__u32 external_job_reference;
	__u32 num_of_workgroups;
	__u32 num_of_workitems;
};

struct drm_mtgpu_tq_job_data {
	__u64 deadline_us;
	__u32 *client_update_offset;
	__u32 *client_update_value;
	__u64 *client_update_ufo_sync_prim_block_handles;
	__u32 client_update_count;
	__u32 *sync_bo_flags;
	__u64 *sync_bo_handles;
	__u32 sync_bo_count;
	__u8 *dm_cmd;
	__u32 command_size;
	char *update_fence_name;
	__s32 check_fence;
	__s32 update_timeline;
	__u32 characteristic1;
	__u32 characteristic2;
	__u32 external_job_reference;
	__u32 pdump_flags;
};

struct drm_mtgpu_ce_job_data {
	__u64 deadline_us;
	__u32 *client_update_offset;
	__u32 *client_update_value;
	__u64 *client_update_ufo_sync_prim_block_handles;
	__u32 client_update_count;
	__u32 *sync_bo_flags;
	__u64 *sync_bo_handles;
	__u32 sync_bo_count;
	__u8 *dm_cmd;
	__u32 command_size;
	char *update_fence_name;
	__s32 check_fence;
	__s32 update_timeline;
	__u32 characteristic1;
	__u32 characteristic2;
	__u32 external_job_reference;
	__u32 pdump_flags;
};

struct drm_mtgpu_render_job_data {
	__u64 deadline_us;
	__u64 hwrt_dataset_handle;
	__u64 msaa_scratch_buffer_handle;
	__u64 pr_fence_ufo_sync_prim_block_handle;
	__u64 zs_buffer_handle;
	__u64 frag_update_sync_offset_array;
	__u64 frag_update_value_array;
	__u64 geom_fence_sync_offset_array;
	__u64 geom_fence_value_array;
	__u64 geom_update_sync_offset_array;
	__u64 geom_update_value_array;
	__u64 sync_bo_flag_array;
	__u8 *frag_cmd;
	__u8 *frag_pr_cmd;
	__u8 *geom_cmd;
	char *geom_update_fence_name;
	char *frag_update_fence_name;
	__u64 frag_update_sync_prim_block_handles;
	__u64 geom_fence_sync_prim_block_handles;
	__u64 geom_update_sync_prim_block_handles;
	__u64 sync_bo_handles;
	__u32 abort;
	__u32 kick_frag;
	__u32 kick_pr;
	__u32 kick_geom;
	__u32 geom_check_fence;
	__u32 frag_check_fence;
	__u32 geom_update_timeline;
	__u32 frag_update_timeline;
	__u32 frag_cmd_size;
	__u32 frag_pr_cmd_size;
	__u32 frag_update_count;
	__u64 frag_update_fence_out;
	__u32 geom_fence_count;
	__u32 geom_update_count;
	__u64 geom_update_fence_out;
	__u32 ext_job_ref;
	__u32 draw_calls_number;
	__u32 indices_number;
	__u32 mrts_number;
	__u32 pdump_flags;
	__u32 pr_fence_ufo_sync_offset;
	__u32 pr_fence_value;
	__u32 render_target_size;
	__u32 sync_bo_count;
	__u32 geom_cmd_size;
};

struct drm_mtgpu_job_submit {
	struct {
		/**
		 * @type: [IN] Type of the job
		 *
		 * This must be one of the values defined by &enum drm_mtgpu_job_type.
		 */
		__u32 type;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;

		/**
		 * @vm_ctx_handle: [IN] handle of job context.
		 */
		__u64 ctx_handle;

		/**
		 * @update_fence: [IN] update fence
		 */
		__u64 update_fence;

		/**
		 * @check_fences: [IN] check fence array
		 */
		__u64 check_fences;

		/**
		 * @check_fence_count: [IN] check fence count
		 */
		__u32 check_fence_count;

		/**
		 * @foreign_fence_fd: [IN] the fd of foreign fences
		 */
		__s32 foreign_fence_fd;

		/**
		 * @data: [IN] Private data of the specific job type
		 */
		__u64 data;
	} in;
};

#define MTGPU_DMA_TRANSFER_DEVICE_TO_HOST 0x0
#define MTGPU_DMA_TRANSFER_HOST_TO_DEVICE 0x1
#define MTGPU_DMA_TRANSFER_DIR BIT(0)

struct drm_mtgpu_dma_transfer {
	struct {
		/**
		 * @bo_handle: [IN] mtgpu_bo handle
		 */
		__u32 bo_handle;

		/**
		 * @transfer_flag: [IN] transmission instructions,
		 * BIT(0) 0:write to bo buffer,1: read from bo buffer.
		 */
		__u32 transfer_flag;

		/**
		 * @ext_handle: [IN] host cpu virtual address or other mtgpu_bo handle,
		 * according to the flag parameter indication.
		 */
		__u64 ext_handle;

		/**
		 * @offset: [IN] the starting position of the transfer.
		 */
		__u64 offset;

		/**
		 * @size: [IN] transfer size.
		 */
		__u64 size;

		/**
		 * @fence_handle: [IN] fence handle for synchronous transmission.
		 */
		__u64 fence_handle;
	} in;
};

struct drm_mtgpu_object_destroy {
	/**
	 * @type: [IN] Type of object to create.
	 *
	 * This must be one of the values defined by &enum drm_mtgpu_object_type.
	 */
	__u32 type;

	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad;

	/**
	 * @handle: [IN] Handle for freelist to be destroyed.
	 */
	__u64 handle;
};

/**
 * struct drm_mtgpu_hwrt_dataset_create_args - Arguments for
 * %DRM_MTGPU_OBJECT_TYPE_HWRT_DATASET
 */
struct drm_mtgpu_hwrt_dataset_create_args {
	__u64 pm_data_va_array;
	__u64 tail_ptr_va_array;
	__u64 vheap_table_va;
	__u64 ppp_multi_sample_ctl;
	__u64 pm_secure_data_va_array;
	__u64 free_list_handles;
	__u64 hwrt_dataset_handles_out;
	__u32 isp_merge_lower_x;
	__u32 isp_merge_lower_y;
	__u32 isp_merge_scale_x;
	__u32 isp_merge_scale_y;
	__u32 isp_merge_upper_x;
	__u32 isp_merge_upper_y;
	__u32 ppp_screen;
	__u32 rgn_stride;
	__u32 teaa;
	__u32 temtile1;
	__u32 temtile2;
	__u32 te_screen;
	__u32 tpc_size;
	__u32 tpc_stride;
	__u16 max_rts;
};

/**
 * struct drm_mtgpu_free_list_create_args - Arguments for
 * %DRM_MTGPU_OBJECT_TYPE_FREE_LIST
 *
 * Free list arguments have the following constraints :
 *
 * - &max_num_pages must be greater than zero.
 * - &grow_threshold must be between 0 and 100.
 * - &grow_num_pages must be less than or equal to &max_num_pages.
 * - &initial_num_pages, &max_num_pages and &grow_num_pages must be multiples
 *   of 4.
 *
 * When &grow_num_pages is 0 :
 * - &initial_num_pages must be equal to &max_num_pages
 *
 * When &grow_num_pages is non-zero :
 * - &initial_num_pages must be less than &max_num_pages.
 */
struct drm_mtgpu_free_list_create_args {
	/**
	 * @free_list_base_dev_vaddr: [IN] base dev vaddr of free list
	 */
	__u64 free_list_base_dev_vaddr;

	/**
	 * @free_list_state_dev_vaddr: [IN] state dev vaddr of free list
	 */
	__u64 free_list_state_dev_vaddr;

	/**
	 * @mem_ctx_handle: [IN] private data
	 */
	__u64 mem_ctx_handle;

	/**
	 * @free_list_bo_handle: [IN] pointer to bo
	 */
	__u64 free_list_bo_handle;

	/**
	 * @free_list_bo_offset: [IN] offset in bo
	 */
	__u64 free_list_bo_offset;

	/**
	 * @free_list_state_bo_handlr: [IN] pointer to state bo
	 */
	__u64 free_list_state_bo_handle;

	/**
	 * @free_list_state_bo_offset: [IN] offset in bo state
	 */
	__u64 free_list_state_bo_offset;

	/**
	 * @global_free_list_handle: [IN] get global freelist id
	 */
	__u64 global_free_list_handle;

	/**
	 * @enable_check_sum: [IN] whether enable free list check
	 */
	__u32 enable_check_sum;

	/** @grow_num_pages: [IN] Pages to grow free list by per request. */
	__u32 grow_num_pages;

	/**
	 * @grow_threshold: [IN] Percentage of free list memory used that should
	 * trigger a new grow request.
	 */
	__u32 grow_threshold;

	/** @initial_num_pages: [IN] Pages initially allocated to free list. */
	__u32 initial_num_pages;

	/** @max_num_pages: [IN] Maximum number of pages in free list. */
	__u32 max_num_pages;
};

/**
 * enum drm_mtgpu_object_type - Arguments for
 * &drm_mtgpu_ioctl_create_object_args.type
 */
enum drm_mtgpu_object_type {
	/**
	 * @DRM_MTGPU_OBJECT_TYPE_FREE_LIST: Free list object. Use &struct
	 * drm_mtgpu_free_list_create_args for object creation arguments.
	 */
	DRM_MTGPU_OBJECT_TYPE_FREE_LIST = 0,
	/**
	 * @DRM_MTGPU_OBJECT_TYPE_HWRT_DATASET: HWRT data set. Use &struct
	 * drm_mtgpu_ioctl_create_hwrt_dataset_args for object creation arguments.
	 */
	DRM_MTGPU_OBJECT_TYPE_HWRT_DATASET,

	/**
	 * @DRM_MTGPU_OBJECT_TYPE_INVALID: Mark flag for validation.
	 */
	DRM_MTGPU_OBJECT_TYPE_INVALID,
};

/**
 * struct drm_mtgpu_ioctl_create_object_args - Arguments for
 * %DRM_MTGPU_OBJECT_CREATE
 */
struct drm_mtgpu_object_create {
	struct {
		/**
		 * @type: [IN] Type of object to create.
		 *
		 * This must be one of the values defined by &enum drm_mtgpu_object_type.
		 */
		__u32 type;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;

		/** @data: [IN] User pointer to arguments for specific object type . */
		__u64 data;
	} in;

	struct {
		/**
		 * @handle: [OUT] Handle for created object.
		 */
		__u64 handle;
	} out;
};

/**
 * struct drm_mtgpu_fence_to_fd - Arguments for
 * %DRM_MTGPU_FENCE_TO_FD
 */
struct drm_mtgpu_fence_to_fd {
	struct {
		/**
		 * @fence: [IN] drm_mtgpu_fence
		 * This contains timeline and seqno.
		 */
		struct drm_mtgpu_fence fence;
	} in;

	struct {
		/**
		 * @fd: [IN] fd for drm_mtgpu_fence
		 */
		__s32 fd;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;
	} out;
};

struct drm_mtgpu_semaphore {
	/**
	 * @handle: [IN] handle of semaphore
	 */
	__u64 handle;

	/**
	 * @value: [IN] signal/wait value of semaphore
	 */
	__u64 value;
};

struct drm_mtgpu_semaphore_create {
	struct {
		/**
		 * @handle: [OUT] handle of semaphore
		 */
		__u64 handle;

		/**
		 * @bo_handle: [OUT] bo handle of semaphore
		 */
		__u64 bo_handle;

		/**
		 * @value_offset: [OUT] value offset of semaphore
		 */
		__u64 value_offset;

		/**
		 * @gpu_address: [OUT] gpu/firmware address of semaphore
		 */
		__u64 gpu_address;
	} out;
};

struct drm_mtgpu_semaphore_destroy {
	/**
	 * @handle: [OUT] handle for a semaphore
	 */
	__u64 handle;
};

enum drm_mtgpu_semaphore_type {
	DRM_MTGPU_SEMAPHORE_GPU_SIGNAL = 0,
	DRM_MTGPU_SEMAPHORE_GPU_WAIT,
};

struct drm_mtgpu_semaphore_submit {
	/**
	 * @ctx_handle: [IN] handle of job context.
	 */
	__u64 ctx_handle;

	/**
	 * @job_type: [IN] Type of the job
	 *
	 * This must be one of the values defined by &enum drm_mtgpu_job_type.
	 */
	__u32 job_type;

	/**
	 * @sema_type: [IN] type of the task.
	 */
	__u32 sema_type;

	/**
	 * @semaphore: [IN] drm mtgpu semaphore.
	 */
	struct drm_mtgpu_semaphore semaphore;
};

enum drm_mtgpu_cache_op_type {
	DRM_MTGPU_CACHE_OP_LLC_PERSISTENCE_GET = 0,
	DRM_MTGPU_CACHE_OP_LLC_PERSISTENCE_SET,
	DRM_MTGPU_CACHE_OP_LLC_PERSISTENCE_RESET,
};

struct drm_mtgpu_llc_persistence_in {
	/**
	 * @replace_mode: [IN] replace_mode for RESIDENCY_CTRL register
	 *
	 * Only required when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_SET
	 */
	__u32 replace_mode;

	/**
	 * @replace_mode: [IN] max_set_aside_size for RESIDENCY_CTRL register
	 *
	 * Only required when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_SET
	 */
	__u64 max_set_aside_size;
};

struct drm_mtgpu_llc_persistence_out {
	/**
	 * @llc_size: [OUT] configured llc size
	 *
	 * Only output when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_GET
	 */
	__u32 llc_size;
	/**
	 * @llc_size: [OUT] maximum llc size of the chip
	 * Only output when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_GET
	 */
	__u32 max_llc_persisting_size;
};

struct drm_mtgpu_cache_op {
	struct {
		/**
		 * @type: [IN] Type of cache command.
		 *
		 * This must be one of the values defined by &enum drm_mtgpu_cache_op_type.
		 */
		__u32 type;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;

		/**
		 * @data: [IN] User pointer to arguments for specific cache type
		 */
		__u64 data;
	} in;

	struct {
		/**
		 * @data: [OUT] User pointer to arguments for specific cache type
		 */
		__u64 data;
	} out;
};

enum drm_mtgpu_tl_event_type {
	MTGPU_TL_STREAM_OPEN = 0,
	MTGPU_TL_STREAM_CLOSE,
	MTGPU_TL_STREAM_DISCOVER,
	MTGPU_TL_DATA_ACQUIRE,
	MTGPU_TL_DATA_RELEASE,
	MTGPU_TL_CMD_INVALID,
};

struct drm_mtgpu_transport_layer {
	struct {
		/**
		 * @type: [IN] Type of tl event.
		 * This must be one of the values defined by &enum drm_mtgpu_tl_event_type.
		 */
		__u32 type;

		__u32 pad;

		/** @data: [IN] Handle for tl stream descriptor . */
		__u64 sd_handle;

		/** @data: [IN] User pointer for specific object type . */
		__u64 data;
	} in;

	struct {
		__u64 data;
	} out;
};

struct drm_mtgpu_stream_open_data_in {
	__u8 *name;
	__u32 mode;
};

struct drm_mtgpu_stream_open_data_out {
	__u64 sd_handle;
	__u64 bo_handle;
	__u32 multi_readers_allowed;
	__u32 read_offset;
};

struct drm_mtgpu_discover_stream_data_in {
	__u8 *pattern_name;
	__u32 size;
};

struct drm_mtgpu_discover_stream_data_out {
	__u32 found_count;
	__u8 *stream_name;
};

struct drm_mtgpu_acquire_data_in {
	__u32 read_offset;
};

struct drm_mtgpu_acquire_data_out {
	__u32 read_len;
	__u32 read_offset;
};

struct drm_mtgpu_release_data_in {
	__u32 read_len;
	__u32 read_offset;
};

struct drm_mtgpu_stream_close_data_in {
	__u32 bo_handle;
};

enum drm_mtgpu_hwperf_event_type {
	MTGPU_HWPERF_CTRL = 0,
	MTGPU_HWPERF_INVALID,
};

struct drm_mtgpu_hwperf {
	struct {
		__u32 type;
		__u32 toggle;
		__u32 stream_id;
		__u32 pad;
		__u64 mask;
	} in;

	struct {
		__u64 data;
	} out;
};

#endif /* __MTGPU_DRM_H__ */
