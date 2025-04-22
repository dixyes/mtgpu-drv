/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_H__
#define __MTGPU_DRM_H__

#ifdef __KERNEL__
#include "linux-types.h"
#else
#include "drm.h"
#if defined(__cplusplus)
extern "C" {
#endif
#endif	/* __KERNEL__ */

#ifndef BIT
#define BIT(nr)	((uint64_t)(uintptr_t)1ul << (nr))
#endif

#define MTGPU_API_MAJOR_VERSION			(0)
#define MTGPU_IOCTL_VERSION			(22)
#define MTGPU_API_VERSION_LIST_MAX_COUNT	(256)


/* error occured in a submission, which use for get last error */
#define MTGPU_SEMAPHORE_ERROR_VALUE		(1ULL << 63)			/* FW abnormal response */
#define MTGPU_SEMAPHORE_FAULT_VALUE		((1ULL << 63) | (1ULL << 32))	/* FW hang */

/**
 * Ioctl numbers for mtgpu driver.
 * These represent the primary ioctl commands.
 */
#define DRM_MTGPU_CMD				0x0f

/**
 * CMD type for DRM_MTGPU ioctl.
 * These define the major categories of commands supported by the ioctl interface.
 */
#define MTGPU_CORE_CMD				0x00	/* Core commands related to device initialization and system-level operations */
#define MTGPU_QUERY_CMD				0x01	/* Query commands for retrieving information from the device */
#define MTGPU_BO_CMD				0x02	/* Buffer Object (BO) management commands, including allocation and handle operations */
#define MTGPU_VM_CMD				0x03	/* Virtual Memory (VM) commands, including context management and mapping operations */
#define MTGPU_SYNC_CMD				0x04	/* Synchronization commands for managing job synchronization and dependencies */
#define MTGPU_JOB_CMD				0x05	/* Job commands for managing and scheduling GPU jobs */
#define MTGPU_PERF_CMD				0x06	/* Performance commands for monitoring and optimizing GPU performance */

#define	MTGPU_PLATFORM_HW			0x00
#define	MTGPU_PLATFORM_HAPS			0x01
#define	MTGPU_PLATFORM_EMU			0x02
#define	MTGPU_PLATFORM_VPS			0x03

/**
 * The flag for import global handle ioctl.
 */
#define MTGPU_SEMAPHORE_IMPORT_FLAG_NO_MTLINK (1 << 0)

/*
 * Sub-operation cmds for DRM_MTGPU_CORE_CMD ioctl.
 * These define the specific actions under the core operations.
 */
enum mtgpu_core_cmd {
	MTGPU_CORE_CMD_DEVICE_INIT = 0,		/* Initialize device */
	MTGPU_CORE_CMD_ALIGN_CHECK,		/* Perform alignment check */
	MTGPU_CORE_CMD_VERSION_CHECK,		/* Check driver version */
	MTGPU_CORE_CMD_GET_VERSION_LIST,	/* Get driver version list */
	MTGPU_CORE_CMD_MAX,			/* Maximum value for mtgpu_core_cmd */
};

/*
 * Sub-operation cmds for DRM_MTGPU_QUERY_CMD ioctl.
 * These define the specific actions for querying various types of information.
 */
enum mtgpu_query_cmd {
	MTGPU_QUERY_CMD_HEAP_COUNT = 0,		/* Query the number of memory heaps */
	MTGPU_QUERY_CMD_HEAP_INFO,		/* Query details of memory heaps */
	MTGPU_QUERY_CMD_MEM_INFO,		/* Query memory usage information */
	MTGPU_QUERY_CMD_BO_INFO,		/* Query buffer object (BO) information */
	MTGPU_QUERY_CMD_DEV_INFO,		/* Query device information */
	MTGPU_QUERY_CMD_PCI_INFO,		/* Query PCI information */
	MTGPU_QUERY_CMD_PLATFORM_INFO,		/* Query platform information */
	MTGPU_QUERY_CMD_HW_CAPABILITY,		/* Query hardware capabilities */
	MTGPU_QUERY_CMD_P2P_CAPABILITY,		/* Query P2P capabilities */
	MTGPU_QUERY_CMD_MTLINK_PATH_INFO,	/* Query mtlink path info */
	MTGPU_QUERY_CMD_DRIVER_INFO,		/* Query driver information */
	MTGPU_QUERY_CMD_MISC_INFO,		/* Query misc info */
	MTGPU_QUERY_CMD_MAX,			/* Maximum value for mtgpu_query_cmd */
};

enum mtgpu_bo_cmd {
	MTGPU_BO_CMD_ALLOC = 0,			/* Allocate buffer object (BO) */
	MTGPU_BO_CMD_FROM_USERPTR,		/* Create BO from user pointer */
	MTGPU_BO_CMD_GET_MMAP_OFFSET,		/* Get mmap offset for BO */
	MTGPU_BO_CMD_EXPORT_GLOBAL_HANDLE,	/* Export global handle for BO */
	MTGPU_BO_CMD_IMPORT_GLOBAL_HANDLE,	/* Import BO from global handle */
	MTGPU_BO_CMD_SET_METADATA,		/* Set metadata for bo */
	MTGPU_BO_CMD_ADD_METADATA,		/* Add metadata for bo */
	MTGPU_BO_CMD_GET_METADATA,		/* Get metadata for bo */
	MTGPU_BO_CMD_MAX,			/* Maximum value for mtgpu_bo_cmd */
};

/*
 * Sub-operation cmds for DRM_MTGPU_VM_CMD ioctl.
 * These define the specific actions for virtual memory (VM) management.
 */
enum mtgpu_vm_cmd {
	MTGPU_VM_CMD_CONTEXT_CREATE = 0,	/* Create a new VM context */
	MTGPU_VM_CMD_CONTEXT_DESTROY,		/* Destroy an existing VM context */
	MTGPU_VM_CMD_MAP,			/* Map a resource into GPU address space */
	MTGPU_VM_CMD_UNMAP,			/* Unmap a resource from GPU address space */
	MTGPU_VM_CMD_MAP_ASYNC,			/* Map a resource into GPU address space asynchronous */
	MTGPU_VM_CMD_MAX,			/* Maximum value for mtgpu_vm_cmd */
};

/*
 * Sub-operation cmds for DRM_MTGPU_SYNC_CMD ioctl.
 * These define the specific actions for synchronization functions.
 */
enum mtgpu_sync_cmd {
	MTGPU_SYNC_CMD_TIMELINE_CREATE = 0,		/* Create a sync timeline(for kmd1.5) */
	MTGPU_SYNC_CMD_TIMELINE_DESTROY,		/* Destroy a sync timeline(for kmd1.5) */
	MTGPU_SYNC_CMD_TIMELINE_READ,			/* Read the current sync timeline value(for kmd1.5) */
	MTGPU_SYNC_CMD_FENCE_WAIT,			/* Wait on a sync fence(for kmd1.5) */
	MTGPU_SYNC_CMD_FENCE_TO_FD,			/* Convert a sync fence to a fd(for kmd1.5) */
	MTGPU_SYNC_CMD_SEMAPHORE_CREATE,		/* Create a semaphore */
	MTGPU_SYNC_CMD_SEMAPHORE_DESTROY,		/* Destroy a semaphore */
	MTGPU_SYNC_CMD_SEMAPHORE_SUBMIT,		/* Submit a semaphore signal or wait operation */
	MTGPU_SYNC_CMD_SEMAPHORE_CPU_SIGNAL,		/* Signal a semaphore from the CPU */
	MTGPU_SYNC_CMD_SEMAPHORE_EXPORT_FD,		/* Export a semaphore to a fd */
	MTGPU_SYNC_CMD_SEMAPHORE_IMPORT_FD,		/* Import a semaphore from a fd */
	MTGPU_SYNC_CMD_SEMAPHORE_WAIT,			/* Wait on a semaphore */
	MTGPU_SYNC_CMD_SEMAPHORE_EXPORT_GLOBAL_HANDLE,	/* Export a semaphore to a global handle */
	MTGPU_SYNC_CMD_SEMAPHORE_IMPORT_GLOBAL_HANDLE,	/* Import a semaphore from a global handle */
	MTGPU_SYNC_CMD_MAX,				/* Maximum value for mtgpu_sync_cmd */
};

/*
 * Sub-operation cmds for DRM_MTGPU_JOB_CMD ioctl.
 * These define the specific actions for job management.
 */
enum mtgpu_job_cmd {
	MTGPU_JOB_CMD_CONTEXT_CREATE = 0,		/* Create a job context(for kmd1.5) */
	MTGPU_JOB_CMD_CONTEXT_DESTROY,			/* Destroy a job context(for kmd1.5) */
	MTGPU_JOB_CMD_SUBMIT,				/* Submit a job(for kmd1.5) */
	MTGPU_JOB_CMD_DMA_TRANSFER,			/* Perform a DMA transfer(for kmd1.5) */
	MTGPU_JOB_CMD_OBJECT_CREATE,			/* Create a job-related object(for kmd1.5) */
	MTGPU_JOB_CMD_OBJECT_DESTROY,			/* Destroy a job-related object(for kmd1.5) */
	MTGPU_JOB_CMD_NOTIFY_QUEUE_UPDATE,		/* Notify the queue of a job update(for kmd1.5) */
	MTGPU_JOB_CMD_GET_LLC_PERSISTENCE,		/* Get llc persistence status(for kmd1.5) */
	MTGPU_JOB_CMD_SET_LLC_PERSISTENCE,		/* Set llc persistence status(for kmd1.5) */
	MTGPU_JOB_CMD_RESET_LLC_PERSISTENCE,		/* Reset llc persistence status(for kmd1.5) */
	MTGPU_JOB_CMD_CONTEXT_CREATE_V3,		/* Create a job context(version 3) */
	MTGPU_JOB_CMD_CONTEXT_DESTROY_V3,		/* Destroy a job context(version 3) */
	MTGPU_JOB_CMD_SUBMIT_V3,			/* Submit a job(version 3) */
	MTGPU_JOB_CMD_APPEND,				/* Append operations to an existing job */
	MTGPU_JOB_CMD_CODEC_WAIT,			/* Wait for a codec operation to complete */
	MTGPU_JOB_CMD_GET_SUBMISSION_LAST_ERROR,	/* Get subm error when wait q sem failed */
	MTGPU_JOB_CMD_GET_DEVICE_LAST_ERROR,		/* Get dev error when wait q sem failed for fw hang */
	MTGPU_JOB_CMD_SUBMIT_WITH_DOORBELL,		/* Submit a job with doorbell */
	MTGPU_JOB_CMD_ACQUIRE_DOORBELL,			/* Acquire a doorbell */
	MTGPU_JOB_CMD_RELEASE_DOORBELL,			/* Release a doorbell */
	MTGPU_JOB_CMD_MAX,				/* Maximum value for mtgpu_job_cmd */
};

/*
 * Sub-operation cmds for DRM_MTGPU_PERF_CMD ioctl.
 * These define the specific actions for performance monitoring.
 */
enum mtgpu_perf_cmd {
	MTGPU_PERF_CMD_HWPERF_CONTROL = 0,	/* Enable or disable the generation of HWPerf event packets */
	MTGPU_PERF_CMD_HWPERF_GET_TIMESTAMPS,	/* Get Soc timestamp and OS timestamp */
	MTGPU_PERF_CMD_HWPERF_FLUSH_BUFFER,	/* Flush L1 buffer to L2 buffer */
	MTGPU_PERF_CMD_TL_STREAM_OPEN,		/* Open a descriptor onto an existing transport stream */
	MTGPU_PERF_CMD_TL_STREAM_CLOSE,		/* Close and release the stream connection to kernel transport layer */
	MTGPU_PERF_CMD_TL_STREAM_DISCOVER,	/* Discover streams which names match a given pattern */
	MTGPU_PERF_CMD_TL_STREAM_ACQUIRE,	/* Safely read the address and length of the stream buffer */
	MTGPU_PERF_CMD_TL_STREAM_RELEASE,	/* Flush the outgoing data from the stream buffer to make room for more data */
	MTGPU_PERF_CMD_MSS_PFM_CONFIG,		/* Config MSS PFM config to SMC */
	MTGPU_PERF_CMD_GET_CONTAINER_PID,
	MTGPU_PERF_CMD_MAX,			/* Maximum value for mtgpu_perf_cmd */
};

/*
 * These macro definitions are used for the mtgpu_get_api_version function
 * to get the API version range supported by the driver.
 */
/* Device Initialization Module */
#define MTGPU_API_DEVICE_INIT				0x0	/* which corresponds to mtgpu_device_initialize() */

/* Query Information Module */
#define MTGPU_API_QUERY_HEAP_COUNT			0x100	/* which corresponds to mtgpu_query_heap_count() */
#define MTGPU_API_QUERY_HEAP_INFO			0x101	/* which corresponds to mtgpu_query_heap_info() */
#define MTGPU_API_QUERY_MEM_INFO			0x102	/* which corresponds to mtgpu_query_mem_info() */
#define MTGPU_API_QUERY_BO_INFO				0x103	/* which corresponds to mtgpu_query_bo_info() */
#define MTGPU_API_QUERY_DEV_INFO			0x104	/* which corresponds to mtgpu_query_dev_info() */
#define MTGPU_API_QUERY_PCI_INFO			0x105	/* which corresponds to mtgpu_query_pci_info() */
#define MTGPU_API_QUERY_PLATFORM_INFO			0x106	/* which corresponds to mtgpu_query_platform_info() */
#define MTGPU_API_QUERY_HW_CAPABILITY			0x107	/* which corresponds to mtgpu_query_hw_capability() */
#define MTGPU_API_QUERY_DRIVER_INFO			0x108	/* which corresponds to mtgpu_query_driver_info() */
#define MTGPU_API_QUERY_P2P_CAPABILITY			0x109	/* which corresponds to mtgpu_query_p2p_capability() */
#define MTGPU_API_QUERY_MTLINK_P2P_PATH			0x10a	/* which corresponds to mtgpu_query_mtlink_p2p_path() */
#define MTGPU_API_QUERY_MISC_INFO			0x10b	/* which corresponds to mtgpu_query_misc_info() */

/* Buffer Object Management Module */
#define MTGPU_API_BO_ALLOC				0x200	/* which corresponds to mtgpu_bo_alloc() */
#define MTGPU_API_BO_FROM_USERPTR			0x201	/* which corresponds to mtgpu_bo_create_from_userptr() */
#define MTGPU_API_BO_CPU_MAP				0x202	/* which corresponds to mtgpu_bo_cpu_map() */
#define MTGPU_API_BO_CPU_MAP_FIXED			0x203	/* which corresponds to mtgpu_bo_cpu_map_fixed() */
#define MTGPU_API_BO_EXPORT_GLOBAL_HANDLE		0x204	/* which corresponds to mtgpu_bo_export_global_handle() */
#define MTGPU_API_BO_IMPORT_GLOBAL_HANDLE		0x205	/* which corresponds to mtgpu_bo_import_global_handle() */
#define MTGPU_API_BO_SET_METADATA			0x206	/* which corresponds to mtgpu_bo_set_metadata() */
#define MTGPU_API_BO_ADD_METADATA			0x207	/* which corresponds to mtgpu_bo_add_metadata() */
#define MTGPU_API_BO_GET_METADATA			0x208	/* which corresponds to mtgpu_bo_get_metadata() */

/* Virtual Memory Management Module */
#define MTGPU_API_VM_CONTEXT_CREATE			0x300	/* which corresponds to mtgpu_vm_context_create() */
#define MTGPU_API_VM_CONTEXT_DESTROY			0x301	/* which corresponds to mtgpu_vm_context_destroy() */
#define MTGPU_API_VM_MAP				0x302	/* which corresponds to mtgpu_bo_vm_map() */
#define MTGPU_API_VM_UNMAP				0x303	/* which corresponds to mtgpu_bo_vm_unmap() */
#define MTGPU_API_VM_MAP_ASYNC				0x304	/* which corresponds to mtgpu_bo_vm_map_async() */

/* Semaphore Management Module */
#define MTGPU_API_SEMAPHORE_CREATE			0x400	/* which corresponds to mtgpu_semaphore_create() */
#define MTGPU_API_SEMAPHORE_DESTROY			0x401	/* which corresponds to mtgpu_semaphore_destroy() */
#define MTGPU_API_SEMAPHORE_SIGNAL			0x402	/* which corresponds to mtgpu_semaphore_signal() */
#define MTGPU_API_SEMAPHORE_WAIT			0x403	/* which corresponds to mtgpu_semaphore_wait() */
#define MTGPU_API_SEMAPHORE_SUBMIT_SIGNAL		0x404	/* which corresponds to mtgpu_semaphore_submit_signal() */
#define MTGPU_API_SEMAPHORE_SUBMIT_WAIT			0x405	/* which corresponds to mtgpu_semaphore_submit_wait() */
#define MTGPU_API_SEMAPHORE_EXPORT_FD			0x406	/* which corresponds to mtgpu_semaphore_export_fd() */
#define MTGPU_API_SEMAPHORE_IMPORT_FD			0x407	/* which corresponds to mtgpu_semaphore_import_fd() */
#define MTGPU_API_SEMAPHORE_EXPORT_GLOBAL_HANDLE	0x408	/* which corresponds to mtgpu_semaphore_export_global_handle() */
#define MTGPU_API_SEMAPHORE_IMPORT_GLOBAL_HANDLE	0x409	/* which corresponds to mtgpu_semaphore_import_global_handle() */

/* Job Management Module */
#define MTGPU_API_JOB_CONTEXT_CREATE			0x500	/* which corresponds to mtgpu_job_context_create() */
#define MTGPU_API_JOB_CONTEXT_DESTROY			0x501	/* which corresponds to mtgpu_job_context_destroy() */
#define MTGPU_API_JOB_SUBMIT				0x502	/* which corresponds to mtgpu_job_submit() */
#define MTGPU_API_JOB_APPEND_STREAM			0x503	/* which corresponds to mtgpu_job_append_stream() */
#define MTGPU_API_CODEC_WAIT_BO				0x504	/* which corresponds to mtgpu_codec_wait_bo() */
#define MTGPU_API_GET_SUBMISSION_LAST_ERROR		0x505	/* which corresponds to mtgpu_get_submission_last_error() */
#define MTGPU_API_GET_DEVICE_LAST_ERROR			0x506	/* which corresponds to mtgpu_get_device_last_error() */
#define MTGPU_API_JOB_SUBMIT_WITH_DOORBELL		0x507	/* which corresponds to mtgpu_job_submit_with_doorbell() */
#define MTGPU_API_ACQUIRE_DOORBELL			0x508	/* which corresponds to mtgpu_job_acquire_doorbell() */
#define MTGPU_API_RELEASE_DOORBELL			0x509	/* which corresponds to mtgpu_job_release_doorbell() */
#define MTGPU_API_JOB_CONTEXT_CREATE_WITH_FLAG		0x50A	/* which corresponds to mtgpu_job_context_create_with_flag() */

/* HWPerf and PFM Module */
#define MTGPU_API_HWPERF_CONTROL			0x600	/* which corresponds to mtgpu_hwperf_control() */
#define MTGPU_API_HWPERF_GET_TIMESTAMPS			0x601	/* which corresponds to mtgpu_hwperf_get_timestamps() */
#define MTGPU_API_PFM_SET_MSS_CONFIG			0x602	/* which corresponds to mtgpu_pfm_set_mss_config() */
#define MTGPU_API_TL_DISCOVER_STREAM			0x603	/* which corresponds to mtgpu_tl_discover_stream() */
#define MTGPU_API_TL_OPEN_STREAM			0x604	/* which corresponds to mtgpu_tl_open_stream() */
#define MTGPU_API_TL_ACQUIRE_DATA			0x605	/* which corresponds to mtgpu_tl_acquire_data() */
#define MTGPU_API_TL_RELEASE_DATA			0x606	/* which corresponds to mtgpu_tl_release_data() */
#define MTGPU_API_TL_CLOSE_STREAM			0x607	/* which corresponds to mtgpu_tl_close_stream() */
#define MTGPU_API_HWPERF_FLUSH_BUFFER			0x608	/* which corresponds to mtgpu_hwperf_flush_buffer() */
#define MTGPU_API_HWPERF_GET_CONTAINER_PID		0x609	/* which corresponds to mtgpu_hwperf_get_container_pid() */


/*
 * These macro definitions are used for the mtgpu_get_abi_version function
 * to get the ABI version range supported by the driver.
 */
#define MTGPU_ABI_DMA_CMD			0
#define MTGPU_ABI_CODEC_JOB_DATA		1

enum mtgpu_submission_error_type {
	MTGPU_SUBM_ERROR_TYPE_NONE = 0,		/* Subimssion all finish */
	MTGPU_SUBM_ERROR_TYPE_INVALED_PARAM,	/* Submission cmd or any parameters INVALID */
	MTGPU_SUBM_ERROR_TYPE_ERROR,		/* Submission error, submission abort */
	MTGPU_SUBM_ERROR_TYPE_PAGE_FAULT,	/* Submission fault and FW Halt */
	MTGPU_SUBM_ERROR_TYPE_BREAK,		/* Submission break and partial kick finish, can NOT resume */
	MTGPU_SUBM_ERROR_TYPE_PARTIAL,		/* Submission partial kick finish, can resume */
	MTGPU_SUBM_ERROR_TYPE_QSEMP_TIMEOUT,	/* Submission queue semaphore wait timeout */
	MTGPU_SUBM_ERROR_TYPE_IDLE_TIMEOUT,	/* Submission STALL/EngineSync... happen CSW/RESULE but submission not change */
	MTGPU_SUBM_ERROR_TYPE_MAX,		/* Maximum value for submission error types */
};

enum mtgpu_device_error_type {
	MTGPU_DEVICE_ERROR_TYPE_NONE = 0,		/* No error, device is functioning normally */
	MTGPU_DEVICE_ERROR_TYPE_FW_HANG,		/* Firmware hang detected, submission may not respond */
	MTGPU_DEVICE_ERROR_TYPE_FW_REBOOT_FAILED,	/* Firmware reboot failed, device recovery may not be possible */
	MTGPU_DEVICE_ERROR_TYPE_MTLINK_DOWN,		/* A link down occurred on mtlink and an exception was processed */
	MTGPU_DEVICE_ERROR_TYPE_ECC_ERROR,		/* An ecc error occurred on vram and an exception was processed */
	MTGPU_DEVICE_ERROR_TYPE_MISS_INTERRUPT,		/* Miss gpu interrupt */
	MTGPU_DEVICE_ERROR_TYPE_INJECT_ERROR,		/* Inject error, including through debugfs or gmi */
	MTGPU_DEVICE_ERROR_TYPE_MAX,			/* Maximum value for device error types */
};

/* used for new ioctl num */
#define DRM_IOCTL_MTGPU_CMD \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_MTGPU_CMD, \
		 struct drm_mtgpu_ioctl_args)

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
 *  *              ACCESS PERMISSION FLAGS                   *
 *  *                                                        *
 *  **********************************************************
 */

