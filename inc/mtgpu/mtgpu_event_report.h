#ifndef __MTGPU_EVENT_REPORT_H
#define __MTGPU_EVENT_REPORT_H

#include "os-interface.h"
#include "mtgpu_module_param.h"
#include "mtgpu_drv.h"

#define EVENT_MSG_COUNT				(50)
/*
 * EVNET REPORT1.0 buffer size 120
 * EVNET REPORT2.0 buffer size 248
 */
#define EVENT_MSG_BUFFER_SIZE			(248)
#define EVENT_REPORT_GEN1			(0)
#define EVENT_REPORT_GEN2			(1)
#define XID_ADDTIONAL_BUFFER_SIZE		(128)
#define UUID_STR_LENGTH				(64)

#define EVENT_REPORT_TYPE_XID			(0)
#define EVENT_REPORT_TYPE_MAX			(1)

#define ERROR_REPORT_INFO_GEN1			(0)
#define XID_INFO_GEN2				(1)

struct wait_queue_head;
struct mtgpu_device;
struct mutex;
struct mtgpu_event_report;
struct file;
struct list_head;
struct _PVRSRV_DEVICE_NODE_;
struct mempool_s;

typedef enum {
	XID_MODULE_DRIVER = 0x0,
	XID_MODULE_GPU,
	XID_MODULE_MSS,
	XID_MODULE_VPU,
	XID_MODULE_DISP,
	XID_MODULE_MTBIOS,
	XID_MODULE_PCIE,
	XID_MODULE_MTLINK,
	XID_MODULE_NOC,
	XID_MODULE_AUDIO,
	XID_MODULE_DMA,
	XID_MODULE_MAX,
} mtgpu_xid_module;

typedef enum {
	DRIVER_XID_NO_ERROR			= 0x0,
	DRIVER_XID_GPU_INIT_FAILED,
	DRIVER_XID_MAX,
} driver_xid_tag;

typedef enum {
	GPU_XID_NO_ERROR			= 0x0,
	GPU_XID_OVERRUN,
	GPU_XID_LOCKUP,
	GPU_XID_PAGE_FAULT,
	GPU_XID_FW_IPC_TIMEOUT,
	GPU_XID_MP_EXCEPTION,
	GPU_XID_HW_FAILED,
	GPU_XID_MAX,
} gpu_xid_tag;

typedef enum {
	MSS_XID_NO_ERROR			= 0x0,
	MSS_XID_SBE_ECC_ERROR,
	MSS_XID_UNCONTAINED_ECC_ERROR,
	MSS_XID_CONTAINED_ECC_ERROR,
	MSS_XID_ECC_COUNTER_OVERFLOW,
	MSS_XID_NEW_PAGE_RETIRED,
	MSS_XID_PAGE_RETIRED_FAILED,
	MSS_XID_MAX,
} mss_xid_tag;

typedef enum {
	MTBIOS_XID_NO_ERROR			= 0x0,
	MTBIOS_XID_GPU_FAN_ABNORMAL,
	MTBIOS_XID_GPU_BIOS_TRAP,
	MTBIOS_XID_GPU_TEMP_TOO_HIGH,
	MTBIOS_XID_GPU_SHUTDOWN,
	MTBIOS_XID_MAX,
} mtbios_xid_tag;

typedef enum {
	PCIE_XID_NO_ERROR			= 0x0,
	PCIE_XID_GPU_FALLEN_OF_BUS,
	PCIE_XID_MAX,
} pcie_xid_tag;

typedef enum {
	MTLINK_XID_NO_ERROR			= 0x0,
	MTLINK_XID_LINK_DOWN,
	MTLINK_XID_LINK_UP,
	MTLINK_XID_RECOVERY_FAILED,
	MTLINK_XID_MAX,
} mtlink_xid_tag;

typedef enum {
	DMA_XID_NO_ERROR			= 0x0,
	DMA_XID_DMA_ERROR,
	DMA_XID_MAX,
} dma_xid_tag;

