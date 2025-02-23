/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_MISC_H__
#define __MTGPU_MISC_H__

#include "uapi_common.h"

#define MTGPU_BUILD_VER_MSG_MAX_DATA_SIZE	(256)

struct device_info {
	u32 ipc_version;
	u16 vendor_id, device_id;
	u64 reserved;
};

struct ipc_msg_interface {
	u64 header;
	u8 data[256]; /* PCIE_IPC_MSG_MAX_DATA_SIZE */
};

struct build_version {
	char info[MTGPU_BUILD_VER_MSG_MAX_DATA_SIZE];
};

#define MTGPU_MISC_GET_BUILD_VERSION		_IOR('M', 0x9, struct build_version)

#define MTGPU_MISC_GET_NUMA_NODE		_IOR('M', 0xA, int)

/* WARNING: MKIS Managed! */
/* !!! CAUTION !!! This value MUST match the definition in MTML */
#define MTGPU_MISC_MEM_ALLOC_MAX        (0x400000)  /* max alloc size 4MB */
struct memory_address {
	phys_addr_t mem_phys_addr;
	size_t mem_size;
};

/* only used for sync_io */
#define MTGPU_MISC_IPC_MESSAGE_TRANSMIT		_IOWR('M', 0x1, struct ipc_msg_interface)
/* only used for async_io */
#define MTGPU_MISC_IPC_IOCTR_SUB		_IOW('M', 0x6, unsigned char)
#define MTGPU_MISC_IPC_IOCTR_UNSUB		_IOW('M', 0x7, unsigned char)

/*
 * MKIS Managed IOCTL Number definitions begin
 *
 * !!! BE CAUTIONED !!!
 * Below IOCTL Numbers are managed by MKIS protocol. Any changing to these
 * contents may lead to compatibility problem against libmtml, so please
 * make sure you have fully understood the purpose of MKIS and how it
 * works before doing any code change. If you indeed have necessity to
 * make changes to these interfaces but don't know how to be compliant with
 * MKIS requirement, please contact zheng.cao@mthreads.com for help.
 * Also, please make sure zheng.cao@mthreads.com is included in your PR
 * reviewer list.
 */
#define MTGPU_MISC_MEMORY_ALLOC			_IOWR('M', 0x3, struct memory_address)
#define MTGPU_MISC_MEMORY_FREE			_IOWR('M', 0x4, struct memory_address)
#define MTGPU_MISC_GET_DEVICE_INFO		_IOR('M', 0x8, struct device_info)

/* mtlink cmd */
#define MISC_HANDLE_MTLINK                0xC0

#define MISC_HANDLE_MTLINK_CMD_START      0
#define MISC_HANDLE_MTLINK_SPEC           (MISC_HANDLE_MTLINK_CMD_START + 0)
#define MISC_HANDLE_MTLINK_CAPABILITY     (MISC_HANDLE_MTLINK_CMD_START + 1)
#define MISC_HANDLE_MTLINK_STATE          (MISC_HANDLE_MTLINK_CMD_START + 2)
#define MISC_HANDLE_MTLINK_COUNT          (MISC_HANDLE_MTLINK_CMD_START + 3)
#define MISC_HANDLE_MTLINK_RESET_COUNT    (MISC_HANDLE_MTLINK_CMD_START + 4)
#define MISC_HANDLE_MTLINK_LAYOUT         (MISC_HANDLE_MTLINK_CMD_START + 5)
#define MISC_HANDLE_MTLINK_REMOTE_DEV     (MISC_HANDLE_MTLINK_CMD_START + 6)
#define MISC_HANDLE_MTLINK_SHORTEST_PATHS (MISC_HANDLE_MTLINK_CMD_START + 7)
#define MISC_HANDLE_MTLINK_MONITOR_START  (MISC_HANDLE_MTLINK_CMD_START + 8)
#define MISC_HANDLE_MTLINK_MONITOR_READ   (MISC_HANDLE_MTLINK_CMD_START + 9)
#define MISC_HANDLE_MTLINK_MONITOR_STOP   (MISC_HANDLE_MTLINK_CMD_START + 10)
#define MISC_HANDLE_MTLINK_SERDES_STATUS  (MISC_HANDLE_MTLINK_CMD_START + 11)
#define MISC_HANDLE_MTLINK_CMD_CNT        (MISC_HANDLE_MTLINK_SERDES_STATUS + 1)

#define MTGPU_MISC_MTLINK_SPEC_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_SPEC), struct mtlink_spec)
#define MTGPU_MISC_MTLINK_CAPABILITY_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_CAPABILITY), \
	     struct mtlink_capability_query)
#define MTGPU_MISC_MTLINK_STATE_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_STATE), struct mtlink_enable_state)
#define MTGPU_MISC_MTLINK_COUNT_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_COUNT), struct mtlink_counter)
#define MTGPU_MISC_MTLINK_COUNT_RESET \
	_IOW('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_RESET_COUNT), u32)
#define MTGPU_MISC_MTLINK_LAYOUT_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_LAYOUT), \
	     struct mtlink_layout)
#define MTGPU_MISC_MTLINK_REMOTE_DEVICE_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_REMOTE_DEV), \
	     struct mtlink_remote_device_info)
#define MTGPU_MISC_MTLINK_SHORTEST_PATHS_GET \
	_IOR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_SHORTEST_PATHS), \
	     struct mtlink_shortest_paths)
#define MTGPU_MISC_MTLINK_MONITOR_START \
	_IOWR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_MONITOR_START), \
	      struct mtlink_monitor_start_info)
#define MTGPU_MISC_MTLINK_MONITOR_READ \
	_IOWR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_MONITOR_READ), \
	      struct mtlink_monitor_read_info)
#define MTGPU_MISC_MTLINK_MONITOR_STOP \
	_IOWR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_MONITOR_STOP), \
	      struct mtlink_monitor_stop_info)
#define MTGPU_MISC_MTLINK_SERDES_STATUS_GET \
	_IOWR('M', (MISC_HANDLE_MTLINK | MISC_HANDLE_MTLINK_SERDES_STATUS), \
	     struct mtlink_serdes_status_info)

/*
 * MKIS Managed IOCTL Number definitions end
 */

bool mtgpu_misc_is_parent_dev(struct mtgpu_misc_info *misc_info);
bool mtgpu_misc_has_dev_node(struct mtgpu_misc_info *misc_info);
int mtgpu_find_misc_dev(int misc_id, struct mtgpu_misc_info **miscinfo);
int mtgpu_register_misc_parent_device(struct mtgpu_device *mtdev);
int mtgpu_register_misc_instance_devices(struct mtgpu_device *mtdev);
void mtgpu_unregister_misc_instance_devices(struct mtgpu_device *mtdev);
void mtgpu_unregister_misc_parent_device(struct mtgpu_device *mtdev);
int mtgpu_misc_init(void);
void mtgpu_misc_deinit(void);

int mtgpu_reset_event_msgs(struct mtgpu_misc_info *misc_info);
int mtgpu_reset_gpu(struct mtgpu_misc_info *misc_info, PROCESS_DRIVER_STATS_OS_TYPE_INFO_TOTAL *msg);
int mtgpu_clear_musa_status(struct mtgpu_misc_info *misc_info, u64 bitmask);

#endif /* __MTGPU_MISC_H__ */