/*!
 * This flag affects the device MMU protection flags, and specifies
 * that the memory may be read by the GPU.
 */
#define MTGPU_BO_FLAGS_GPU_READABLE			BIT(0)

/*!
 * This flag affects the device MMU protection flags, and specifies
 * that the memory may be written by the GPU.
 */
#define MTGPU_BO_FLAGS_GPU_WRITEABLE			BIT(1)

/*!
 * This flag indicates that the memory may be read and written by the GPU.
 */
#define MTGPU_BO_FLAGS_GPU_READ_WRITE			(MTGPU_BO_FLAGS_GPU_READABLE | MTGPU_BO_FLAGS_GPU_WRITEABLE)

/*!
 * This flag indicates that an allocation is mapped as readable to the CPU.
 */
#define MTGPU_BO_FLAGS_CPU_READABLE			BIT(2)

/*!
 * This flag indicates that an allocation is mapped as writable to the CPU.
 */
#define MTGPU_BO_FLAGS_CPU_WRITEABLE			BIT(3)

/*!
 * This flag indicates that the memory may be read and written by the CPU.
 */
#define MTGPU_BO_FLAGS_CPU_READ_WRITE			(MTGPU_BO_FLAGS_CPU_READABLE | MTGPU_BO_FLAGS_CPU_WRITEABLE)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                      CONTROL FLAGS                     *
 *  *                                                        *
 *  **********************************************************
 */