#define IMPACT_SEVERITY_INVAL			(~0x0)
typedef enum {
	IMPACT_SEVERITY_NOTI			= 0x0,
	IMPACT_SEVERITY_WARN			= 0x1,
	IMPACT_SEVERITY_FATAL			= 0x2,
	IMPACT_SEVERITY_MAX,
} mtgpu_xid_impact_severity;

#define IMPACT_SCOPE_INVAL			(~0x0)
typedef enum {
	IMPACT_SCOPE_PROCESS			= 0x0,
	IMPACT_SCOPE_GPU			= 0x1,
	IMPACT_SCOPE_HOST			= 0x2,
	IMPACT_SCOPE_SYSTEM			= 0x3,
	IMPACT_SCOPE_MAX,
} mtgpu_xid_impact_scope;

struct impact_severity_info {
	mtgpu_xid_impact_severity impact_sev;
	char *name;
};

struct impact_scope_info {
	mtgpu_xid_impact_scope impact_sev;
	char *name;
};

struct mtgpu_xid_list {
	u32 xid_id;
	char *xid_name;
	mtgpu_xid_impact_severity impact_sev;
	mtgpu_xid_impact_scope impact_scp;
};

struct mtgpu_xid_module_list {
	mtgpu_xid_module xid_module;
	struct mtgpu_xid_list *xid_list;
	u32 xid_list_size;
};

struct mtgpu_file_node {
	struct file *file;
	struct list_head node;
};

struct mtgpu_xid_file_header {
	char tag[4];            /* tag[0] = 'X', tag[1] = 'I', tag[2] = 'D' */
	u8 rsvd : 7;            /* Must be 0 */
	u8 order : 1;           /* Byte order(endianness): 0 - little endian, 1 - big endianness */
	u16 file_vserion;       /* The version of this header */
	u16 xid_version;        /* The version of xid record stored in this file, must be equal to the version in each record */
	u32 rsvd2;              /* Must be 0 */
	u8 uuid[DEV_UUID_LEN];            /* uuid */
};

/*
 * TLV - Type Length Value
 * XID1.0 mtgpu_event size 128
 * XID2.0 mtgpu_event size 256
 */
struct mtgpu_event {
	u32 type;	/*
			 * type[24:32]:event version(uint8_t)
			 * type[0:23]: event type(bit_map)
			 *  type[0]:TypeNone
			 *  type[1]:TypeXID
			 */
	u32 length;
	char msg[EVENT_MSG_BUFFER_SIZE];
};

struct mtgpu_xid_info {
	u16 version;	/* Version of the error report structure */
	u16 rsvd;
	u32 xid_id;	/*
			 * xid_id [24:32]: xid_module
			 * xid_id [0:23]: Unique ID within the module scope
			 */
	u32 scope : 3;	 /*
			  * Impact scope
			  *  0:Process
			  *  1:GPU
			  *  2:Host
			  *  3:System
			  */
	u32 severity : 3;/*
			  * Impact severity
			  *  0:Notify
			  *  1:Warning
			  *  2:Fatal
			  */
	u32 rsvd2 : 26;
	u64 timestamp;	/* Timestamp when the error occurred */
	u64 sbdf;	/* Domain, Bus, Device, Function (SBDF) information */
	u64 pid;	/* Process ID */
	/* External buffer for additional error report information */
	char addtional[XID_ADDTIONAL_BUFFER_SIZE];
};

struct mtgpu_xid_record {
	u8 uuid[DEV_UUID_LEN];    /* uuid */
	struct mtgpu_xid_info xid_info;
};

typedef void (*event_notify)(struct mtgpu_event_report *event_report, int type);

struct mtgpu_event_info {
	struct mtgpu_event_report *event_report;
	struct work_struct *work;
	int msg_size;
	int event_type;
	union mtgpu_event_record {
		struct mtgpu_xid_record xid_record;
	} event_record;
};

