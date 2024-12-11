#ifndef __MTGPU_EVENT_REPORT_H
#define __MTGPU_EVENT_REPORT_H

#include "os-interface.h"
#include "mtgpu_module_param.h"

#define EVENT_MSG_COUNT				50
#define EVENT_MSG_BUFFER_SIZE			120

#define ERROR_REPORT_EVENT_TYPE			1

#define ERROR_REPORT_BUFFER_SIZE		128
#define ERROR_REPORT_BUFFER_SHFIT		7
#define ERROR_REPORT_EXTERN_BUFFER_SIZE		64

#define HW_ERROR_REPORT_BASE			1
#define SW_ERROR_REPORT_BASE			20001

#define HW_ERROR_REPORT_PCIE_BASE		HW_ERROR_REPORT_BASE
#define HW_ERROR_REPORT_MTLINK_BASE		(HW_ERROR_REPORT_BASE + 1000)
#define HW_ERROR_REPORT_META_BASE		(HW_ERROR_REPORT_BASE + 2000)
#define HW_ERROR_REPORT_DM_BASE			(HW_ERROR_REPORT_BASE + 3000)
#define HW_ERROR_REPORT_DDR_BASE		(HW_ERROR_REPORT_BASE + 4000)
#define HW_ERROR_REPORT_DMA_BASE		(HW_ERROR_REPORT_BASE + 5000)
#define HW_ERROR_REPORT_CE_BASE			(HW_ERROR_REPORT_BASE + 6000)
#define HW_ERROR_REPORT_SMC_BASE		(HW_ERROR_REPORT_BASE + 7000)
#define HW_ERROR_REPORT_FEC_BASE		(HW_ERROR_REPORT_BASE + 8000)
#define HW_ERROR_REPORT_GPU_BASE		(HW_ERROR_REPORT_BASE + 9000)
#define HW_ERROR_REPORT_VPU_BASE		(HW_ERROR_REPORT_BASE + 10000)
#define HW_ERROR_REPORT_DISPLAY_BASE		(HW_ERROR_REPORT_BASE + 11000)
#define HW_ERROR_REPORT_RESERVED_BASE		(HW_ERROR_REPORT_BASE + 12000)

#define SW_ERROR_REPORT_MMU_BASE		SW_ERROR_REPORT_BASE
#define SW_ERROR_REPORT_RESOURCE_BASE		(SW_ERROR_REPORT_BASE + 1000)
#define SW_ERROR_REPORT_HANDLE_BASE		(SW_ERROR_REPORT_BASE + 2000)
#define SW_ERROR_REPORT_MEM_BASE		(SW_ERROR_REPORT_BASE + 3000)
#define SW_ERROR_REPORT_BUILD_OPTION_BASE	(SW_ERROR_REPORT_BASE + 4000)
#define SW_ERROR_REPORT_GP_CMD_BASE		(SW_ERROR_REPORT_BASE + 5000)
#define SW_ERROR_REPORT_KCCB_BASE		(SW_ERROR_REPORT_BASE + 6000)
#define SW_ERROR_REPORT_CLEANUP_THREAD_BASE	(SW_ERROR_REPORT_BASE + 7000)

struct wait_queue_head;
struct mtgpu_device;
struct mutex;
struct mtgpu_event_report;
struct file;
struct list_head;

enum error_report_index {
	ERROR_REPORT_NO_ERROR,
	ERROR_REPORT_BAR0_ACCESS_FAILED,
	ERROR_REPORT_BAR2_ACCESS_FAILED,
	ERROR_REPORT_BAR2_UNSTABLE,
	ERROR_REPORT_MTLINK_UP,
	ERROR_REPORT_MTLINK_DOWN,
	ERROR_REPORT_MTLINK_RECOVERY_FAILED,
	ERROR_REPORT_NO_ACTIVE_CONNECTION,
	ERROR_REPORT_DEVICE_STATUS_NOT_OK,
	ERROR_REPORT_DEVICE_ERROR,
	ERROR_REPORT_BOOT_FW_ERROR,
	ERROR_REPORT_GUILTY_LOCKUP,
	ERROR_REPORT_GUILTY_OVERRUN,
	ERROR_REPORT_DDR_MEMFAULT,
	ERROR_REPORT_MMU_FAULT,
	ERROR_REPORT_FREE_ERROR,
	ERROR_REPORT_LOOK_UP_HANDLE_ERROR,
	ERROR_REPORT_RA_REQUEST_ALLOC_FAIL,
	ERROR_REPORT_KMD_UMD_MISMATCH,
	ERROR_REPORT_GP_CMD_TIMEOUT,
	ERROR_REPORT_KCCB_TIMEOUT,
	ERROR_REPORT_CLEANUP_ITEMS_DROPPED,
};