/*
 * GPU domain
 * ==========
 * The following defines are used to control the GPU cache bit field.
 */

/*!
 * GPU domain. Request cached memory, but not coherent (i.e. no cache
 * snooping). Services will flush the GPU internal caches after every GPU
 * task so no cache maintenance requests from the users are necessary.
 */
#define MTGPU_BO_FLAGS_GPU_CACHED			BIT(4)

/*!
 * This flag indicates uncached memory. This means that any writes to memory
 * allocated with this flag are written straight to memory and thus are
 * coherent for any device in the system.
 */
#define MTGPU_BO_FLAGS_GPU_UNCACHED			BIT(5)

/*!
 * This flag indicates uncached write-combining (WC) memory. This means that
 * sequential writes to memory allocated with this flag are combined to
 * reduce memory access and perform burst writes, potentially improving
 * performance for certain workloads.
 */
#define MTGPU_BO_FLAGS_GPU_UNCACHED_WC			BIT(6)

/*!
 * This flag affects the GPU MMU protection flags.
 * The allocation will be cached.
 * Services will try to set the coherent bit in the GPU MMU tables so the
 * GPU cache is snooping the CPU cache. If coherency is not supported the
 * caller is responsible to ensure the caches are up to date.
 */
#define MTGPU_BO_FLAGS_GPU_CACHE_COHERENT		BIT(7)

/*
 * CPU domain
 * ==========
 * The following defines are used to control the CPU cache bit field.
 */

/*!
 * CPU domain. Request cached memory, but not coherent (i.e. no cache
 * snooping). This means that if the allocation needs to transition from
 * one device to another services has to be informed so it can
 * flush/invalidate the appropriate caches.
 */
#define MTGPU_BO_FLAGS_CPU_CACHED			BIT(9)

/*!
 * This flag indicates uncached memory. This means that any writes to memory
 * allocated with this flag are written straight to memory and thus are
 * coherent for any device in the system.
 */
#define MTGPU_BO_FLAGS_CPU_UNCACHED			BIT(10)

/*!
 * This flag indicates uncached write-combining (WC) memory(if supported). This means that
 * sequential writes to memory allocated with this flag are combined to
 * reduce memory access and perform burst writes, potentially improving
 * performance for certain workloads.
 */
#define MTGPU_BO_FLAGS_CPU_UNCACHED_WC			BIT(11)

/*!
 * This flag affects the CPU MMU protection flags.
 * The allocation will be cached.
 * Services will try to set the coherent bit in the CPU MMU tables so the
 * CPU cache is snooping the GPU cache. If coherency is not supported the
 * caller is responsible to ensure the caches are up to date.
 */
#define MTGPU_BO_FLAGS_CPU_CACHE_COHERENT		BIT(12)

/*
 * P2P domain
 * ==========
 * The following define is used to control P2P memory access.
 */

/*!
 * This flag indicates that the peer memory should be accessed through pcie instead of mtlink.
 */
#define MTGPU_BO_FLAGS_NO_MTLINK_ACCESS			BIT(14)

/*!
 * This flag indicates that kmd will send mmu invalid cmd to fw.
 */
#define MTGPU_BO_FLAGS_MMU_INVALID                     BIT(15)

#define MTGPU_BO_FLAGS_MAPPING_MASK	(MTGPU_BO_FLAGS_GPU_READ_WRITE | \
					 MTGPU_BO_FLAGS_GPU_CACHED | \
					 MTGPU_BO_FLAGS_GPU_UNCACHED | \
					 MTGPU_BO_FLAGS_GPU_UNCACHED_WC | \
					 MTGPU_BO_FLAGS_GPU_CACHE_COHERENT | \
					 MTGPU_BO_FLAGS_NO_MTLINK_ACCESS | \
					 MTGPU_BO_FLAGS_MMU_INVALID)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                   MEMORY ALLOC FLAGS                   *
 *  *                                                        *
 *  **********************************************************
 */

/*!
 * This flag indicates that non-contiguous VRAM can be allocated.
 */
#define MTGPU_BO_FLAGS_NON_CONTIGUOUS			BIT(24)

/*!
 * This flag indicates that memory is allocated only on VRAM.
 */
#define MTGPU_BO_FLAGS_VARM_ONLY			BIT(25)

/*!
 * This flag indicates that the memory allocated is initialized with zeroes.
 */
#define MTGPU_BO_FLAGS_ZERO_ON_ALLOC			BIT(26)

/*!
 * This flag indicates that the allocated memory is scribbled over with a poison value.
 *
 * Not compatible with ZERO_ON_ALLOC
 *
 */
#define MTGPU_BO_FLAGS_POISON_ON_ALLOC			BIT(27)

/*!
 * This flag indicates that the memory is trashed when freed, used when debugging only,
 * not to be used as a security measure.
 */
#define MTGPU_BO_FLAGS_POISON_ON_FREE			BIT(28)

/*!
 * This flag indicates that the memory allocated on gpu affinitive numa node.
 */
#define MTGPU_BO_FLAGS_NUMA_ENABLE			BIT(29)

/*!
 * This flag appends a dummy page when creating user bo.
 * This flag is used in the workaround to the CE dummy write issue on QY2.
 */
#define MTGPU_BO_FLAGS_APPEND_DUMMY_PAGE		BIT(30)

