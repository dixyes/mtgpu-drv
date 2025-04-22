/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_MDEV_H__
#define __MTGPU_MDEV_H__

#include "mtgpu.h"
#include "vgpu_types.h"
#include "mtgpu_vdma.h"
#include "vgpu_shm.h"
#include "vgpu_virtual_display.h"
#include "mtgpu_sriov.h"

typedef enum {
	SRIOV_DISABLED,
	SRIOV_SHARED,
	SRIOV_DEDICATED,
} sriov_mode;

/* driver private data */
struct mtgpu_mdev_drv_data {
	mt_kref kref;
	u32 hwr_backup_mode;
	u32 vgpu_guest_ccb_dump_period;
	bool vgpu_qos;
	bool vgpu_host_trigger_hwr;
	sriov_mode sriov_mode;
	atomic_t pdev_cnt;
	struct workqueue_struct *vgpu_wq;
	struct vgpu_mem_drv_data *vmm_drv_data;
	int vgpu_hotplug_upgrade_vram_mode;
	struct mtgpu_mdev_common_drv_data *mmc_drv_data;
	bool vgpu_sriov_support_mpc_two_config;
	enum mtgpu_eata_mode eata_mode;
};

struct vsync_timer;

#define VFIO_PCI_OFFSET_SHIFT   40
#define VFIO_PCI_OFFSET_TO_INDEX(off)   (off >> VFIO_PCI_OFFSET_SHIFT)
#define VFIO_PCI_INDEX_TO_OFFSET(index) ((u64)(index) << VFIO_PCI_OFFSET_SHIFT)
#define VFIO_PCI_OFFSET_MASK    (((u64)(1) << VFIO_PCI_OFFSET_SHIFT) - 1)

#define MTGPU_SYSMEM_DEV_BASE                     0x8000000000UL

/* Windows guest PC entry invalidate value */
#define MTGPU_VGPU_WINDOWS_PC_ENTRY_INVALIDATE    0x80000000

/* Windows guest PD&PT entry invalidate value */
#define MTGPU_VGPU_WINDOWS_PD_PT_ENTRY_INVALIDATE 0x8000000000UL

#define MTGPU_FW_MMU_RESERVED_SIZE      0x100000
#define MTGPU_FW_HEAP_SIZE              (1 << RGX_FW_HEAP_SHIFT)
#define MTGPU_VGPU_CONFIG_SPACE_SIZE    0x100 // 256 bytes

/* The number of vgpu_state arrays in memory */
#define MTGPU_VGPU_STATE_COUNT         (48)

/* sudi supported 10 per chip */
#define MTGPU_SUDI_MAX_VGPU_COUNT      (10)
/**
 * QY1 supported 14(sriov_enable), 32(sriov_disable);
 * TODO: Temporarily set the value to 14, the value will be
 * changed to 20 when the vpu PR and multi MC PR is ready.
 */
#define MTGPU_QY1_SRIOV_MAX_VGPU_COUNT (14)
#define MTGPU_QY1_SOFT_MAX_VGPU_COUNT  (32)
/*
 * TODO: Temporarily set the value to 8, The final value is confirmed by PRD.
 */
#define MTGPU_QY2_SRIOV_MAX_VGPU_COUNT (8)
#define MTGPU_QY2_SOFT_MAX_VGPU_COUNT  (8)

// SUDI GPU regs in BAR0 2MB~18MB range.
#define MTGPU_SUDI_GPU_REGS_RANGE      (0x1000000)

#define MTGPU_VGPU_MAX_HW_OSID          8

#define MTGPU_VGPU_INVALID_INSTANCE_ID  0xFF

#if defined(SUPPORT_SW_OSID_EXTENSION)
/* TODO:
 * Due to video hardware capabilities and memory size limitations, can only support 7 guest os now
 */
#define MTGPU_PER_MPC_USED_MAX_OSID     PVRSRV_VZ_NUM_OSID
#define MTGPU_SW_OSID_EXT_KICK_DOMAIN_HOST 4
#define MTGPU_SW_OSID_EXT_DOMAIN_BEGIN  1
#define MTGPU_SW_OSID_EXT_DOMAIN_END    3
#define MTGPU_SW_OSID_EXT_DOMAIN_OFFSET 4
#else
#define MTGPU_PER_MPC_USED_MAX_OSID     (8)
#endif

#define MTGPU_VGPU_INVALID_OSID         0
#define MTGPU_VGPU_DEDICATED_REG_SIZE   (64 * 1024)
#define MTGPU_VGPU_CUSTOM_REG_SIZE      (64 * 1024)

#define MTGPU_VGPU_MAX_MEM_CONTEXT      32

#define MTGPU_VGPU_DISPLAY_REGION       16

#define STORE_LE16(addr, val)           (*(u16 *)(addr) = (val))
#define STORE_LE32(addr, val)           (*(u32 *)(addr) = (val))

/* MMU Table */
#define MTGPU_VGPU_MMU_PC_MASK          0xFFFFFFF0
#define MTGPU_VGPU_MMU_PD_MASK          0xFFFFFFF000
#define MTGPU_VGPU_MMU_PT_MASK          0xFFFFFFF000

/* pci cfg space offsets for fb config (dword) */
#define MTGPU_VGPU_VENDORCAP_OFFSET	0x40
#define MTGPU_VGPU_VENDORCAP_SIZE	0x10
#define MTGPU_VGPU_FORMAT_OFFSET	(MTGPU_VGPU_VENDORCAP_OFFSET + 0x04)
#define MTGPU_VGPU_WIDTH_OFFSET 	(MTGPU_VGPU_VENDORCAP_OFFSET + 0x08)
#define MTGPU_VGPU_HEIGHT_OFFSET	(MTGPU_VGPU_VENDORCAP_OFFSET + 0x0c)

#define MTGPU_VGPU_PCI_CAP_VGPU_ID	(0xAA)
#define MTGPU_VGPU_DEVICE_ID_OFFSET	(0x2)
#define MTGPU_VGPU_DEVICE_ID		(0xAAAA)

