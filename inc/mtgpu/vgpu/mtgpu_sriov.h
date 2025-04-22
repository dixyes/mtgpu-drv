/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_SRIOV_H__
#define __MTGPU_SRIOV_H__

#include "mtgpu.h"

/* Globally defines how many VFs a supported chip contains */
#define MTGPU_SRIOV_VF_MAX_NUM    (64)

/* The initial value of the mtgpu_sriov_vf->bar2_merged_by variable */
#define VF_BAR2_MERGE_DEFAULT     (0xFFFF)
#define VF_BAR0_MERGE_DEFAULT     VF_BAR2_MERGE_DEFAULT

/* The max number of eATA descriptors of a VM. */
#define EATA_DESCRIPTOR_NUM_MAX	  (64)
/* The invalid value of eATA table. */
#define EATA_TABLE_INVAL	  (0xFF)

enum mtgpu_sriov_gpu_mode {
	MTGPU_SRIOV_GPU_MODE_DISABLE = 1,
	MTGPU_SRIOV_GPU_MODE_PRIMARY = 2,
	MTGPU_SRIOV_GPU_MODE_SECONDARY = 3,
};

enum mtgpu_sriov_vf_status {
	/* VF connected GPU core is in Secondary mode or damaged */
	MTGPU_SRIOV_VF_STATE_DISABLE = 1,
	/* VF connected GPU core is in Primary or Primary Standalone mode */
	MTGPU_SRIOV_VF_STATE_ENABLE = 2,
	/* VF BAR0 resource already assigned to vm */
	MTGPU_SRIOV_VF_STATE_IN_USE = 3,
};

enum mtgpu_sriov_vf_irq_id {
	MTGPU_SRIOV_VF_IRQ_GPU = 0,
	MTGPU_SRIOV_VF_IRQ_DMA_WRITE,
	MTGPU_SRIOV_VF_IRQ_DMA_READ,
	MTGPU_SRIOV_VF_IRQ_WAVE517,
	MTGPU_SRIOV_VF_IRQ_WAVE627,
	MTGPU_SRIOV_VF_IRQ_SOFT,
	MTGPU_SRIOV_VF_IRQ_COUNT
};

enum mtgpu_eata_desc_vf_regions {
	/* there are 8 GPU eATAs but each VF will just use one of them. */
	MTGPU_GPU_EATA_DS = 0,
	/* there are 2 VPU eATAs and each VF may use both of them. */
	MTGPU_VPU_EATA0_DS,
	MTGPU_VPU_EATA1_DS,
	MTGPU_DMA_EATA_DS,
	MTGPU_CE_EATA_DS,
	MTGPU_EATA_DS_REGIONS_MAX_COUNT,
};

enum mtgpu_eata_mode {
	BYPASS,
	CHECK,
	TRANSFER,
};

struct mtgpu_sriov_irq_desc {
	void (*handler_function)(void *data);
	void *handler_data;
};

/* describe the vf */
struct mtgpu_sriov_vf {
	/*
	 * The secondary core in each group of MCs will be scheduled by the
	 * primary core, and the vram transaction in the secondary core will
	 * also have the same restrictions as the primary core, so it is
	 * defined as merge here.
	 */
	bool                       gpu_is_merged;

	/*
	 * This vf bar2 resource is merged to other vf; if VF is reserved for
	 * host or VPU used，please let bar2_is_merged:=true, bar2_merged_by:=0xffff.
	 * not merge : 0(false)
	 * merged    : 1(true)
	 */
	bool                       bar2_is_merged;

	/*
	 * Which primary GPU core (logical_core_id)is connected to this vf.
	 * pgpu_id : 0, 1, 2, 3, 4, 5, 6, 7
	 */
	u8                         pgpu_id;

	/*
	 * Connected GPU core osid x.
	 *   Quyuan1: 4,5,6,7
	 *   Quyuan2: 2,3,4,5,6,7
	 */
	u8                         mc_osid;

	/*
	 * The VF of the primary core in the same MC will deliver the workload
	 * to the VF of the secondary core
	 *   ...
	 *   vf3  :  3
	 *   ...
	 *   vf15 : 15
	 *   ...
	 */
	u16                        gpu_merged_by;