/*
 *  **********************************************************
 *  *                                                        *
 *  *                        USAGE FLAGS                     *
 *  *                                                        *
 *  **********************************************************
 */

/*!
 * This flag indicates that memory is allocated for display.
 */
#define MTGPU_BO_USAGE_DISPLAY				BIT(48)

#define MTGPU_BUFFER_ACCESS_FLAG_READ			0x0
#define MTGPU_BUFFER_ACCESS_FLAG_WRITE			0x1

struct drm_mtgpu_ioctl_args {
	/**
	 * @cmd_type: [IN] Type of the command group
	 *	           This defines the high-level command group type, such as core commands,
	 *	           buffer object (BO) commands, etc.
	 */
	__u32 cmd_type;

	/**
	 * @cmd: [IN] Sub-operation command
	 *	      This specifies the specific command within the command group,
	 *	      for example, MTGPU_CORE_CMD_DEVICE_INIT, MTGPU_BO_CMD_ALLOC, etc.
	 */
	__u32 cmd;

	/**
	 * @checksum: [IN] Sum of struct for align check between libdrm and kmd.
	 */
	__u64 checksum;

	/**
	 * @size: [IN] Size of the data structure
	 *	       This represents the size of the data structure being passed via the `data` field.
	 */
	__u32 size;

	/**
	 * @pad: just for padding
	 */
	__u32 pad;	/* IGNORE ALIGN CHECK */

	/**
	 * @data: [IN/OUT] Pointer to the data structure
	 *	           A 64-bit pointer to the specific data structure needed for the command execution.
	 *	           The structure may vary depending on the command being used,
	 *	           such as device init, alignment check, etc.
	 */
	__u64 data;
};

#define MTGPU_COMMIT_STRING_LENGTH (32)

struct drm_mtgpu_device_init {
	struct {
		__u32 api_major_version;
		__u32 pad;	/* IGNORE ALIGN CHECK */
		char libdrm_version[MTGPU_COMMIT_STRING_LENGTH];
		char shared_inc_version[MTGPU_COMMIT_STRING_LENGTH];
	} in;
};

#define MTGPU_HEAPNAME_MAXLENGTH (128)
#define MTGPU_CORE_COUNT_MAX (32)

struct drm_mtgpu_heap_info { /* IGNORE STRUCT */
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

	/*
	 * Whether this heap can use multi page size.
	 */
	__u32 enable_multi_page_size;

	/*
	 * Include at most three page size, because the more page size
	 * number, the worse mmu performance.
	 * For example, 0x5000 = (1 << 12) | (1 << 14), means we use 4k
	 * and 16k.
	 */
	__u32 page_shift_bit_mask;

	/*
	 * When multi page size is enabled, the minimum size of PA allocated
	 * by one allocation.
	 */
	__u32 log2_import_alignment;

};

struct mtgpu_mem_info { /* IGNORE STRUCT */
	struct {
		__u64 total_size;
		__u64 free_size;
	} system;
	struct {
		__u64 hw_size;
		__u64 total_size;
		__u64 free_size;
	} vram;
};

struct mtgpu_bo_info { /* IGNORE STRUCT */
	__u64 size;
	__u64 align;
	__u64 flags;
	__u32 domain;
	__u32 segment_id;
	__u64 metadata_addr;
	__u64 metadata_size;
	__u64 metadata_id;
	__u32 has_metadata;
	__u32 pad; /* IGNORE ALIGN CHECK */
};

#define MTGPU_DEVICE_MARKETING_NAME_SIZE 48 /* 48 Bytes */

struct mtgpu_dev_info { /* IGNORE STRUCT */
	__u32 dev_id;
	__u8 marketing_name[MTGPU_DEVICE_MARKETING_NAME_SIZE];
	__u32 dev_status;
	__u32 dev_clock_speed;
	__u32 mem_clock_speed;
	__u32 mem_max_clock_speed;
	__u32 num_cores;
	__u32 mpx_map;
	__u32 soc_timer_clock_speed;
	__u8 uuid[16];
	__u16 subvendor_id;
	__u16 subsystem_id;
	__u32 llc_persisting_hw_max_size;
	__u32 llc_size;
	__u8 is_igpu;
};

struct mtgpu_pci_info { /* IGNORE STRUCT */
	/**
	 * @domain_number: [OUT] PCI domian number.
	 */
	__u32 domain_number;
	/**
	 * @bus_number: [OUT] PCI bus number.
	 */
	__u32 bus_number;
	/**
	 * @device_number: [OUT] PCI device number.
	 */
	__u32 device_number;
	/**
	 * @function_number: [OUT] PCI function number.
	 */
	__u32 function_number;
	/**
	 * @current_speed: [OUT] current PCI gen speed.
	 * Gen1:2.5GT Gen2:5GT Gen3:8GT Gen4:16GT Gen5:32GT
	 */
	__u32 current_gen_speed;
	/**
	 * @current_width: [OUT] current PCI width.
	 */
	__u32 current_width;
	/**
	 * @numa_node_id: [OUT] numa node for PCI device.
	 * disable: -1 enable : > 0
	 */
	__s32 numa_node_id;
	/**
	 * @total_pci_device_memory_accessible: [OUT] pci device memory access capability
	 * can not dirctly access all pci device memory: 0
	 * can dirctly access all device memory: 1
	 */
	__u8 total_pci_device_memory_accessible;
	/**
	 * @total_system_memory_accessible: [OUT] system memory access capability
	 * can not dirctly access all system memory: 0
	 * can dirctly access all system memory: 1
	 */
	__u8 total_system_memory_accessible;
	/**
	 * @no_snoop: [OUT] whether gpu can snoop cpu cache
	 * support snoop, gpu mmu will maintain coherency with cpu cache: 0
	 * do not support snoop, gpu mmu will not maintain coherency with cpu cache,  : 1
	 */
	__u8 no_snoop;

	/**
	 * @subvendor_id: [OUT] subvendor id for PCI device
	 */
	__u16 subvendor_id;

	/**
	 * subsystem_id: [OUT] susystem id for PCI device
	 */
	__u16 subsystem_id;

	/**
	 * @reserved: [IN] for padding
	 */
	__u8 reserved;	/* IGNORE ALIGN CHECK */
};

struct mtgpu_platform_info { /* IGNORE STRUCT */
	/**
	 * @mtlink_enable: [OUT] Status of mtlink.
	 * disable: 0 enable: 1
	 */
	__u8 mtlink_enable;
	/**
	 * @mtlink_enable: [OUT] Status of iommu.
	 * disable: 0 enable: 1
	 */
	__u8 iommu_enable;
	/**
	 * @is_vps: whether run in vps
	 * not in vps: 0
	 * in vps: 1
	 */
	__u8 is_vps;
	/**
	 * @platform_type: current platform type
	 * platform_type: emu/haps/hw/vps
	 */
	__u8 platform_type;
	/**
	 * @direct_cache_access_support:
	 * support direct access cpu cache : 1
	 * do not support : 0
	 */
	__u8 direct_cache_access_support;
};

struct mtgpu_hw_capability { /* IGNORE STRUCT */
	/**
	 * @llc: [OUT] Status of llc operation support.
	 * unsupport: 0 support: 1
	 */
	__u64 support_llc : 1;

	/**
	 * @ce: [OUT] Status of copy engine support.
	 * unsupport: 0 support: 1
	 */
	__u64 support_ce : 1;

        /**
         * @dma: [OUT] Status of dma support.
         * unsupport: 0 support: 1
         */
        __u64 support_dma : 1;

	/**
	 * @reserved: [IN] reserved for future
	 */
	__u64 reserved : 61;
};

struct mtgpu_driver_info { /* IGNORE STRUCT */
	/**
	 * @fec_sched: [OUT] Status of fec schedule.
	 * unsupport: 0 support: 1
	 */
	__u8 fec_sched;

	/**
	 * @drm_sched: [OUT] Status of drm scheduler.
	 * unsupport: 0 support: 1
	 */
	__u8 drm_sched;

	/**
	 * @drm_sched: [OUT] Status of scheduler.
	 */
	__u8 sched_mode;

	/**
	 * @reserved: [IN] reserved for future
	 */
	__u8 reserved[5];
};

/* The input argument for mtgpu_p2p_capability is peer_drm_fd */
struct mtgpu_p2p_capability { /* IGNORE STRUCT */
	/**
	 * @pci_capability: [OUT] pci p2p capability
	 */
	__u32 pci_capability;

	/**
	 * @mtlink_capability: [OUT] mtlink p2p capability
	 */
	__u32 mtlink_capability;

	/**
	 * @mtlink_version: [OUT] mtlink version
	 */
	__u32 mtlink_version;

	/**
	 * @mtlink_bandwidth: [OUT] mtlink bandwidth
	 */
	__u32 mtlink_bandwidth;

	/**
	 * @mtlink_link_num: [OUT] mtlink link port num between loacl and peer device
	 * 0: The two devices are not directly connected by mtlink
	 */
	 __u32 mtlink_link_num;
};

#define MTGPU_MAX_MTLINK_PATH_NUM	256
#define MTGPU_MAX_DEVICE_NUM		16

struct mtgpu_mtlink_path {
	/**
	 * @length: [OUT] path length
	 */
	__u32 length;

	/**
	 * @pad: [OUT] for padding
	 */
	__u32 pad; /* IGNORE ALIGN CHECK */

	/**
	 * @path_node: [OUT] path length
	 */
	__u32 path_node[MTGPU_MAX_DEVICE_NUM];
};

/* The input argument for mtgpu_mtlink_path_info is peer_drm_fd */
struct mtgpu_mtlink_path_info { /* IGNORE STRUCT */
	/**
	 * @path: [OUT] shortest path
	 */
	struct mtgpu_mtlink_path path[MTGPU_MAX_MTLINK_PATH_NUM]; /* IGNORE ALIGN CHECK */

	/**
	 * @path_num: [OUT] number of path
	 */
	__u32 path_num;

	/**
	 * @pad: [OUT] for padding
	 */
	__u32 pad; /* IGNORE ALIGN CHECK */
};