enum error_report_tag {
	MTGPU_ERROR_REPORT_NO_ERROR,

	MTGPU_ERROR_REPORT_BAR0_ACCESS_FAILED = HW_ERROR_REPORT_PCIE_BASE,
	MTGPU_ERROR_REPORT_BAR2_ACCESS_FAILED,
	MTGPU_ERROR_REPORT_BAR2_UNSTABLE,

	MTGPU_ERROR_REPORT_MTLINK_UP = HW_ERROR_REPORT_MTLINK_BASE,
	MTGPU_ERROR_REPORT_MTLINK_DOWN,
	MTGPU_ERROR_REPORT_MTLINK_RECOVERY_FAILED,

	MTGPU_ERROR_REPORT_NO_ACTIVE_CONNECTION = HW_ERROR_REPORT_META_BASE,
	MTGPU_ERROR_REPORT_DEVICE_STATUS_NOT_OK,
	MTGPU_ERROR_REPORT_DEVICE_ERROR,
	MTGPU_ERROR_REPORT_BOOT_FW_ERROR,

	MTGPU_ERROR_REPORT_GUILTY_LOCKUP = HW_ERROR_REPORT_DM_BASE,
	MTGPU_ERROR_REPORT_GUILTY_OVERRUN,

	MTGPU_ERROR_REPORT_DDR_MEMFAULT = HW_ERROR_REPORT_DDR_BASE,

	MTGPU_ERROR_REPORT_MMU_FAULT = SW_ERROR_REPORT_MMU_BASE,

	MTGPU_ERROR_REPORT_FREE_ERROR = SW_ERROR_REPORT_RESOURCE_BASE,

	MTGPU_ERROR_REPORT_LOOK_UP_HANDLE_ERROR = SW_ERROR_REPORT_HANDLE_BASE,

	MTGPU_ERROR_REPORT_RA_REQUEST_ALLOC_FAIL = SW_ERROR_REPORT_MEM_BASE,

	MTGPU_ERROR_REPORT_KMD_UMD_MISMATCH = SW_ERROR_REPORT_BUILD_OPTION_BASE,

	MTGPU_ERROR_REPORT_GP_CMD_TIMEOUT = SW_ERROR_REPORT_GP_CMD_BASE,

	MTGPU_ERROR_REPORT_KCCB_TIMEOUT = SW_ERROR_REPORT_KCCB_BASE,

	MTGPU_ERROR_REPORT_CLEANUP_ITEMS_DROPPED = SW_ERROR_REPORT_CLEANUP_THREAD_BASE,
};

enum mtgpu_module_id {
	MTGPU_MODULE_NULL,
	MTGPU_MODULE_PCIE,
	MTGPU_MODULE_MTLINK,
	MTGPU_MODULE_META,
	MTGPU_MODULE_DM,
	MTGPU_MODULE_DDR,
	MTGPU_MODULE_DMA,
	MTGPU_MODULE_CE,
	MTGPU_MODULE_SMC,
	MTGPU_MODULE_FEC,
	MTGPU_MODULE_GPU,
	MTGPU_MODULE_VPU,
	MTGPU_MODULE_DISPLAY,

	MTGPU_MODULE_MMU,
	MTGPU_MODULE_RESOURCE,
	MTGPU_MODULE_HANDLE,
	MTGPU_MODULE_MEM,
	MTGPU_MODULE_BUILD_OPTION,
	MTGPU_MODULE_GP_CMD,
	MTGPU_MODULE_KCCB,
	MTGPU_MODULE_CLEANUP_THREAD,
};

struct mtgpu_file_node {
	struct file *file;
	struct list_head node;
};

/* Records the relationship between the specified module_id, error_report_tag,
 * and the specified module name
 */
struct mtgpu_event_module {
	u32 error_report_tag;
	char *module_type;
	u32 module_id;
};