#define MTGPU_VGPU_INT_ID_BODA955	23
#define MTGPU_VGPU_INT_ID_WAVE517	24
#define MTGPU_VGPU_INT_ID_WAVE627	25
#define MTGPU_VGPU_INT_ID_GPU		26

#define MTGPU_VGPU_NAME      		"mtgpu"
#define MTGPU_VGPU_CLASS_NAME		"mtgpu"

#define MTGPU_VGPU_DDR_GRANULARITY_SIZE 0x1000000UL /* 16M */
#define MTGPU_VGPU_DDR_SIZE_COEFFICIENT 7/8

#define MAX_VM_MEM_SEGMENT 64

#define MTGPU_TIME_CORR_FORMAT_US 29
#define MTGPU_TIME_CORR_FORMAT_MS 25

/**
 * For gen3, vpu heap is an independent heap.
 * Don't need to alloc memory from vpu group but from vpu heap instead.
 */
#define MTGPU_VGPU_VPU_NON_GROUP	  (0xff)

/*
 * For gen1/gen2, Due to hardware limitations of the vpu, the memory
 * allocated to the 4G space to be accessed ranges from 0x204000 to 0xfffef000.
 */
#define MTGPU_VPU_MEM_RESERVE_HEAD        (0x204000ULL)
#define MTGPU_VPU_MEM_RESERVE_TAIL        (0x11000ULL)

#define MTGPU_VPU_SHM_SIZE (0x200000ULL)

/* virtual display num */
#define MTGPU_VGPU_DISPLAY_NUM_MAX      2

#if defined(VDI_PLATFORM_RUIJIE) || \
    defined(VDI_PLATFORM_CTYUN) || \
    defined(VDI_PLATFORM_CSTACK) || \
    defined(VDI_PLATFORM_ARCHEROS)
#define MTGPU_VGPU_DISPLAY_NUM_DEFAULT  1
#else
#define MTGPU_VGPU_DISPLAY_NUM_DEFAULT  0
#endif

/* For gfn_to_pfn workaround.
 * It seems the gfn_to_pfn will fail if it is called from ioctl or other
 * context different from vcpu context, so use the workaround before fixing it
 */
#define MTGPU_VGPU_GFN_TO_PFN_WORKAROUND

/**
 * Allocate 80M local memory for the 4K single-screen Encode.
 * For 1104 or higher vgpu types allocate 160M local memory for the 4K Dual-screen Encode.
 */
#define VGPU_MEMSIZE_2G (0x80000000)
#define VGPU_MEMSIZE_4G (0x100000000)
#define VGPU_4K_ENCODER_RESERVED_SIZE (0x5000000)
#define VGPU_DUAL4K_ENCODER_RESERVED_SIZE (0xA000000)
#define VGPU_8K_ENCODER_RESERVED_SIZE (0x14000000)

enum mtgpu_mdev_interrupt_status_id {
	MTGPU_MDEV_INT_STATUS_GPU,
	MTGPU_MDEV_INT_STATUS_BODA955,
	MTGPU_MDEV_INT_STATUS_WAVE517,
	MTGPU_MDEV_INT_STATUS_WAVE627,
	MTGPU_MDEV_INT_STATUS_LAST
};

typedef enum {
	DEVICE_TYPE_VGPU, /* vgpu general heap */
	DEVICE_TYPE_VVPU, /* vpu-vcpu */
	DEVICE_TYPE_MAX,
} DEVICE_TYPE;

typedef enum {
	MTGPU_VINTR_NONE,
	MTGPU_VINTR_INJECT,
	MTGPU_VINTR_ACK,
} MTGPU_VINTR_STATE;

typedef enum {
	LINUX_GUEST_HWR_HOST_NONE,
	LINUX_GUEST_HWR_HOST_STARTED,
	LINUX_GUEST_HWR_HOST_END
} LINUX_GUEST_HWR_HOST_STAGE;

#define MTGPU_VGPU_MEM_SEG_FLAG_GPU_AVAIL		BIT_ULL(0)
#define MTGPU_VGPU_MEM_SEG_FLAG_VPU_AVAIL		BIT_ULL(1)
#define MTGPU_VGPU_MEM_SEG_FLAG_VPU_VCPU		BIT_ULL(2)
#define MTGPU_VGPU_MEM_SEG_FLAG_VCORE_SIZE		BIT_ULL(3)
#define MTGPU_VGPU_MEM_SEG_FLAG_VPU_ENCODE_SIZE		BIT_ULL(4)
#define MTGPU_VGPU_MEM_SEG_FLAG_VPU_SYS_SHM_OFFSET      BIT_ULL(5)

#define MTGPU_VGPU_MEM_SEG_FLAG_MEM_MASK                                                           \
	(MTGPU_VGPU_MEM_SEG_FLAG_GPU_AVAIL | MTGPU_VGPU_MEM_SEG_FLAG_VPU_AVAIL |                   \
	 MTGPU_VGPU_MEM_SEG_FLAG_VPU_VCPU)

struct mem_segment {
	u64 base;
	u64 size;
	u64 flag;
};

/* The definition of the structure must be consistent with the definition of wddm */
struct vm_segment_info {
	volatile u64 size;
	volatile u32 segment_cnt;
	struct mem_segment mem_segment[MAX_VM_MEM_SEGMENT];
};

struct vgpu_info_ext {
	u64 padding;
	u32 max_resolution_width;  /* virtual display max resolution width */
	u32 max_resolution_height; /* virtual display max resolution height */
	u32 max_encode_num;
	u32 max_decode_num;
	u64 pb_fl_va_base;
	u64 pb_fl_pa_base;
	u32 pb_fl_size;
	u32 mpc_core_nums;
	u32 context_switch_flags;
	u32 reserved_u32_1;
	u64 iova_start;
	u32 max_vdisplay_num;     /* max virtual display num */
	u64 reserved_u64_1;
	u64 reserved_u64_2;
};