	/*
	 * Which VF uses the BAR2 resource of this VF.
	 *   default: 0xffff(host or vpu use)
	 *   ...
	 *   vf3  :  3
	 *   ...
	 *   vf15 : 15
	 *   ...
	 */
	u16                        bar2_merged_by;

	/*
	 * Record the ID of the current VF.
	 *   vf0 : 0
	 *   vf1 : 1
	 *   ...
	 *   vf31(qy1) : 31
	 *   ...
	 *   vf47(qy2) : 47
	 */
	u16                        vf_id;

	/* Sturct pci_dev for this vf device. */
	struct pci_dev             *pci_dev;
	struct mtgpu_sriov         *sriov;

	/*
	 * VF status:
	 *   VF_DISABLE : 0x0
	 *   VF_ENABLE  : 0x1
	 *   VF_INUSE   : 0x2
	 */
	enum mtgpu_sriov_vf_status vf_status;

	/*
	 * The state of the GPU this VF is connected to.
	 *   disable: 1
	 *   enable & primary & primary standalone : 2
	 *   enable & secondary : 3
	 */
	enum mtgpu_sriov_gpu_mode  gpu_core_status;

	/*
	 * EATA is deeply bound to SRIOV when it is designed, and the size of
	 * VF BAR2 is the same as that of EATA isolation granularity. Here
	 * record the EATA description tables corresponding to the current VF.
	 */
	struct mtgpu_io_region     vf_eata_desc_regions[MTGPU_EATA_DS_REGIONS_MAX_COUNT];

	/*
	 * Some systems' 32-bit MMIO addresses do not support too many VFs,
	 * so the QY1 VF BAR0 space is adjusted to 0KB, and GPU registers that
	 * require 64KB are looked up from the address of the PF socif.
	 */
	u32                        socif_offset;
	unsigned long              socif_size;

	/* The count of irq vectors actually used */
	u16                          irq_count;

	/* The base of irq vectors from system */
	u16                          irq_start;

	/* The spinlock for set vf interrupt handler */
	spinlock_t                   *irq_set_lock;

	/* The description of vf interrupt request */
	struct mtgpu_sriov_irq_desc  irq_desc[MTGPU_SRIOV_VF_IRQ_COUNT];
};

/* Record the VF BAR0 resources and VF BAR2 resources used by the created vGPU */
struct mtgpu_sriov_alloc_vf_res {
	/* Master VF that provide resources such as BAR0 and interrupts. */
	struct mtgpu_sriov_vf *master;

	/* Which vgpu use these vf resource. */
	u16                   vgpu_id;

	/* How many vf BAR2 used by master vf. */
	u16                   vf_bar2_num;

	/* Total VF BAR2 size used by vGPU. */
	u64                   vram_size;

	/* VF BAR2 size. */
	u64                   vf_bar2_size;

	/* Record the VF that provides the BAR2 resource to the vGPU. */
	struct mtgpu_sriov_vf *slave[MTGPU_SRIOV_VF_MAX_NUM];
};

struct mtgpu_vf_int_data {
	struct mtgpu_device        *mtdev;
	u16                        vf_id;
	void                       *data;
};

/* sriov main structure */
struct mtgpu_sriov {
	struct pci_dev             *pf_pci_dev;
	struct mtgpu_device        *mtdev;
	struct mtgpu_sriov_vf      vf_devices[MTGPU_SRIOV_VF_MAX_NUM];
	u16                        enabled_vfs;
	u16                        total_vfs;

	/* How many VFs are supported per GPU core. */
	u16                        vf_per_gpu_core;

	/*
	 * VF BAR2 size.
	 *   512MB :  512 * 1024 * 1024
	 *   1024MB: 1024 * 1024 * 1024
	 */
	u64                        vf_bar2_size;

	u64                        eata_granularity;

	/* Record usage of vgpu_id, especially resource allocation for vpu. */
	u8                         vgpu_id[MTGPU_SRIOV_VF_MAX_NUM + 1];