struct mtgpu_event_report {
	struct mtgpu_device	*mtdev;
	struct wait_queue_head	*event_msg_wait_head;	/* the wait_queue_head for the vps_poll */
	struct workqueue_struct *event_msg_workqueue;	/*  */
	struct mtgpu_event	*event_msgs;		/* all types of the event msg */
	atomic_t		index;			/* the index represents
							 * the next index of mtgpu_event to be written
							 */
	struct mtgpu_event_report *next;		/* Chain link to access all files for this device event_report */
	event_notify		event_notify;
	struct list_head	file_list;		/* Link all file that read the event_report */
	struct mutex		*file_lock;
	struct mempool_s	*event_info_pool;;
	char			*uuid_str;
	char			uuid[DEV_UUID_LEN];
	struct file		*record_file[EVENT_REPORT_TYPE_MAX];
};

/**************************************************************************/ /*!
@Function       find_event_report
@Description    Retrieve the mtgpu_event_report associated with the specified PCI device.
@Input          dev     pci device
@Return         the specified mtgpu_event_report
 */ /**************************************************************************/
struct mtgpu_event_report *find_event_report(struct device *dev);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_xid_record
@Description    Logs the specified error message to the buffer of the associated device
@Input          dev     pci device
@Input          module_id   xid module id
@Input          xid_id   internal xid_id
@Input		msg	The message format string
@Input		size	The message buffer size
@Return         Error status
 */ /**************************************************************************/
int mtgpu_event_report_xid_record(struct device *dev,
				  const mtgpu_xid_module module_id,
				  const u32 xid_id,
				  const char *msg, int size);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_msg
@Description    fill the event messages with a report of the sepcified type
		and wake up the process of sleep.
@Input          event_report	mtgpu_event_report of specified PCI device
@Input          msg	report buffer with the specified type
@Input          size	buffer size
@Input          version	event type version
@Input          type	the specified type
@Return         None
 */ /**************************************************************************/
void mtgpu_event_report_msg(struct mtgpu_event_report *event_report,
			    void *buffer, u32 size,
			    u32 version, u32 type);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_msg_all_dev
@Description    fill the event messages with a report of the sepcified type
		and wake up the process of sleep.
@Input          msg	report buffer with the specified type
@Input          size	buffer size
@Input          version	event type version
@Input          type	the specified type
@Return         None
 */ /**************************************************************************/
void mtgpu_event_report_msg_all_dev(void *msg, u32 size, u32 version, u32 type);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_persist_init
@Description    init fd about persist.
@Input          dev_node
@Return         Error status
 */ /**************************************************************************/
int mtgpu_event_report_persist_init(struct _PVRSRV_DEVICE_NODE_ *dev_node);

/**************************************************************************/ /*!
@Function       mtgpu_event_report_persist_deinit
@Description    deinit fd about persist.
@Input          dev_node
 */ /**************************************************************************/
void mtgpu_event_report_persist_deinit(struct _PVRSRV_DEVICE_NODE_ *dev_node);

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
#define MTGPU_XID_IMPL(dev, module_id, xid_id, fmt, ...)
#else
#define MTGPU_XID_IMPL(dev, module_id, xid_id, fmt, ...)						\
do {													\
	char buffer[XID_ADDTIONAL_BUFFER_SIZE];								\
	int size;											\
	if (enable_event_report) {									\
		if (fmt) {										\
			size = os_snprintf(buffer, XID_ADDTIONAL_BUFFER_SIZE, fmt, ##__VA_ARGS__);	\
		} else {										\
			size = os_snprintf(buffer, XID_ADDTIONAL_BUFFER_SIZE, "\n");			\
		}											\
		mtgpu_event_report_xid_record(dev, module_id, xid_id, buffer, size);			\
	}												\
} while (0)
#endif

#define MTGPU_XID_REPORT(dev, module_id, xid_id, ...)						\
	MTGPU_XID_IMPL(dev, module_id, xid_id, __VA_ARGS__)

#endif