#define MTGPU_VGPU_INFO_MAGIC			(0xAA557491U)
#define MTGPU_VGPU_INFO_VERSION1		(1U)
#define MTGPU_VGPU_INFO_VERSION2		(2U)
#define MTGPU_VGPU_INFO_FLAG_ENABLE_VPU		BIT_ULL(0)
#define MTGPU_VGPU_INFO_FLAG_ENABLE_IOMMU_DMA	BIT_ULL(1) /* deprecated */
#define MTGPU_VGPU_INFO_FLAG_ENABLE_IOMMU_IRQ	BIT_ULL(2)
#define MTGPU_VGPU_INFO_FLAG_VPU_BAR4		BIT_ULL(3) /* deprecated */
#define MTGPU_VGPU_INFO_FLAG_PB_SHARED		BIT_ULL(4)
#define MTGPU_VGPU_INFO_FLAG_SHARE_VPU_MEM	BIT_ULL(5) /* deprecated */
#define MTGPU_VGPU_INFO_FLAG_ADDR_TRANS_NOTRAP	BIT_ULL(6)
#define MTGPU_VGPU_INFO_FLAG_VPU_SYS_SHM	BIT_ULL(7)
#define MTGPU_VGPU_INFO_FLAG_VPU_SET_VCORE_SIZE BIT_ULL(8)
#define MTGPU_VGPU_INFO_FLAG_LIMIT_DECODE_1080P BIT_ULL(9)
#define MTGPU_VGPU_INFO_FLAG_SUPPORT_HDR	BIT_ULL(10)

#define MTGPU_VGPU_INFO_GUEST_FLAG_ONLINE	BIT_ULL(0)
#define MTGPU_VGPU_INFO_GUEST_FLAG_WINDOWS	BIT_ULL(1)
#define MTGPU_VGPU_INFO_GUEST_FLAG_LINUX	BIT_ULL(2)

struct vgpu_info {
	volatile u32 magic;
	volatile u32 version;
	volatile u32 osid;
	volatile u64 flag;
	volatile u64 vm_mem_size;
	volatile u64 vm_bar2_actual_mem_size;
	union {
		char padding[0xc20]; /* vgpu_info v1 */
		struct vm_segment_info segment_info[DEVICE_TYPE_MAX]; /* deprecated */
		struct mem_segment ms[128];
	};
	u64 guest_flag; /* RO for host */
	volatile u32 ms_cnt;
	volatile u64 reserved_u64_1;
	volatile u32 reserved_u32_1;

	volatile u64 ext_size;

	/*
	 * WARNING: Do not change anything above here!
	 * New added variables should be stored in 'struct vgpu_info_ext'
	 */
	struct vgpu_info_ext ext_info;
};

/**
 * MPC info indicates the logical core id, physical core id and multi primary core id start with 0.
 * Each primary core can create multi osids, and osid may be same on different primary core. To
 * locate the resources, the vgpu instance has to know which primary core it is on. So it is
 * necessary to touch the primary core info as convenient as possible.
 */
struct mtgpu_mpc_info {
	/* id of the multicore that this core on. Start with 0. */
	u32 mpc_id;
	/* the logical id of the primary core of the multicore that this core on. */
	u32 logical_core_id;
	/* the physical id of the primary core of the multicore that this core on. */
	u32 physical_core_id;
	/* number of cores on the MPC. */
	u32 core_nums;
};


/* vpu guest and host shared memory */
struct vpu_shared_mem {
	u64 video_mem_cpu_phy_addr;
	u64 video_mem_card_phy_addr;
	void *video_mem_va;
	u64 video_mem_size;
};

/* vpu group info, encode/decode resolution and instacne info */
struct mtgpu_mdev_vpu_info {
	u32 video_mem_group_id;
	u64 video_mem_group_base;
	u32 video_mem_vcpu_id;
	u64 video_mem_vcpu_base;

	u32 max_resolution_width;
	u32 max_resolution_height;
	u32 max_encode_num;
	u32 max_decode_num;

	u64 para1; /* indicate if mixed vgpu type */
};

struct mtgpu_mdev_ops {
	int (*device_create)(int osid, struct mtgpu_mdev_vpu_info *info, struct device *dev,
			     void *priv_data);
	int (*device_remove)(int osid, struct device *dev, void *priv_data);

	int (*driver_load)(int osid, struct vpu_shared_mem *shared_mem,
			   struct mtgpu_mdev_vpu_info *info, struct device *dev, void *priv_data);
	int (*driver_unload)(int osid, struct device *dev, void *priv_data);

	/*
	 * Called when accessing the custom registers.
	 * Return value:
	 *     0: the operation is not handled, MDEV will continue to handle this access.
	 *     1: the operation is handled, MDEV will not handle this access anymore.
	 */
	int (*reg_write)(int osid, u32 offset, u64 val, void *priv_data);
	int (*reg_read)(int osid, u32 offset, u64 *val, void *priv_data);
};

struct mtgpu_mdev_vcpu_mem_info {
	u64 start;
	u64 length;
	u64 free;
};

/* If vm uses old version of UEFI, mmio space size may be limited to 32G, 
 * so that vGPU can be upgraded to 16G at most. 
 */
typedef enum {
	VGPU_HOTPLUG_UPGRADE_DISABLED,
	VGPU_HOTPLUG_UPGRADE_TO_16G,
	VGPU_HOTPLUG_UPGRADE_TO_MAX
} VGPU_HOTPLUG_UPGRADE_VRAM_MODE;

struct attribute_group;
struct mdev_device;
struct vm_area_struct;
struct device_attribute;
struct notifier_block;
struct mdev_parent_ops;
struct device;
struct kvm;
struct attribute;
struct mdev_driver;
struct mtgpu_vgpu_state;
struct mtgpu_mdev_device_state;
struct vfio_device;
struct vfio_device_ops;
struct mdev_parent;
struct mdev_type;
struct mtgpu_mdev_type_wrapper;
struct vgpu_mm_state;
struct mtgpu_vgpu_capacity;
struct mdev_state;
struct _WIN_FW_INFO_;
struct vgpu_qos_scheduler;