	/*
	 * Reserve a range of addresses starting with vram for the host
	 * vf_0 ~ vf_vf_host_reserve.
	 */
	u8                         vf_host_reserve;

	/*
	 * GPU device id:
	 *   Quyuan1 : 0x0200
	 *   Quyuan2 : 0x0300
	 */
	u16                        device_id;

	/*
	 * Physical GPU core status, gpu_core_status[0] represent physical
	 * GPU core id 0.
	 *   disable: 0x1
	 *   enable & (primary | primary standalone) : 0x2
	 *   enable & secondary : 0x3
	 */
	u8                         *gpu_core_status;

	/* How many cores are supported in a GPU chip. */
	u16                        chip_max_core;

	/* sriov access resource mutex lock */
	struct mutex               *sriov_access_lock;

	/* vf msi register region */
	struct mtgpu_io_region     vf_irq_region;

	/* spinlock for vf interrupt request */
	spinlock_t                 *irq_lock;

	/* The minimum number of vf interrupts expected */
	u8                         irq_min_vector;

	/* The maximum number of vf interrupts expected */
	u8                         irq_max_vector;

	struct mtgpu_vf_int_data   *vf_int_data;
};

#if (RGX_NUM_OS_SUPPORTED > 1)
s32 mtgpu_sriov_init(struct mtgpu_device *mtdev);

s32 mtgpu_sriov_exit(struct mtgpu_device *mtdev);

/* called by mdev vgpu create */
s32 mtgpu_sriov_vgpu_res_alloc(struct mtgpu_sriov *sriov,
			       u64 vram_size,
			       u32 physical_core_id,
			       u32 osid,
			       u32 vgpu_id,
			       u32 vgpu_segment_count,
			       void *vgpu_mem_segment,
			       struct mtgpu_sriov_alloc_vf_res **vf_res);
/* called by mdev vgpu remove */
void mtgpu_sriov_vgpu_res_release(struct mtgpu_sriov *sriov,
				  struct mtgpu_sriov_alloc_vf_res *vf_res);

u64 mtgpu_sriov_min_vram_res_get(struct mtgpu_sriov *sriov, u64 vram_size);

int mtgpu_sriov_register_vf_irq_handler(struct mtgpu_sriov_vf *vfdev,
					u16 irq_id,
					void (*handler)(void *),
					void *data);

int mtgpu_sriov_unregister_vf_irq_handler(struct mtgpu_sriov_vf *vfdev,
					  u16 irq_id);

u32 mtgpu_sriov_query_available_vf_bar0(struct mtgpu_sriov *sriov);

void mtgpu_sriov_eata_access_set(struct mtgpu_sriov *sriov,
				 struct mtgpu_sriov_vf *vf,
				 u8 *table,
				 u32 size);
#else
static inline s32 mtgpu_sriov_init(struct mtgpu_device *mtdev)
{
	return -1;
}

static inline s32 mtgpu_sriov_exit(struct mtgpu_device *mtdev)
{
	return -1;
}

static inline int mtgpu_sriov_register_vf_irq_handler(struct mtgpu_sriov_vf *vfdev,
						      u16 irq_id,
						      void (*handler)(void *),
						      void *data)
{
	return 0;
}

static inline int mtgpu_sriov_unregister_vf_irq_handler(struct mtgpu_sriov_vf *vfdev,
							u16 irq_id)
{
	return 0;
}

static inline void mtgpu_sriov_eata_access_set(struct mtgpu_sriov *sriov,
					       struct mtgpu_sriov_vf *vf,
					       u8 *table,
					       u32 size)
{
}
#endif

#if (RGX_NUM_OS_SUPPORTED > 1) && (PVRSRV_APPHINT_DRIVERMODE == 0)
/* vgpu host only */
int mtgpu_sriov_vf_irq_init(struct pci_dev *pdev);
void mtgpu_sriov_vf_irq_exit(struct pci_dev *pdev);
#else
static inline int mtgpu_sriov_vf_irq_init(struct pci_dev *pdev)
{
	return 0;
}

static inline void mtgpu_sriov_vf_irq_exit(struct pci_dev *pdev)
{
}
#endif

#endif /* __MTGPU_SRIOV_H__ */