/* TLV - Type Length Value */
struct mtgpu_event {
	u32 type;
	u32 length;
	char msg[EVENT_MSG_BUFFER_SIZE];
};

struct mtgpu_error_report_info {
	/* Version of the error report structure */
	u32 vserion;
	/* Unique identifier representing the error (error_report_tag) */
	u32 error_id;
	/* Domain, Bus, Device, Function (SBDF) information */
	u64 sbdf;
	/* Timestamp when the error occurred */
	u64 timestamp;
	/* Identifier for the module associated with the error */
	u32 module_id;
	/* Reserved space for future use */
	u64 resv[2];
	/* External buffer for additional error report information */
	char external[ERROR_REPORT_EXTERN_BUFFER_SIZE];
};

typedef void (*event_notify)(struct mtgpu_event_report *event_report, int type);

struct mtgpu_event_report {
	struct mtgpu_device *mtdev;
	/* the wait_queue_head for the vps_poll */
	struct wait_queue_head *event_msg_wait_head;
	/* all types of the event msg */
	struct mtgpu_event *event_msgs;
	/* the index represents the next index of mtgpu_event to be written */
	atomic_t index;
	struct mtgpu_event_report *next;
	event_notify event_notify;
	/* Chain link to access all files for this device event_report */
	struct list_head file_list;
	struct mutex *file_lock;
};

/**************************************************************************/ /*!
@Function       find_event_report
@Description    Retrieve the mtgpu_event_report associated with the specified PCI device.
@Input          dev     pci device
@Return         the specified mtgpu_event_report
 */ /**************************************************************************/
struct mtgpu_event_report *find_event_report(struct device *dev);

/**************************************************************************/ /*!
@Function       mtgpu_error_record
@Description    Logs the specified error message to the buffer of the associated device
@Input          dev     pci device
@Input          error_report_index   related error_report_index
@Input		msg	The message format string
@Input		size	The message buffer size
@Return         Error status
 */ /**************************************************************************/
int mtgpu_error_record(struct device *dev, const int error_report_index,
		       const char *msg, int size);

/**************************************************************************/ /*!
@Function       mtgpu_report_msg
@Description    fill the event messages with a report of the sepcified type
		and wake up the process of sleep.
@Input          event_report	mtgpu_event_report of specified PCI device
@Input          msg	report buffer with the specified type
@Input          size	buffer size
@Input          type	the specified type
@Return         None
 */ /**************************************************************************/
void mtgpu_report_msg(struct mtgpu_event_report *event_report, void *buffer, u32 size, u32 type);

/**************************************************************************/ /*!
@Function       mtgpu_all_dev_report_msg
@Description    fill the event messages with a report of the sepcified type
		and wake up the process of sleep.
@Input          msg	report buffer with the specified type
@Input          size	buffer size
@Input          type	the specified type
@Return         None
 */ /**************************************************************************/
void mtgpu_all_dev_report_msg(void *msg, u32 size, u32 type);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_init
@Description    Initialize the mtgpu_event_report of the associated device.
@Input		mtdev	mtgpu device
@Return         Error status
 */ /**************************************************************************/
int mtgpu_event_report_init(struct mtgpu_device *mtdev);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_deinit
@Description    Unload the mtgpu_event_report of the associated device.
@Input		mtdev	mtgpu device
@Return         None
 */ /**************************************************************************/
void mtgpu_event_report_deinit(struct mtgpu_device *mtdev);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_global_init
@Description    Initialize mtgpu report global information.
@Return         Error status
 */ /**************************************************************************/
int mtgpu_event_report_global_init(void);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_global_deinit
@Description    Unload mtgpu report global information.
@Return         None
 */ /**************************************************************************/
void mtgpu_event_report_global_deinit(void);

#ifdef NO_HARDWARE
#define MTGPU_ERROR_REPORT(dev, error_report_index, fmt, ...)
#else
#define MTGPU_ERROR_REPORT(dev, error_report_index, fmt, ...)					\
do {												\
	char buffer[ERROR_REPORT_EXTERN_BUFFER_SIZE];						\
	int size;										\
	if (enable_event_report) {								\
		size = os_snprintf(buffer, ERROR_REPORT_EXTERN_BUFFER_SIZE, fmt, ##__VA_ARGS__);\
		mtgpu_error_record(dev, error_report_index, buffer, size);			\
	}											\
} while (0)
#endif

#endif