#if (RGX_NUM_OS_SUPPORTED > 1) && (PVRSRV_APPHINT_DRIVERMODE == 0)
struct workqueue_struct *
mtgpu_dev_state_get_vgpu_wq(struct mtgpu_mdev_device_state *mdev_device_state);
#ifdef ENABLE_VGPU_LIVE_MIGRATION
typedef enum {
	VGPU_MIG_HOST_NONE,
	VGPU_MIG_HOST_COPY_RAM,
	VGPU_MIG_HOST_RAM_OVER_THRESHOLD,
	VGPU_MIG_HOST_FINISHED,
} VGPU_MIG_HOST_STAGE;

struct vgpu_mig_state *mtgpu_vgpu_state_get_mig_state(struct mtgpu_vgpu_state *vgpu_state);
u64 vgpu_mig_get_fw_heap_base(struct mtgpu_vgpu_state *vgpu_state);
void vgpu_mig_get_heap_info(struct mtgpu_vgpu_state *vgpu_state,
			    struct vm_segment_info *vgpu_segment_info, u32 type);
bool vgpu_mig_resume_system_share_mem(struct mtgpu_vgpu_state *vgpu_state, u64* shm_gpa_records);
void vgpu_mig_get_gpa(struct mtgpu_vgpu_state *vgpu_state, u64 *vgpu_info_gpa, u64 *share_mem_gpa);
bool vgpu_mig_query_guest_can_stop(struct mtgpu_vgpu_state *vgpu_state);
int vgpu_notify_guest_mig_start(struct mtgpu_vgpu_state *vgpu_state);
void vgpu_mig_disable_src_guest_dirty_track(struct mtgpu_vgpu_state *vgpu_state,
					    struct vgpu_mig_state *mig_state);
void vgpu_mig_disable_dst_guest_dirty_track(struct mtgpu_vgpu_state *vgpu_state,
					    struct vgpu_mig_state *mig_state);
void vgpu_mig_after_source_finished(struct mtgpu_vgpu_state *vgpu_state,
				    struct vgpu_mig_state *mig_state);
void vgpu_mig_get_vgpu_type(struct mtgpu_vgpu_state *vgpu_state, char *vgpu_type);
u32 vgpu_mig_get_eata_desc_cnt(struct mtgpu_vgpu_state *vgpu_state, int type);
u32 vgpu_mig_get_eata_table_size(struct mtgpu_vgpu_state *vgpu_state);
void vgpu_mig_get_gdpa_to_dpa_table(struct mtgpu_vgpu_state *vgpu_state, u8 *gdpa_to_dpa_table);
#endif /* ENABLE_VGPU_LIVE_MIGRATION */

int mtgpu_vgpu_alloc_vcpu_mem(struct device *dev, u32 id, size_t size, dma_addr_t *dev_addr,
			      void **handle);
void mtgpu_vgpu_free_vcpu_mem(void *handle);
int mtgpu_vgpu_get_vcpu_mem_info(struct device *dev, struct mtgpu_mdev_vcpu_mem_info *vmi);

bool mtgpu_vgpu_get_qos_enable(void);
bool mtgpu_vgpu_state_is_open(struct mtgpu_mdev_device_state *mdev_dev_state, int i);
int vgpu_qos_get_sysmem_size(struct mtgpu_mdev_device_state *mdev_dev_state, u64 *sysmemsz, int i);
void mtgpu_vgpu_send_qos_info(struct mtgpu_mdev_device_state *mdev_device_state,
			    struct vgpu_qos_scheduler *qos_sched, int i);
bool mtgpu_vgpu_qos_enable(struct mtgpu_mdev_device_state *mdev_dev_state,
		      struct vgpu_qos_scheduler *qos_sched);
bool mtgpu_vgpu_1g_support_4k(void);

bool mtgpu_vgpu_get_time_corr_enable(void);
bool mtgpu_check_time_corr_enable(struct mtgpu_mdev_device_state *mdev_dev_state);
u64 vgpu_get_fw_time(struct mtgpu_mdev_device_state *mdev_dev_state);
u64 vgpu_calibrate_cr_to_os(void *devInfo, u64 fw_ts);
void vgpu_convert_ts_to_utc(void *devInfo, u64 fw_ts, char *ts);

int mtgpu_vgpu_vsync_adjust(struct device *dev, u8 *instances, u32 count);
void mtgpu_vgpu_vsync_simulate(struct virtual_display *vdisp);

int mtgpu_vz_device_init(struct mtgpu_device *mtgpu);
void mtgpu_vz_device_exit(struct mtgpu_device *mtgpu);
int mtgpu_vz_driver_init(void);
void mtgpu_vz_driver_exit(void);
void *mtgpu_mdev_drv_data_get(void);
void mtgpu_mdev_drv_data_put(void);

int mtgpu_vz_mpc_init(struct mtgpu_device *mtdev);
void mtgpu_vz_mpc_deinit(struct mtgpu_device *mtdev);