struct drm_mtgpu_query_info { /* IGNORE STRUCT */
	struct {
		__u32 type;
		__u32 pad;	/* IGNORE ALIGN CHECK */
		__u64 data;
	} in;

	struct {
		__u64 data;
	} out;
};

struct drm_mtgpu_query_heap_count {
	struct {
		__u32 heap_count;
	} out;
};

struct drm_mtgpu_query_heap_info {
	struct {
		__u32 index;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;

	struct {
		__u32 id;
		char name[MTGPU_HEAPNAME_MAXLENGTH];
		__u64 base;
		__u64 length;
		__u32 log2_page_size;
		__u32 enable_multi_page_size;
		__u32 page_shift_bit_mask;
		__u32 log2_import_alignment;
	} out;
};

struct drm_mtgpu_query_mem_info {
	struct {
		__u64 vram_hw_size;
		__u64 vram_total_size;
		__u64 vram_free_size;
		__u64 sysmem_total_size;
		__u64 sysmem_free_size;
	} out;
};

#define MTGPU_BO_NAME_MAX_LEN (64)

struct drm_mtgpu_query_bo_info {
	struct {
		__u32 bo_handle;
		__u32 metadata_id;
		__u64 metadata_addr;
		__u64 metadata_size;
	} in;

	struct {
		__u64 size;
		__u64 align;
		__u64 flags;
		__u32 domain;
		__u32 segment_id;
		__u32 has_metadata;
		__u32 pad; /* IGNORE ALIGN CHECK */
		char name[MTGPU_BO_NAME_MAX_LEN];
	} out;
};

struct drm_mtgpu_query_dev_info {
	struct {
		__u32 dev_id;
		__u8 marketing_name[MTGPU_DEVICE_MARKETING_NAME_SIZE];
		__u32 dev_status;
		__u32 dev_clock_speed;
		__u32 mem_clock_speed;
		__u32 mem_max_clock_speed;
		__u32 soc_timer_clock_speed;
		__u32 num_cores;
		__u32 mpx_map;
		__u8 uuid[16];
		__u16 subvendor_id;
		__u16 subsystem_id;
		__u32 llc_persisting_hw_max_size;
		__u32 llc_size;
		__u8 is_igpu;
		__u8 pad[7]; /* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_query_pci_info {
	struct {
		__u32 domain_number;
		__u32 bus_number;
		__u32 device_number;
		__u32 function_number;
		__u32 current_gen_speed;
		__u32 current_width;
		__s32 numa_node_id;
		__u8 total_pci_device_memory_accessible;
		__u8 total_system_memory_accessible;
		__u8 no_snoop;
		__u16 subvendor_id;
		__u16 subsystem_id;
		__u8 reserved[5];	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_query_platform_info {
	struct {
		__u8 mtlink_enable;
		__u8 iommu_enable;
		__u8 is_vps;
		__u8 platform_type;
		__u8 direct_cache_access_support;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_query_hw_capability { /* IGNORE STRUCT */
	struct {
		__u64 support_llc : 1;
		__u64 support_ce : 1;
		__u64 support_dma : 1;
		__u64 reserved : 61;
	} out;
};

/**
 * @brief Macros defining scheduling modes of the MTGPU driver.
 * These macros represent different operating states of the MTGPU driver,
 * guiding task and resource management and enabling communication between
 * kernel - and user - space components.
 */
#define MTGPU_SCHED_MODE_META_ONLY	0
#define MTGPU_SCHED_MODE_DRM_NODEQ	1
#define MTGPU_SCHED_MODE_DRM_CCBQ	2
#define MTGPU_SCHED_MODE_FEC		3

struct drm_mtgpu_query_driver_info {
	struct {
		__u8 sched_mode;
		__u8 reserved[7];	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_query_p2p_capability {
	struct {
		__u32 peer_fd;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;

	struct {
		__u32 pci_capability;
		__u32 mtlink_capability;
		__u32 mtlink_version;
		__u32 mtlink_bandwidth;
		__u32 mtlink_link_num;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_misc_info {
	struct {
		__u32 misc_count;
		__u32 misc_id[MTGPU_CORE_COUNT_MAX];	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_mtlink_path_info {
	struct {
		__u32 peer_fd;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;

	struct {
		/**
		* @path: [OUT] shortest path
		*/
		struct mtgpu_mtlink_path path[MTGPU_MAX_MTLINK_PATH_NUM]; /* IGNORE ALIGN CHECK */

		/**
		* @path_num: [OUT] number of path
		*/
		__u32 path_num;

		/**
		* @pad: [OUT] for padding
		*/
		__u32 pad; /* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_bo_create {
	struct {
		__u64 size;
		__u64 align;
		__u64 flags;
		__u32 domains;
		__u32 group_id;
		char name[MTGPU_BO_NAME_MAX_LEN];
	} in;

	struct {
		__u32 bo_handle;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_bo_from_userptr {
	struct {
		__u64 userptr;
		__u64 size;
		__u32 flags;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;

	struct {
		__u32 bo_handle;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_bo_get_mmap_offset {
	struct {
		__u32 bo_handle;
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;

	struct {
		__u64 offset;
	} out;
};

struct drm_mtgpu_bo_global_handle_export {
	struct {
		/**
		 * @bo_handle: [IN] Handle for exported buffer object.
		 */
		__u32 bo_handle;
		/**
		 * @pad: [IN] for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;

	struct {
		/**
		 * @global_handle: [OUT] Global handle for exported buffer object.
		 */
		__u64 global_handle;
	} out;
};

struct drm_mtgpu_bo_global_handle_import {
	struct {
		/**
		 * @global_handle: [IN] Global handle for exported buffer object.
		 */
		__u64 global_handle;
	} in;

	struct {
		/**
		 * @size: [OUT] size of imported buffer object.
		 */
		__u64 size;
		/**
		 * @bo_handle: [OUT] Handle for imported buffer object.
		 */
		__u32 bo_handle;
		/**
		 * @pad: [OUT] for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_bo_set_metadata {
	/**
	 * @bo_handle: [IN] Handle for exported buffer object.
	 */
	__u64 bo_handle;

	/**
	 * @metadata_addr: [IN] user space addr of metadata.
	 */
	__u64 metadata_addr;

	/**
	 * @metadata_size: [IN] size of metadata.
	 */
	__u64 metadata_size;
};

struct drm_mtgpu_bo_add_metadata {
	/**
	 * @bo_handle: [IN] Handle for exported buffer object.
	 */
	__u64 bo_handle;

	/**
	 * @metadata_id: [IN] id of metadata.
	 */
	__u64 metadata_id;

	/**
	 * @metadata_addr: [IN] user space addr of metadata.
	 */
	__u64 metadata_addr;

	/**
	 * @metadata_size: [IN] size of metadata.
	 */
	__u64 metadata_size;
};

struct drm_mtgpu_bo_get_metadata {
	/**
	 * @bo_handle: [IN] Handle for exported buffer object.
	 */
	__u64 bo_handle;

	/**
	 * @metadata_id: [IN] id of metadata.
	 */
	__u64 metadata_id;

	/**
	 * @metadata_addr: [IN] user space addr of metadata.
	 */
	__u64 metadata_addr;

	/**
	 * @metadata_size: [IN] size of metadata.
	 */
	__u64 metadata_size;
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
	__u32 pad;	/* IGNORE ALIGN CHECK */
	/**
	 * @size: [IN] Size of the requested mapping. Must be aligned to
	 * the device page size for the heap containing the requested address,
	 * as well as the host page size.
	 */
	__u64 size;

	/*
	 * The page size of gpu va.
	 */
	__u32 log2_page_size;
};

struct drm_mtgpu_vm_map_async {
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
	__u32 pad;	/* IGNORE ALIGN CHECK */
	/**
	 * @size: [IN] Size of the requested mapping. Must be aligned to
	 * the device page size for the heap containing the requested address,
	 * as well as the host page size.
	 */
	__u64 size;
	/*
	 * The page size of gpu va.
	 */
	__u32 log2_page_size;
	/**
	 * @check_semaphore_count: [IN] check semaphore count
	 */
	__u32 update_semaphore_count;
	/**
	 * @check_semaphores: [IN] handle array of check semaphores
	 */
	__u64 update_semaphore;
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

struct drm_mtgpu_timeline_create { /* IGNORE STRUCT */
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

struct drm_mtgpu_timeline_destroy { /* IGNORE STRUCT */
	struct {
		/**
		 * @timeline_handle: [IN] handle of fence timeline
		 */
		__u64 timeline_handle;
	} in;
};

struct drm_mtgpu_timeline_read { /* IGNORE STRUCT */
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

struct drm_mtgpu_fence { /* IGNORE STRUCT */
	/**
	 * @timeline_handle: [IN] handle of fence timeline
	 */
	__u64 timeline_handle;

	/**
	 * @seqno: [IN] fence sequence number
	 */
	__u64 seqno;
};

struct drm_mtgpu_fence_wait { /* IGNORE STRUCT */
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
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

/* context related */
struct drm_mtgpu_context_create { /* IGNORE STRUCT */
	struct {
		/**
		 * @type: [IN] Type of the context to be created
		 *
		 * This must be one of the values defined by &enum drm_mtgpu_job_type.
		 */
		__u32 type;

		/**
		 * @flags: [IN] Flags of the context to be created
		 */
		__u32 flags;

		/**
		 * @ccbsize: [IN] ccbsize of the context to be created
		 */
		__u32 ccbsize;

		/**
		 * @priority: [IN] Priority of new context.
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

struct drm_mtgpu_context_destroy { /* IGNORE STRUCT */
	/**
	 * @type: [IN] Type of the context to be created
	 *
	 * This must be one of the values defined by &enum drm_mtgpu_job_type.
	 */
	__u32 type;

	/**
	 * @pad: [IN] for padding
	 */
	__u32 pad;	/* IGNORE ALIGN CHECK */

	/** @ctx_handle: [IN] handle of job context. */
	__u64 ctx_handle;
};

/* 1: kmd set job context no skip status and not submit SKIP CMD when response error.
 * All of later job submission use this context will return error.
 */
#define MTGPU_JOB_CONTEXT_FLAGS_NO_SKIP			BIT(0)

/* definition of drm_mtgpu_context_create for ddk2.0 */
struct drm_mtgpu_job_context_create {
	struct {
		/**
		 * @type: [IN] Type of the context to be created
		 *
		 * This must be one of the values defined by &enum drm_mtgpu_job_submission_type.
		 */
		__u32 type;

		/*
		 * @flags: [IN] Flags of the context to be created
		 * */
		__u32 flags;

		/**
		 * @priority: [IN] Priority of new context.
		 */
		__u32 priority;

		/**
		 * @vm_ctx_handle: [IN] handle of vm context.
		 */
		__u64 vm_ctx_handle;
	} in;

	struct {
		/**
		 * @ctx_handle: [OUT] handle of job context.
		 */
		__u64 ctx_handle;
	} out;
};

struct drm_mtgpu_job_context_destroy { /* IGNORE STRUCT */
	/** @ctx_handle: [IN] handle of job context. */
	__u64 ctx_handle;
};

struct drm_mtgpu_tq_context_data { /* IGNORE STRUCT */
	/** @robustness_addr: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
};

struct drm_mtgpu_render_context_data { /* IGNORE STRUCT */
	/** @robustness_addr: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
	/** @max_3d_deadline_ms: [IN] Max 3D deadline limit in MS. */
	__u32 max_3d_deadline_ms;
	/** @max_ta_deadline_ms: [IN] Max TA deadline limit in MS. */
	__u32 max_ta_deadline_ms;
};

struct drm_mtgpu_compute_context_data { /* IGNORE STRUCT */

	/** @robustness_address: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
	/** @robustness_address: [IN] Max deadline limit in MS. */
	__u32 max_deadline_ms;
};

struct drm_mtgpu_ce_context_data { /* IGNORE STRUCT */
	/** @framework_cmd: [IN] Framework command. */
	__u64 framework_cmd;
	/** @framework_cmd: [IN] Framework command size. */
	__u32 framework_cmd_size;
	/** @robustness_addr: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
};

struct drm_mtgpu_dma_context_data { /* IGNORE STRUCT */
	 /** @robustness_address: [IN] GPU VA which describe context reset reason. */
	__u64 robustness_addr;
};

enum drm_mtgpu_job_type {
	MTGPU_JOB_NOP = 0,
	MTGPU_JOB_TQ,
	MTGPU_JOB_CE,
	MTGPU_JOB_RENDER,
	MTGPU_JOB_COMPUTE,
	MTGPU_JOB_DMA,
	MTGPU_JOB_INVALID,
};

struct drm_mtgpu_compute_job_data { /* IGNORE STRUCT */
	__u32 num_of_workgroups;
	__u32 num_of_workitems;
};

struct drm_mtgpu_tq_job_data { /* IGNORE STRUCT */
	__u32 characteristic1;
	__u32 characteristic2;
};

struct drm_mtgpu_ce_job_data { /* IGNORE STRUCT */
	__u32 characteristic1;
	__u32 characteristic2;
};

enum drm_mtgpu_dma_addr_type  {
	MTGPU_DMA_ADDR_TYPE_USER_PTR = 0,
	MTGPU_DMA_ADDR_TYPE_BO_HANDLE,
};

struct drm_mtgpu_dma_cmd {
	/** @abi_version: [IN] drm_mtgpu_dma_cmd version. */
	__u64 abi_version;
	/** @abi_version: [IN] drm_mtgpu_dma_cmd checksum. */
	__u64 abi_checksum;
	/** @src_type: [IN] Src data type of DMA transfer. */
	enum drm_mtgpu_dma_addr_type src_type;
	/** @dst_type: [IN] Dst data type of DMA transfer. */
	enum drm_mtgpu_dma_addr_type dst_type;
	/** @src_addr: [IN] Src data addr of DMA transfer. */
	__u64 src_addr;
	/** @dst_addr: [IN] Dst data addr of DMA transfer. */
	__u64 dst_addr;
	/** @src_offset: [IN] Src data address offset of DMA transfer. */
	__u64 src_offset;
	/** @dst_offset: [IN] Dst data address offset of DMA transfer. */
	__u64 dst_offset;
	/** @xfer_size: [IN] Size of DMA transfer. */
	__u64 xfer_size;
};

struct drm_mtgpu_render_job_data { /* IGNORE STRUCT */
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
	__u32 pad1;	/* IGNORE ALIGN CHECK */

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
	__u32 pad2;	/* IGNORE ALIGN CHECK */

	/**
	 * @frag_foreign_fence_fd: [IN] the fd of 3D foreign fence
	 */
	__s32 frag_foreign_fence_fd;

	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad3;	/* IGNORE ALIGN CHECK */

	/**
	 * @frag_cmd_array: [IN] 3D dm command buffer array
	 */
	__u8 *frag_cmd_array;

	/**
	 * @frag_cmd_size: [IN] 3D dm command size
	 */
	__u32 frag_cmd_size;

	/**
	 * @frag_cmd_count: [IN] the count of 3D dm command
	 */
	__u32 frag_cmd_count;

	/**
	 * @frag_pr_cmd_array: [IN] 3D PR dm command buffer array
	 */
	__u8 *frag_pr_cmd_array;

	/**
	 * @frag_pr_cmd_size: [IN] 3D PR dm command size
	 */
	__u32 frag_pr_cmd_size;

	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad4;	/* IGNORE ALIGN CHECK */

	/**
	 * @hwrt_dataset_handle: [IN] the handle of hardware render target
	 */
	__u64 hwrt_dataset_handle;

	/**
	 * @msaa_scratch_buffer_handle: [IN] the handle of msaa scratch buffer
	 */
	__u64 msaa_scratch_buffer_handle;

	/**
	 * @zs_buffer_handle: [IN] the handle of zs buffer
	 */
	__u64 zs_buffer_handle;

	/**
	 * @draw_calls_number: [IN] the count of draw call
	 */
	__u32 draw_calls_number;

	/**
	 * @indices_number: [IN] the count of index
	 */
	__u32 indices_number;

	/**
	 * @mrts_number: [IN] the count of mrts
	 */
	__u32 mrts_number;

	/**
	 * @render_target_size: [IN] the size of hardware render target
	 */
	__u32 render_target_size;

	/**
	 * @kick_geom: [IN] whether kick geometry
	 */
	__u32 kick_geom;

	/**
	 * @kick_pr: [IN] whether partial render
	 * 		actually, if kick_frag is true, kick_pr will be true. 
	 */
	__u32 kick_pr;

	/**
	 * @kick_geom: [IN] whether kick geometry
	 */
	__u32 kick_frag;

	/**
	 * @abort: [IN] app uses it to destroy this surface when some config is wrong.
	 */
	__u32 abort;
};

struct drm_mtgpu_job_submit { /* IGNORE STRUCT */
	struct {
		/**
		 * @type: [IN] Type of the job
		 * This must be one of the values defined by &enum drm_mtgpu_job_type.
		 */
		__u32 type;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad1;	/* IGNORE ALIGN CHECK */

		/**
		 * @ctx_handle: [IN] handle of job context.
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
		__u32 pad2;	/* IGNORE ALIGN CHECK */

		/**
		 * @update_semaphores: [IN] update semaphore array
		 */
		__u64 update_semaphores;
		/**
		 * @update_semaphore_count: [IN] update semaphore count
		 */
		__u32 update_semaphore_count;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad3;	/* IGNORE ALIGN CHECK */

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
		 * @sync_buf_flags: [IN] array of sync buf flag. 0x1 is wirting.
		 */
		__u32 *sync_buf_flags;
		/**
		 * @sync_buf_count: [IN] the count of sync buf
		 */
		__u32 sync_buf_count;
		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad4;	/* IGNORE ALIGN CHECK */

		/**
		 * @update_fence: [IN] update fence
		 */
		__u64 update_fence;
		/**
		 * @update_fence_name: [IN] update fence name
		 */
		const char *update_fence_name;

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

enum drm_mtgpu_job_submission_type {
	MTGPU_SUBMISSION_NOP = 0,
	MTGPU_SUBMISSION_GPU_NOP,
	MTGPU_SUBMISSION_GPU_TQ,
	MTGPU_SUBMISSION_GPU_CE,
	MTGPU_SUBMISSION_GPU_GFX,
	MTGPU_SUBMISSION_GPU_COMPUTE,
	MTGPU_SUBMISSION_GPU_COMPUTE_STREAM,
	MTGPU_SUBMISSION_GPU_UNIVERSAL,
	MTGPU_SUBMISSION_DMA,
	MTGPU_SUBMISSION_CODEC,
	MTGPU_SUBMISSION_INVALID,
};

/* kmd will not execute gpu reset and subsequent cmd sending behavior by default. */
#define MTGPU_SUBMISSION_FLAGS_DISABLE_HWR		BIT(0)

/* FW schedule pause if this submission has ERROR. */
#define MTGPU_SUBMISSION_FLAGS_ERROR_PAUSE		BIT(1)

/* FW schedule pause when this submission finish. */
#define MTGPU_SUBMISSION_FLAGS_USER_PAUSE		BIT(2)

/* FEC is bypassed to provide a fast path for some cases. */
#define MTGPU_SUBMISSION_FLAGS_FAST_PATH		BIT(3)

/* enable job submit asynchronously in kmd */
#define MTGPU_SUBMISSION_FLAGS_SUBMIT_ASYNC		BIT(4)

/* 0: This submission will block all others follow it */
#define MTGPU_SUBMISSION_FLAGS_OUT_OF_ORDER		BIT(5)

/* resubmit stalled submission */
#define MTGPU_SUBMISSION_FLAGS_USER_RESUBMIT		BIT(6)

/*
 * This submission will be scheduled immediately when its dependent 
 * submission was in the same subm queue and has already been scheduled.
 */
#define MTGPU_SUBMISSION_FLAGS_EARLY_SUBMIT		BIT(7)

/* definition of drm_mtgpu_job_submit for ddk2.0 */
struct drm_mtgpu_job_submit_v3 {
	struct {
		/**
		 * @ctx_handle: [IN] handle of job context.
		 */
		__u64 job_ctx_handle;

		/**
		 * @check_semaphores: [IN] handle array of check semaphores
		 */
		__u64 check_semaphores;
		/**
		 * @check_semaphore_count: [IN] check semaphore count
		 */
		__u32 check_semaphore_count;
		/**
		 * @pad: just for padding
		 */
		__u32 pad1;	/* IGNORE ALIGN CHECK */

		/**
		 * @update_semaphores: [IN] handle array of update semaphores
		 */
		__u64 update_semaphores;
		/**
		 * @update_semaphore_count: [IN] update semaphore count
		 */
		__u32 update_semaphore_count;
		/**
		 * @pad: just for padding
		 */
		__u32 pad2;	/* IGNORE ALIGN CHECK */
		/**
		 * @buf_sync_fds: [IN] dmabuf sync fd array
		 */
		__u64 buf_sync_fds;
		/**
		 * @buf_sync_flags: [IN] array of read/write flags, 0x1 is write
		 */
		__u64 buf_sync_flags;
		/**
		 * @buf_sync_count: [IN] the count of buffer sync
		 */
		__u32 buf_sync_count;
		/**
		 * @pad: just for padding
		 */
		__u32 pad3;	/* IGNORE ALIGN CHECK */
		/**
		 * @submissions: [IN] va of this submission
		 * gpu_va for gpu job; cpu_va for dma job
		 */
		__u64 submission_va;
		/**
		 * @submission_size: [IN] size of this submission
		 */
		__u32 submission_size;
		/**
		 * @submission_flags: [IN] user flags for this submission
		 */
		__u32 submission_flags;
		/**
		 * @submission_id: [IN] id updated by submission makers
		 * to track submitted job (for profiling purpose)
		 */
		__u64 submission_id;
	} in;

	struct {
		/**
		 * @data: [OUT] private data of the specific job type.
		 */
		__u64 data;
	} out;
};

struct drm_mtgpu_job_append {
	/**
	 * @ctx_handle: [IN] handle of job context.
	 */
	__u64 job_ctx_handle;

	/**
	 * @stream_uid: [IN] uniqueu id of stream submission
	 */
	__u64 stream_uid;
};

struct drm_mtgpu_job_submit_with_doorbell {
	struct {
		/**
		 * @ctx_handle: [IN] handle of job context.
		 */
		__u64 job_ctx_handle;

		/**
		 * @check_semaphores: [IN] handle array of check semaphores
		 */
		__u64 check_semaphores;
		/**
		 * @check_semaphore_count: [IN] check semaphore count
		 */
		__u32 check_semaphore_count;
		/**
		 * @pad: just for padding
		 */
		__u32 pad1;	/* IGNORE ALIGN CHECK */

		/**
		 * @update_semaphores: [IN] handle array of update semaphores
		 */
		__u64 update_semaphores;
		/**
		 * @update_semaphore_count: [IN] update semaphore count
		 */
		__u32 update_semaphore_count;
		/**
		 * @pad: just for padding
		 */
		__u32 pad2;	/* IGNORE ALIGN CHECK */
		/**
		 * @submissions: [IN] va of this submission
		 * gpu_va for gpu job; cpu_va for dma job
		 */
		__u64 submission_va;
		/**
		 * @submission_size: [IN] size of this submission
		 */
		__u32 submission_size;
		/**
		 * @submission_flags: [IN] user flags for this submission
		 */
		__u32 submission_flags;
		/**
		 * @submission_id: [IN] id updated by submission makers
		 * to track submitted job (for profiling purpose)
		 */
		__u64 submission_id;
		/**
		 * @doorbell_handle: [IN] doorbell_handle of this submission
		 */
		__u32 doorbell_handle;
		/**
		 * @pad: just for padding
		 */
		__u32 pad3;	/* IGNORE ALIGN CHECK */
	} in;
};

struct drm_mtgpu_job_acquire_doorbell {
	struct {
		/**
		 * @job_ctx_handle: [IN] handle of job context.
		 */
		__u64 job_ctx_handle;
		/**
		 * @user_va: [in] userspace cpu_va from mmap().
		 */
		__u64 user_va;
	} in;

	struct {
		/**
		 * @doorbell_handle: [OUT] available doorbell handle.
		 */
		__u32 doorbell_handle;
		/**
		 * @doorbell_addr_offset: [OUT] page offset of the physical page where the doorbell is located.
		 */
		__u32 doorbell_addr_offset;
	} out;
};

struct drm_mtgpu_job_release_doorbell {
	struct {
		/**
		 * @job_ctx_handle: [IN] handle of job context.
		 */
		__u64 job_ctx_handle;
		/**
		 * @doorbell_handle: [IN] doorbell handle to free.
		 */
		__u32 doorbell_handle;

		__u32 pad;	/* IGNORE ALIGN CHECK */
	} in;
};

#define MTGPU_DMA_TRANSFER_DIR BIT(0)
#define MTGPU_DMA_TRANSFER_DEVICE_TO_HOST	0x0
#define MTGPU_DMA_TRANSFER_HOST_TO_DEVICE	0x1
#define MTGPU_DMA_TRANSFER_LOCAL_TO_PEER	0x4
#define MTGPU_DMA_TRANSFER_PEER_TO_LOCAL	0x5

struct drm_mtgpu_dma_transfer { /* IGNORE STRUCT */
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
		 * @offset: [IN] the bo_handle starting position of the transfer.
		 */
		__u64 offset;

		/**
		 * @ext_offset: [IN] the ext_handle starting position of the transfer.
		 */
		__u64 ext_offset;

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

struct drm_mtgpu_object_destroy { /* IGNORE STRUCT */
	/**
	 * @type: [IN] Type of object to create.
	 *
	 * This must be one of the values defined by &enum drm_mtgpu_object_type.
	 */
	__u32 type;

	/**
	 * @pad: [IN] just for padding
	 */
	__u32 pad;	/* IGNORE ALIGN CHECK */

	/**
	 * @handle: [IN] Handle for freelist to be destroyed.
	 */
	__u64 handle;
};

/**
 * struct drm_mtgpu_hwrt_dataset_create_args - Arguments for
 * %DRM_MTGPU_OBJECT_TYPE_HWRT_DATASET
 */
struct drm_mtgpu_hwrt_dataset_create_args { /* IGNORE STRUCT */
	__u64 pm_data_va_array_mcg;
	__u64 tail_ptr_va_array_mcg;
	__u64 vheap_table_va;
	__u64 ppp_multi_sample_ctl;
	__u64 pm_data_va_array;
	__u64 pm_secure_data_va_array;
	__u64 tail_ptr_va_array;
	__u64 free_list_handles;
	__u32 free_lists_count;
	__u32 mcg_core_num;
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
struct drm_mtgpu_free_list_create_args { /* IGNORE STRUCT */
	/**
	 * @free_list_base_dev_vaddr: [IN] base dev vaddr of free list
	 */
	__u64 free_list_base_dev_vaddr;

	/**
	 * @free_list_state_dev_vaddr: [IN] state dev vaddr of free list
	 */
	__u64 free_list_state_dev_vaddr;

	/**
	 * @mem_ctx_handle: [IN] vm context handle
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

struct drm_mtgpu_render_resource_create_args { /* IGNORE STRUCT */
	/**
	 * @mcg_core_num: [IN] Mcg core num.
	 */
	__u32 mcg_core_num;

	/**
	 * @free_list_count: [IN] How many free lists we are going to create,
	 * 			  in mcg, we can create more free lists, and
	 * 			  use part of them.
	 */
	__u32 free_list_count;

	/**
	 * @free_list_create_args: [IN] array of drm_mtgpu_free_list_create_args.
	 */
	__u64 free_list_create_args;
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
	 * @DRM_MTGPU_OBJECT_TYPE_RENDER_RESOURCE: Render resource object. Use &struct
	 * drm_mtgpu_free_list_create_args for object creation arguments.
	 */
	DRM_MTGPU_OBJECT_TYPE_RENDER_RESOURCE,

	/**
	 * @DRM_MTGPU_OBJECT_TYPE_INVALID: Mark flag for validation.
	 */
	DRM_MTGPU_OBJECT_TYPE_INVALID,
};

/**
 * struct drm_mtgpu_ioctl_create_object_args - Arguments for
 * %DRM_MTGPU_OBJECT_CREATE
 */
struct drm_mtgpu_object_create { /* IGNORE STRUCT */
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
		__u32 pad;	/* IGNORE ALIGN CHECK */

		/** @data: [IN] User pointer to arguments for specific object type . */
		__u64 data;
	} in;

	struct {
		/**
		 * @handle: [OUT] Handle for created object.
		 */
		__u64 *handles;
	} out;
};

/**
 * struct drm_mtgpu_fence_to_fd - Arguments for
 * %DRM_MTGPU_FENCE_TO_FD
 */
struct drm_mtgpu_fence_to_fd { /* IGNORE STRUCT */
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
		__u32 pad;	/* IGNORE ALIGN CHECK */
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

enum drm_mtgpu_semaphore_type {
	MTGPU_SEMAPHORE_TYPE_NORMAL = 0,		/* fw: binary */
	MTGPU_SEMAPHORE_TYPE_NORMAL_WITH_SHADOW,	/* fw: binary + shadow */
	MTGPU_SEMAPHORE_TYPE_TIMELINE,			/* fw: inc */
	MTGPU_SEMAPHORE_TYPE_TIMELINE_WITH_SHADOW,	/* fw: inc + shadow */
	MTGPU_SEMAPHORE_TYPE_BINARY,			/* fw: signal */
	MTGPU_SEMAPHORE_TYPE_BINARY_WITH_SHADOW,	/* fw: signal + shadow */
	MTGPU_SEMAPHORE_TYPE_BINARY_AUTO_CLEAR,		/* fw: signal + set_zero */
	MTGPU_SEMAPHORE_TYPE_INVALID,
};

#define MTGPU_SEMAPHORE_FLAG_ERROR_DIFFUSION	BIT(0)

struct drm_mtgpu_semaphore_create {
	struct {
		/**
		 * @type: [OUT] type of semaphore
		 */
		__u32 type;

		/**
		 * @flag: [IN] Reserved for the future
		 */
		__u32 flag;
	} in;

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
		 * @bo_handle: [OUT] bo handle of semaphore
		 */
		__u64 shadow_bo_handle;

		/**
		 * @value_offset: [OUT] value offset of semaphore
		 */
		__u64 value_offset;

		/**
		 * @gpu_address: [OUT] gpu/firmware virtual address of semaphore
		 */
		__u64 gpu_address;
	} out;
};

struct drm_mtgpu_semaphore_destroy {
	/**
	 * @handle: [IN] handle for this semaphore
	 */
	__u64 handle;
};

enum drm_mtgpu_semaphore_submit_type {
	DRM_MTGPU_SEMAPHORE_GPU_SIGNAL = 0,
	DRM_MTGPU_SEMAPHORE_GPU_WAIT,
};

struct drm_mtgpu_semaphore_submit {
	/**
	 * @ctx_handle: [IN] handle of job context.
	 */
	__u64 ctx_handle;

	/**
	 * @job_type: [IN] job type of this semaphore
	 *
	 * This must be one of the values defined by &enum drm_mtgpu_job_type.
	 */
	__u32 job_type;

	/**
	 * @submit_type: [IN] type of this semaphore.
	 */
	__u32 submit_type;

	/**
	 * @semaphore: [IN] drm mtgpu semaphore.
	 */
	struct drm_mtgpu_semaphore semaphore;
};

struct drm_mtgpu_semaphore_cpu_signal {
	/**
	 * @vm_ctx_handle: [IN] handle of vm context.
	 */
	__u64 vm_ctx_handle;
	/**
	 * @semaphore: [IN] drm mtgpu semaphore.
	 */
	struct drm_mtgpu_semaphore semaphore;
};

struct drm_mtgpu_semaphore_export_fd {
	struct {
		/**
		 * @semaphore: [IN] drm mtgpu semaphore.
		 */
		struct drm_mtgpu_semaphore semaphore;
	} in;

	struct {
		/**
		 * @fd: [IN] fd for drm_mtgpu_semaphore
		 */
		__s32 fd;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_semaphore_import_fd {
	struct {
		/**
		 * @fd: [IN] fd
		 */
		__s32 fd;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */

		/** @vm_ctx_handle: [IN] Handle of VM context. */
		__u64 vm_ctx_handle;
	} in;

	struct {
		/**
		 * @semaphore: [OUT] drm mtgpu semaphore.
		 */
		struct drm_mtgpu_semaphore semaphore;

		/**
		 * @bo_handle: [OUT] bo handle of semaphore
		 */
		__u64 bo_handle;

		/**
		 * @bo_handle: [OUT] bo handle of semaphore
		 */
		__u64 shadow_bo_handle;

		/**
		 * @value_offset: [OUT] value offset of semaphore
		 */
		__u64 value_offset;

		/**
		 * @type: [OUT] type of semaphore
		 */
		__u32 type;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */
	} out;
};

struct drm_mtgpu_semaphore_wait {
	struct {
		/**
		 * @semaphore: [IN] drm_mtgpu_semaphore array.
		 */
		__u64 semaphores;

		/**
		 * @semaphore_count: [IN] semaphore count
		 */
		__u32 count;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */

		/**
		* @timeout_ns: [IN] maximum waiting time
		*/
		__u64 timeout_ns;
	} in;
};

struct drm_mtgpu_semaphore_export_global_handle {
	struct {
		/**
		 * @handle: [IN] handle of semaphore
		 */
		__u64 handle;
	} in;

	struct {
		/**
		 * @handle: [IN] global_handle of semaphore
		 */
		__u64 global_handle;
	} out;
};

struct drm_mtgpu_semaphore_import_global_handle {
	struct {
		/**
		 * @handle: [IN] global_handle of semaphore
		 */
		__u64 global_handle;

		/**
		 * @flag: [IN] The flag of the way to get p2p addr when import semaphore.
		 */
		__u64 flag;
	} in;

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
		 * @bo_handle: [OUT] bo handle of semaphore
		 */
		__u64 shadow_bo_handle;

		/**
		 * @value_offset: [OUT] value offset of semaphore
		 */
		__u64 value_offset;
	} out;
};

struct drm_mtgpu_llc_persistence { /* IGNORE STRUCT */
	struct {
		/**
		 * @replace_mode: replace_mode for RESIDENCY_CTRL register
		 *
		 * Only required when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_SET
		 */
		__u32 replace_mode;

		/**
		 * @pad: [IN] just for padding
		 */
		__u32 pad;	/* IGNORE ALIGN CHECK */

		/**
		 * @llc_size: maximum llc size of the chip
		 * Only output when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_GET
		 */
		__u64 max_set_aside_size;
	} in;

	struct {
		/**
		 * @llc_size: configured llc size
		 *
		 * Only output when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_GET
		 */
		__u32 llc_size;

		/**
		 * @llc_size: maximum llc size of the chip
		 *
		 * Only output when drm_mtgpu_rgx_llc.in.type == DRM_MTGPU_RGX_LLC_TYPE_GET
		 */
		__u32 max_llc_persisting_size;
	} out;
};

struct drm_mtgpu_transport_layer {
	struct {
		/**
		 * @type: [IN] Type of tl event.
		 * This must be one of the values defined by &enum drm_mtgpu_tl_event_type.
		 */
		__u32 type;

		__u32 pad;	/* IGNORE ALIGN CHECK */

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
	__u64 bo_size;
	__u32 multi_readers_allowed;
	__u32 read_offset;
	__u32 tl_stream_size;
	__u32 pad;	/* IGNORE ALIGN CHECK */
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
	MTGPU_HWPERF_GET_TIMESTAMPS,
	MTGPU_HWPERF_FLUSH_BUFFER,
	MTGPU_HWPERF_INVALID,
};

struct drm_mtgpu_hwperf { /* IGNORE STRUCT */
	struct {
		__u32 type;
		__u32 toggle;
		__u32 stream_id;
		__u32 pad;	/* IGNORE ALIGN CHECK */
		__u64 mask;
	} in;

	struct {
		__u64 data;
	} out;
};

struct drm_mtgpu_hwperf_control {
	struct {
		__u32 toggle;
		__u32 stream_id;
		__u64 mask;
	} in;

	struct {
		__u64 data;
	} out;
};

struct mtgpu_hwperf_timestamps { /* IGNORE STRUCT */
	__u64 soc_timestamp;
	__u64 os_timestamp;
};

struct drm_mtgpu_hwperf_get_timestamps {
	struct {
		__u64 soc_timestamp;
		__u64 os_timestamp;
	} out;
};

struct mtgpu_hwperf_flush_buffer { /* IGNORE STRUCT */
	__u32 num_flushed;
};

struct drm_mtgpu_hwperf_flush_buffer {
	struct {
		__u32 num_flushed;
	} out;
};

struct drm_mtgpu_mss_pfm_config {
	/**
	 * @ctx_handle: [IN] handle of job context.
	 */
	__u64 job_ctx_handle;
	/**
	 * @pad: [IN] data size in bytes
	 */
	__u64 size;
	/**
	 * @data: [IN] User pointer to arguments for specific object type .
	 */
	__u64 data;
};

struct drm_mtgpu_get_container_pid {
	struct 
	{
		__u32 host_pid;
	} in;
	
	struct 
	{
		__u32 container_pid;
	} out;
};

struct drm_mtgpu_notify_queue_update { /* IGNORE STRUCT */
	/**
	 * @type: [IN] Type of the job
	 */
	__u32 type;

	/**
	 * @pad: just for padding
	 */
	__u32 pad;	/* IGNORE ALIGN CHECK */
	/**
	 * @ctx_handle: [IN] handle of job context.
	 */
	__u64 ctx_handle;
};

struct drm_mtgpu_codec_wait {
	/**
	* @bo_handle: [IN] bo handle of codec job buffer
	*/
	__u64 bo_handle;

	/**
	* @offset: [IN] offset of bo_handle addr
	*/
	__u32 offset;

	/**
	* @flag: [IN] flag for feature
	*/
	__u32 flag;

	/**
	* @timeout_ns: [IN] maximum waiting time
	*/
	__u64 timeout_ns;
};

#include "mtgpu_aligncheck.h"

struct mtgpu_api_version_info {
	__u64 api_id;
	__u32 version_min;
	__u32 version_max;
};

struct mtgpu_abi_version_info {
	__u64 abi_id;
	__u32 version_min;
	__u32 version_max;
};

struct drm_mtgpu_get_version_list {
	struct {
		__u32 supported_api_count;
		__u32 supported_abi_count;
		__u32 supported_fwif_count;
		__u32 pad;			/* IGNORE ALIGN CHECK */
		__u64 api_version_data;
		__u64 abi_version_data;
		__u64 fwif_version_data;
	} out;
};

struct drm_mtgpu_get_submission_last_error {
	struct {
		/**
		 * @ctx_handle: [IN] handle of job context.
		 */
		__u64 job_ctx_handle;
	} in;

	struct {
		/**
		 * @submission_last_error: [OUT] last error of submission.
		 */
		__u64 submission_last_error;
	} out;
};

struct drm_mtgpu_get_device_last_error {
	struct {
		/**
		 * @device_last_error: [OUT] last error of device.
		 */
		__u64 device_last_error;
	} out;
};

#ifndef __KERNEL__
#if defined(__cplusplus)
}
#endif
#endif	/* __KERNEL__ */

#endif /* __MTGPU_DRM_H__ */