void mtgpu_vgpu_set_vpu_status(struct device *dev, u32 status);
u64 mtgpu_vgpu_get_host_available_mem_size(struct device *dev);
int mtgpu_vgpu_osid_count_per_core(struct mtgpu_device *mtdev, int vf_per_gpu_core, int mpc_cnt);
u64 mtgpu_vgpu_get_mpc_mem_card_base(struct mtgpu_device *mtdev, u32 mpc_id);
u64 mtgpu_vgpu_get_mpc_mem_size(void);
u64 mtgpu_mdev_vram_size_in_segment(struct device *dev, u32 instance_id, u64 dpa, u64 size);
int mtgpu_mdev_guest_offset_to_dpa(struct device *dev, u32 instance_id, u64 gdpa, u64 *dpa);
void *mtgpu_mdev_gpa_area_vmap(struct device *dev,  u32 instance_id, u64 gpa, u64 size);
void mtgpu_mdev_gpa_area_vunmap(void *vaddr, u64 size);
int mtgpu_mdev_gpa_to_hva(struct device *dev, u32 instance_id, u64 gpa, u64 *hva);
int mtgpu_mdev_gpa_to_hpa(struct device *dev, u32 instance_id, u64 gpa, u64 *hpa);
int mtgpu_mdev_dpa_to_gpa(struct device *dev, u32 instance_id, u64 dpa, u64 *gpa);
u64 mtgpu_mdev_gdpa_to_dpa(struct mtgpu_vgpu_state *vgpu_state, u64 gdpa);
u64 mtgpu_mdev_iova_to_hpa(struct device *dev, u32 instance_id, u64 iova);
int mtgpu_mdev_trigger_interrupt(struct device *dev, u32 instance_id, int int_id);
void mtgpu_mdev_register_callback(struct mtgpu_mdev_ops ops, struct device *dev, void *priv_data);
void mtgpu_mdev_unregister_callback(struct mtgpu_mdev_ops ops, struct device *dev, void *priv_data);
int mtgpu_vgpu_get_scheduling_policy(void);
int mtgpu_vgpu_get_time_sliced_value(void);
unsigned long mtgpu_vgpu_get_host_mem_size(void);
void mtgpu_vgpu_set_host_mem_size(unsigned long size);
bool mtgpu_vgpu_should_force_mmio_in_4g(void);
bool mtgpu_vgpu_is_win_fw_mode(void);
bool mtgpu_vgpu_is_dyn_mpc_mode(void);
u32 mtgpu_vgpu_get_win_fw_csw_value(void);
int mtgpu_get_vgpu_hotplug_upgrade_vram_mode(void);
u64 mtgpu_get_max_vgpu_ddr_size(struct mtgpu_device *mtdev);
const struct mtgpu_vgpu_capacity *get_vgpu_capacity_by_name_or_id(struct device *dev,
								  const char *name,
								  unsigned int id);
u32 mtgpu_mdev_query_available_vgpu_count(const struct mtgpu_vgpu_capacity *capacity,
					  struct device *dev);
u32 mtgpu_mdev_query_max_vgpu_count(const struct mtgpu_vgpu_capacity *capacity, struct device *dev);
int mtgpu_mdev_set_type_groups(struct device *dev, struct attribute_group ***groups);
void mtgpu_mdev_capacity_deinit(struct mtgpu_mdev_device_state *mdev_dev_state);
int mtgpu_mdev_lock_init(struct mtgpu_mdev_device_state *mdev_device_state);
int mtgpu_set_max_vgpu_count(struct mtgpu_device *mtgpu);
void mtgpu_mdev_lock_deinit(struct mtgpu_mdev_device_state *mdev_device_state);
int mtgpu_mdev_mpc_data_init(struct mtgpu_device *mtgpu);
void mtgpu_mdev_mpc_data_deinit(struct mtgpu_device *mtgpu);
int mtgpu_vgpu_vfio_group_notifier(struct notifier_block *nb, unsigned long action, void *data);

ssize_t vgpu_rw(struct mdev_device *mdev, const char *write_buf, char *read_buf, size_t count,
		loff_t *ppos, bool is_write);
void *mtgpu_vgpu_get_mdev_types(struct mtgpu_mdev_device_state *mdev_device_state);
void *mtgpu_vgpu_get_type_wrappers(struct mtgpu_mdev_device_state *mdev_device_state);
void mtgpu_vgpu_set_mdev_parent(struct mtgpu_mdev_device_state *mdev_device_state,
				 struct mdev_parent *parent);
void *mtgpu_vgpu_get_mdev_parent(struct mtgpu_mdev_device_state *mdev_device_state);
u32 mtgpu_vgpu_get_num_types(struct mtgpu_mdev_device_state *mdev_device_state);

int vgpu_create(const struct mtgpu_vgpu_capacity *capacity, struct mdev_device *mdev);
void vgpu_remove(struct mdev_device *mdev);
int vgpu_open(struct mdev_device *mdev);
void vgpu_release(struct mdev_device *mdev);
int vgpu_mmap(struct mdev_device *mdev, struct vm_area_struct *vma);

long vgpu_vfio_device_get_info(struct mdev_device *mdev, unsigned long arg);
long vgpu_vfio_device_get_region_info(struct mdev_device *mdev, unsigned long arg);
long vgpu_vfio_device_get_irq_info(struct mdev_device *mdev, unsigned long arg);
long vgpu_vfio_device_set_irqs(struct mdev_device *mdev, unsigned long arg);
long vgpu_vfio_device_reset(struct mdev_device *mdev, unsigned long arg);
long vgpu_vfio_device_query_gfx_plane(struct mdev_device *mdev, unsigned long arg);
long vgpu_vfio_device_get_gfx_dmabuf(struct mdev_device *mdev, unsigned long arg);
void mtgpu_mdev_set_linux_fw_info(void *linux_fw_info, void *dev, u32 mpc_id);
bool mtgpu_vgpu_get_watchdog_token(void *dev, u32 instance_id, u32 *os_token, u32 *fw_token);
bool mtgpu_vgpu_get_connection_state(void *dev, u32 instance_id, u32 *os_state, u32 *fw_state);
/* windows fw */
void mtgpu_mdev_set_win_fw_info(void *win_fw_info, void *dev, u32 mpc_id);
void mtgpu_mdev_set_fw_info(void *fw_info, void *dev, u32 mpc_id);
bool mtgpu_get_intr_info(void *dev, u32 instance_id,
			 u32 *osid, u64 *intr_enqueue, u64 *intr_inject,
			 u64 *intr_eoi, u64 *intr_vm_handled);
void vgpu_get_cmd_exec_info(void *dev, u32 mpc_id, u32 osid,
			struct vgpu_cmd_exec_info *cmd_info);
u32 mtgpu_vgpu_get_osid_count(struct device *dev);
u32 mtgpu_vgpu_get_osid_start(struct device *dev);
void mtgpu_mdev_vgpu_int_cb(u32 int_id, bool is_osid0, void *priv_data, u32 mpc_id);
void vgpu_hwr_request(void *vgpu_state);
u64 mtgpu_get_hwr_info(void *dev, u32 mpc_id);
bool mtgpu_vgpu_is_pb_shared_enabled(struct device *dev);
int vgpu_get_open_state(struct mdev_device *mdev);
void vgpu_set_open_state(struct mdev_device *mdev, int open);

void vgpu_notify_linux_guest_do_hwr(struct device *dev, u32 mpc_id, bool start);
/* extension osid for windows fw */
#if defined(SUPPORT_SW_OSID_EXTENSION)
void mtgpu_mdev_set_ext_osid_info_mem(void *ext_osid_info,
				      void *priv_data, u32 mpc_id);
void mtgpu_mdev_set_master_kick_reg(void *master_kick_reg, void *priv_data, u32 mpc_id);
void mtgpu_mdev_vgpu_kick(int osid, u32 kick_value, void *priv_data);
#endif

/* attributes */
ssize_t vgpu_id_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t vgpu_device_info_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t vgpu_device_state_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t display_num_max_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t display_connect_num_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t display_connect_num_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count);
ssize_t mtext_debug_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t mtext_debug_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count);
ssize_t vdma_enable_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t vdma_enable_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count);
ssize_t vdma_perf_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t vdma_perf_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count);
ssize_t guest_running_commit_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf);
ssize_t fw_running_versioin_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf);
void mtgpu_mdev_get_vgpu_ids(struct mtgpu_vgpu_state *vgpu_state,
			     u32 *instance_id, u32 *mpc_id, u64 *osid);
struct device *mtgpu_mdev_parent_dev(struct mtgpu_vgpu_state *vgpu_state);
int vgpu_ipc_host_get_gpu_util(struct mtgpu_device *mtdev, u32 id, u64 *util);
int vgpu_ipc_host_get_gpu_mem_info(struct mtgpu_device *mtdev, u32 id, u64 *used, u64 *total);
void mtgpu_mdev_init_mpc_heap(struct mtgpu_device *mtdev, struct mtgpu_platform_data *pdata,
			      const u32 mpc_id);
int mtgpu_vgpu_get_opened_num(struct mtgpu_device *mtdev);
void *mtgpu_mdev_get_vgpu_monitor(struct mtgpu_device *mtdev);
bool vgpu_queue_work(struct work_struct *work);
bool vgpu_queue_delayed_work(struct delayed_work *dwork, unsigned long delay);
bool mtgpu_card_is_support_vgpu(struct mtgpu_device *mtdev);
void vgpu_update_upgrade_conf(void);
void mtgpu_vgpu_vf_interrupt_handler(void *data);
bool vgpu_qy1_sriov_mpc_two_config(void);
bool vgpu_set_force_gpu_hwr(struct mtgpu_device *mtdev, u32 mpc_id);
void mtgpu_vgpu_upgrade_fw(struct device *dev);
#else

#ifndef MT_UNREFERENCED_PARAMETER
#define MT_UNREFERENCED_PARAMETER(param) ((void)(param))
#endif
static inline void mtgpu_mdev_set_ext_osid_info_mem(void *ext_osid_info,
						    void *priv_data, u32 mpc_id)
{
}

static inline void mtgpu_mdev_set_master_kick_reg(void *master_kick_reg,
						  void *priv_data, u32 mpc_id)
{
}

static inline void mtgpu_mdev_vgpu_kick(int osid, u32 kick_value, void *priv_data)
{
}

static inline int mtgpu_vgpu_vsync_adjust(struct device *dev, u8 *instances, u32 count)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instances);
	MT_UNREFERENCED_PARAMETER(count);

	return 0;
}

static inline void mtgpu_vgpu_vsync_simulate(struct virtual_display *vdisp)
{
	MT_UNREFERENCED_PARAMETER(vdisp);
}

static inline int mtgpu_vz_device_init(struct mtgpu_device *mtgpu)
{
	MT_UNREFERENCED_PARAMETER(mtgpu);
	return 0;
}

static inline void mtgpu_vz_device_exit(struct mtgpu_device *mtgpu)
{
	MT_UNREFERENCED_PARAMETER(mtgpu);
}

static inline int mtgpu_vz_driver_init(void)
{
	return 0;
}

static inline void mtgpu_vz_driver_exit(void)
{
}

static inline void *mtgpu_mdev_drv_data_get(void)
{
	return NULL;
}

static inline void mtgpu_mdev_drv_data_put(void)
{
}

static inline int mtgpu_vz_mpc_init(struct mtgpu_device *mtdev)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	return 0;
}

static inline void mtgpu_vz_mpc_deinit(struct mtgpu_device *mtdev)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
}

static inline int mtgpu_vgpu_alloc_vcpu_mem(struct device *dev, u32 id, size_t size,
					    dma_addr_t *dev_addr, void **handle)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(id);
	MT_UNREFERENCED_PARAMETER(size);
	MT_UNREFERENCED_PARAMETER(dev_addr);
	MT_UNREFERENCED_PARAMETER(handle);

	return 0;
}

static inline void mtgpu_vgpu_free_vcpu_mem(void *handle)
{
	MT_UNREFERENCED_PARAMETER(handle);
}

static inline int mtgpu_vgpu_get_vcpu_mem_info(struct device *dev,
					       struct mtgpu_mdev_vcpu_mem_info *vmi)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(vmi);

	return 0;
}

static inline int mtgpu_mdev_lock_init(struct mtgpu_mdev_device_state *mdev_device_state)
{
	MT_UNREFERENCED_PARAMETER(mdev_device_state);
	return 0;
}

static inline int mtgpu_set_max_vgpu_count(struct mtgpu_device *mtgpu)
{
	MT_UNREFERENCED_PARAMETER(mtgpu);
	return 0;
}

static inline void mtgpu_mdev_lock_deinit(struct mtgpu_mdev_device_state *mdev_device_state)
{
	MT_UNREFERENCED_PARAMETER(mdev_device_state);
}

static inline int mtgpu_mdev_mpc_data_init(struct mtgpu_device *mtgpu)
{
	MT_UNREFERENCED_PARAMETER(mtgpu);
	return 0;
}

static inline void mtgpu_mdev_mpc_data_deinit(struct mtgpu_device *mtgpu)
{
	MT_UNREFERENCED_PARAMETER(mtgpu);
}

static inline void mtgpu_vgpu_set_vpu_status(struct device *dev, u32 status)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(status);
}

static inline u64 mtgpu_vgpu_get_host_available_mem_size(struct device *dev)
{
	MT_UNREFERENCED_PARAMETER(dev);
	return 0;
}

static inline bool mtgpu_vgpu_enabled_vpu_multi_mem_group(struct device *dev)
{
	MT_UNREFERENCED_PARAMETER(dev);
	return false;
}

static inline int mtgpu_vgpu_osid_count_per_core(struct mtgpu_device *mtdev,
						 int vf_per_gpu_core,
						 int mpc_cnt)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	MT_UNREFERENCED_PARAMETER(vf_per_gpu_core);
	MT_UNREFERENCED_PARAMETER(mpc_cnt);
	return 0;
}

static inline u64 mtgpu_vgpu_get_mpc_mem_card_base(struct mtgpu_device *mtdev, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
	return 0;
}

static inline u64 mtgpu_vgpu_get_mpc_mem_size(void)
{
	return 0;
}

static inline u64 mtgpu_mdev_vram_size_in_segment(struct device *dev, u32 instance_id, u64 dpa,
						  u64 size)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(dpa);
	MT_UNREFERENCED_PARAMETER(size);
	return 0;
}

static inline int mtgpu_mdev_guest_offset_to_dpa(struct device *dev,
						 u32 instance_id, u64 gdpa, u64 *dpa)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(gdpa);
	MT_UNREFERENCED_PARAMETER(dpa);
	return -1;
}

static inline void *mtgpu_mdev_gpa_area_vmap(struct device *dev, u32 instance_id, u64 gpa, u64 size)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(gpa);
	MT_UNREFERENCED_PARAMETER(size);
	return NULL;
}

static inline void mtgpu_mdev_gpa_area_vunmap(void *vaddr, u64 size)
{
	MT_UNREFERENCED_PARAMETER(vaddr);
	MT_UNREFERENCED_PARAMETER(size);
}

static inline int mtgpu_mdev_gpa_to_hva(struct device *dev, u32 instance_id, u64 gpa, u64 *hva)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(gpa);
	MT_UNREFERENCED_PARAMETER(hva);
	return -1;
}

static inline int mtgpu_mdev_gpa_to_hpa(struct device *dev, u32 instance_id, u64 gpa, u64 *hpa)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(gpa);
	MT_UNREFERENCED_PARAMETER(hpa);
	return -1;
}

static inline int mtgpu_mdev_dpa_to_gpa(struct device *dev, u32 instance_id, u64 dpa, u64 *gpa)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(dpa);
	MT_UNREFERENCED_PARAMETER(gpa);
	return -1;
}

static inline u64 mtgpu_mdev_gdpa_to_dpa(struct mtgpu_vgpu_state *vgpu_state, u64 gdpa)
{
	MT_UNREFERENCED_PARAMETER(vgpu_state);
	MT_UNREFERENCED_PARAMETER(gdpa);
	return -1;
}

static inline u64 mtgpu_mdev_iova_to_hpa(struct device *dev, u32 instance_id, u64 iova)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(iova);
	return 0;
}

static inline int mtgpu_mdev_trigger_interrupt(struct device *dev, u32 instance_id, int int_id)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(int_id);
	return -1;
}

static inline void mtgpu_mdev_register_callback(struct mtgpu_mdev_ops ops, struct device *dev,
						void *priv_data)
{
	MT_UNREFERENCED_PARAMETER(ops);
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(priv_data);
}

static inline void mtgpu_mdev_unregister_callback(struct mtgpu_mdev_ops ops, struct device *dev,
						  void *priv_data)
{
	MT_UNREFERENCED_PARAMETER(ops);
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(priv_data);
}

static inline int mtgpu_vgpu_get_scheduling_policy(void)
{
	return -1;
}

static inline int mtgpu_vgpu_get_time_sliced_value(void)
{
	return -1;
}

static inline bool mtgpu_vgpu_is_pb_shared_enabled(struct device *dev)
{
	return false;
}

static inline unsigned long mtgpu_vgpu_get_host_mem_size(void)
{
	return 0;
}

static inline void mtgpu_vgpu_set_host_mem_size(unsigned long size)
{
	MT_UNREFERENCED_PARAMETER(size);
}

static inline bool mtgpu_vgpu_should_force_mmio_in_4g(void)
{
	return false;
}

static inline u32 mtgpu_vgpu_get_osid_start(struct device *dev)
{
	MT_UNREFERENCED_PARAMETER(dev);
	return 0;
}

static inline u32 mtgpu_vgpu_get_osid_count(struct device *dev)
{
	MT_UNREFERENCED_PARAMETER(dev);
	return 0;
}

static inline void mtgpu_mdev_vgpu_int_cb(u32 int_id, bool is_osid0, void *priv_data, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(int_id);
	MT_UNREFERENCED_PARAMETER(is_osid0);
	MT_UNREFERENCED_PARAMETER(priv_data);
	MT_UNREFERENCED_PARAMETER(mpc_id);
}

static inline bool mtgpu_vgpu_is_win_fw_mode(void)
{
	return false;
}

static inline u32 mtgpu_vgpu_get_win_fw_csw_value(void)
{
	return 0;
}

static inline int mtgpu_get_vgpu_hotplug_upgrade_vram_mode(void)
{
	return 0;
}

static inline u64 mtgpu_get_max_vgpu_ddr_size(struct mtgpu_device *mtdev)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	return 0;
}

static inline bool mtgpu_vgpu_get_qos_enable(void)
{
       return false;
}

static inline void mtgpu_mdev_set_linux_fw_info(void *linux_fw_info, void *dev, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(linux_fw_info);
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
}

static inline void mtgpu_mdev_set_win_fw_info(void *win_fw_info, void *dev, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(win_fw_info);
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
}

static inline void mtgpu_mdev_set_fw_info(void *fw_info, void *dev, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(fw_info);
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
}

static inline bool mtgpu_vgpu_get_watchdog_token(void *dev, u32 instance_id,
						 u32 *os_token, u32 *fw_token)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(os_token);
	MT_UNREFERENCED_PARAMETER(fw_token);
	return false;
}

static inline bool mtgpu_vgpu_get_connection_state(void *dev, u32 instance_id,
						   u32 *os_state, u32 *fw_state)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(os_state);
	MT_UNREFERENCED_PARAMETER(fw_state);
	return false;
}

static inline bool mtgpu_vgpu_get_time_corr_enable(void)
{
	return false;
}

static inline bool mtgpu_check_time_corr_enable(struct mtgpu_mdev_device_state *mdev_dev_state)
{
	MT_UNREFERENCED_PARAMETER(mdev_dev_state);
	return false;
}

static inline u64 vgpu_get_cr_time(struct mtgpu_mdev_device_state *mdev_dev_state)
{
	MT_UNREFERENCED_PARAMETER(mdev_dev_state);
	return 0;
}

static inline u64 vgpu_calibrate_cr_to_os(void *devInfo, u64 fw_ts)
{
	MT_UNREFERENCED_PARAMETER(devInfo);
	MT_UNREFERENCED_PARAMETER(fw_ts);
	return 0;
}

static inline void vgpu_convert_ts_to_utc(void *devInfo, u64 fw_ts, char* ts)
{
	MT_UNREFERENCED_PARAMETER(devInfo);
	MT_UNREFERENCED_PARAMETER(fw_ts);
	MT_UNREFERENCED_PARAMETER(ts);
	return;
}

static inline bool mtgpu_get_intr_info(void *dev, u32 instance_id,
				       u32 *osid, u64 *intr_enqueue, u64 *intr_inject,
				       u64 *intr_eoi, u64 *intr_vm_handled)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(osid);
	MT_UNREFERENCED_PARAMETER(intr_enqueue);
	MT_UNREFERENCED_PARAMETER(intr_inject);
	MT_UNREFERENCED_PARAMETER(intr_eoi);
	MT_UNREFERENCED_PARAMETER(intr_vm_handled);
	return false;
}

static inline void vgpu_get_cmd_exec_info(void *dev, u32 mpc_id, u32 osid,
		       struct vgpu_cmd_exec_info *cmd_info)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
	MT_UNREFERENCED_PARAMETER(osid);
	MT_UNREFERENCED_PARAMETER(cmd_info);
}

static inline u64 mtgpu_get_hwr_info(void *dev, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
	return 0;
}

static inline void mtgpu_mdev_get_vgpu_ids(struct mtgpu_vgpu_state *vgpu_state,
					   u32 *instance_id, u32 *mpc_id, u64 *osid)
{
	MT_UNREFERENCED_PARAMETER(vgpu_state);
	MT_UNREFERENCED_PARAMETER(instance_id);
	MT_UNREFERENCED_PARAMETER(mpc_id);
	MT_UNREFERENCED_PARAMETER(osid);
}

static inline struct device *mtgpu_mdev_parent_dev(struct mtgpu_vgpu_state *vgpu_state)
{
	MT_UNREFERENCED_PARAMETER(vgpu_state);
	return NULL;
}

static inline int vgpu_ipc_host_get_gpu_util(struct mtgpu_device *mtdev, u32 id, u64 *util)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	MT_UNREFERENCED_PARAMETER(id);
	MT_UNREFERENCED_PARAMETER(util);

	return  -OS_VAL(ENOTSUPP);
}

static inline int vgpu_ipc_host_get_gpu_mem_info(struct mtgpu_device *mtdev, u32 id,
						 u64 *used, u64 *total)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	MT_UNREFERENCED_PARAMETER(id);
	MT_UNREFERENCED_PARAMETER(used);
	MT_UNREFERENCED_PARAMETER(total);

	return  -OS_VAL(ENOTSUPP);
}

static inline void mtgpu_mdev_init_mpc_heap(struct mtgpu_device *mtdev,
					    struct mtgpu_platform_data *pdata, const u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	MT_UNREFERENCED_PARAMETER(pdata);
	MT_UNREFERENCED_PARAMETER(mpc_id);
}

static inline void vgpu_notify_linux_guest_do_hwr(struct device *dev, u32 mpc_id, bool start)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(mpc_id);
	MT_UNREFERENCED_PARAMETER(start);
}

static inline bool vgpu_queue_work(struct work_struct *work)
{
	MT_UNREFERENCED_PARAMETER(work);
	return false;
}
static inline bool vgpu_queue_delayed_work(struct delayed_work *dwork, unsigned long delay)
{
	MT_UNREFERENCED_PARAMETER(dwork);
	MT_UNREFERENCED_PARAMETER(delay);
	return false;
}

static inline void vgpu_update_upgrade_conf(void)
{

}

static inline bool vgpu_qy1_sriov_mpc_two_config(void)
{
	return false;
}

static inline bool vgpu_set_force_gpu_hwr(struct mtgpu_device *mtdev, u32 mpc_id)
{
	MT_UNREFERENCED_PARAMETER(mtdev);
	MT_UNREFERENCED_PARAMETER(mpc_id);

	return false;
}

static inline void mtgpu_vgpu_upgrade_fw(struct device *dev)
{
	MT_UNREFERENCED_PARAMETER(dev);
}

#endif

static inline u64 roundup_power2(const u64 x)
{
	u64 y = x;

	y--;
	y |= y >> 1;
	y |= y >> 2;
	y |= y >> 4;
	y |= y >> 8;
	y |= y >> 16;
	y |= y >> 32;
	y++;
	return y;
}

#endif /* __MTGPU_MDEV_H__ */
