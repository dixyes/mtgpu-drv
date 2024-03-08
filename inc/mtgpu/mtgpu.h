/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */


#ifndef __MTGPU_H__
#define __MTGPU_H__

#include "linux-types.h"
#include "mtgpu_defs.h"
#include "img_types.h"
#include "mtgpu_segment.h"

struct device;
struct pci_dev;
struct mtgpu_device;
struct mtgpu_dma_xfer_desc;
struct ipc_msg;
struct mtgpu_board_configs;
enum reset_type;
enum mtgpu_subsys_id;
struct mtgpu_resource;
typedef struct spinlock spinlock_t;
struct mtlink_ops;
struct mtlink_private_data;
enum mtgpu_clk_domain;
struct ion_device;
struct mgmt_mpc_conf;
struct mtgpu_ob_conf;
struct mtgpu_ob_map_table;
struct mtgpu_pcie_perf_bw;
struct mtgpu_pcie_link_monitor;
struct vgpu_info;
struct vgpu_share_mem;

#if defined(CONFIG_VPS)
struct vps_dma;
#endif

enum mtgpu_type_t {
	MTGPU_TYPE_SUDI104 = 0,
	MTGPU_TYPE_QUYUAN1,
	MTGPU_TYPE_QUYUAN2,
	MTGPU_TYPE_INVALID = -1,
};

struct mtgpu_interrupt_desc {
	u32 interrupt_src;
	u32 group_id_4;
	u32 group_id_8;
	char *interrupt_name;
	bool enabled;
	bool suspended;
	void (*handler_function)(void *data);
	void *handler_data;
};

struct mtgpu_interrupt_table {
	enum mtgpu_interrupt_id interrupt_id;
	struct mtgpu_interrupt_desc desc;
};

struct mtgpu_region {
	resource_size_t base;
	resource_size_t size;
};

struct mtgpu_io_region {
	struct mtgpu_region region;
	void __iomem *registers;
};

#define MTGPU_GPU_CORE_NON_PRIMARY 0xF
/*
 * group_type is gpu multi primary core group type,
 * example:
 * 4+4,   {4, 4, 0, 0, 0, 0, 0, 0,...}
 * 4+2+2, {4, 2, 2, 0, 0, 0, 0, 0,...}
 * 1+1+1+1+1+1+1+1, {1, 1, 1, 1, 1, 1, 1, 1,...}
 */
struct gpu_cfg_req {
	u8 group_type[MTGPU_CORE_COUNT_MAX];
};

/* eATA configuration IPC transmit payload */
struct gpu_eata_cfg_info {
	u32 core_id;	/* physical core id */
	u32 hyp_id;	/* userbit equal to hyp_id will bypass*/
	/**
	 * qy1: 0:keep user bit 2 1:ignore user bit 2
	 * qy2: 0:hyp id bypass 1:hyp id check or transfer
	 */
	u8 user_ctl;
	u8 size_mode;	/* 0:16G mode 1:32G mode */
	u8 eata_enable;	/* 0:disable eata, bypass whatever eata_mode is 1:eata enable */
	u8 eata_mode;	/* 0:bypass 1:check the descriptor and transfer the address */
};

/* CAUTION: this struct shared with smc, so it MUST synchronized with smc */
struct gpu_cfg_info {
	u8  group_type[MTGPU_CORE_COUNT_MAX];
	u8  primary_physical_id[MTGPU_CORE_COUNT_MAX];
	u8  primary_logical_id[MTGPU_CORE_COUNT_MAX];
	u32 actual_core_count;
};

typedef void (*interrupt_handler)(void *);

struct mtgpu_module_param {
	unsigned long mem_mode;
	unsigned long vpu_mem_size;
	unsigned long smc_mem_size;
	unsigned long cursor_size;
};

struct mtgpu_intc_ops {
	int (*init)(struct mtgpu_device *mtgpu);
	int (*claim)(struct mtgpu_device *mtgpu, int irq_vector);
	void (*clear)(struct mtgpu_device *mtgpu, int irq_vector, int intc_id);
	void (*set_state)(struct mtgpu_device *mtgpu, int interrupt_src,
			  int interrupt_target, int enable);
	void (*exit)(struct mtgpu_device *mtgpu);
	void (*suspend)(struct mtgpu_device *mtgpu);
	int (*resume)(struct mtgpu_device *mtgpu);
	int (*get_desc_table)(const struct mtgpu_interrupt_table **desc_table);
	void (*msi_config)(struct mtgpu_device *mtgpu);
};

struct mtgpu_pcie_local_mgmt_ops {
	int (*init)(struct mtgpu_device *mtgpu);
	void (*exit)(struct mtgpu_device *mtgpu);
};

struct mtgpu_intd_ops {
	void (*sgi_set_state)(struct mtgpu_device *mtgpu, int interrupt_src,
				int target_plic, int event, int enable);
	void (*spi_set_state)(struct mtgpu_device *mtgpu, int interrupt_src,
				int target_plic, int enable);
};

struct mtgpu_display_ops {
	void (*register_display_device)(struct mtgpu_device *mtdev);
};

struct mtgpu_smc_ops {
	int (*smc_check_efuse)(struct mtgpu_device *mtdev);
	int (*smc_get_board_cfg)(struct mtgpu_device *mtdev);
	int (*smc_get_ddr_clock)(struct mtgpu_device *mtdev, u32 *ddr_clock, u32 *max_ddr_clk);
	int (*smc_set_gpu_cfg)(struct mtgpu_device *mtdev, struct gpu_cfg_req *gpu_req);
	int (*smc_get_gpu_cfg)(struct mtgpu_device *mtdev, struct gpu_cfg_info *gpu_info);
	int (*smc_get_vpu_core_info)(struct mtgpu_device *mtdev, u32 *vpu_core_info);
	int (*smc_reset_subsystem)(struct mtgpu_device *mtdev, enum mtgpu_subsys_id ss_id,
				   u64 bit_mask, enum reset_type cmd);
	int (*smc_set_gpu_eata_cfg)(struct mtgpu_device *mtdev,
				    struct gpu_eata_cfg_info *eata_cfg_info);
	s32 (*smc_get_clk_id)(enum mtgpu_clk_domain domain, u16 sub_id);
};

struct mtgpu_fec_ops {
	/*
	 * Verify memory data signature. Once the verify passed,
	 * this range can not be accessed by KMD.
	 */
	int (*protect_mem)(struct mtgpu_device *mtdev, uint64_t addr, size_t len,
			   const char *signature, size_t sig_len);
	/*
	 * Remove the memory protention.
	 * The `addr` and `len` must be same as the protect_mem() call.
	 */
	int (*unprotect_mem)(struct mtgpu_device *mtdev, uint64_t addr, size_t len);
	/* Start FEC on specific PC address. Note that this address must be protectted. */
	int (*startup)(struct mtgpu_device *mtdev, uint64_t pc);
	/* Clock gating FEC. Can only be called when FEC is already started. */
	int (*clock_gating_ctl)(struct mtgpu_device *mtdev, bool clock_en);
	/* Force FEC cluster stop. May cause data loss. */
	int (*force_stop)(struct mtgpu_device *mtdev);
};

struct mtgpu_llc_ops {
	void (*persisting_get)(struct mtgpu_device *mtdev, u32 *llc_size,
			       u32 *max_llc_persisting_size);
	int (*persisting_set)(struct mtgpu_device *mtdev, u32 replace_mode, u64 max_set_aside_size);
	void (*persisting_reset)(struct mtgpu_device *mtdev);
};

struct mtgpu_ob_ops {
	int (*ob_map_cfg)(struct mtgpu_device *mtdev, struct mtgpu_ob_map_table *map_table);
	int (*ob_max_win_num)(void);
};

struct mtgpu_gpu_ss_ops {
	void (*soc_timer_enable)(struct mtgpu_device *mtdev);
	void (*soc_timer_disable)(struct mtgpu_device *mtdev);
	void (*usc_timer_enable)(struct mtgpu_device *mtdev);
	void (*usc_timer_disable)(struct mtgpu_device *mtdev);
};

struct pci_dev_config {
	u8 pci_config_data[256];
};

struct mtgpu_device;

struct mtgpu_irq_data {
	/* The interrupt vector sequence number applied by msi-x, which represents the number
	 * of interrupts applied.
	 */
	int index;
	struct mtgpu_device *mtdev;
};

struct mtgpu_device {
	struct device *dev;

	struct pci_saved_state *pci_state;

	int mem_mode;

	struct mtgpu_region bar[PCI_STD_NUM_BARS];
	struct mtgpu_region pcie_mem;
	struct mtgpu_region bar2_check_mem;
	struct mtgpu_region vpu_mem;
	struct mtgpu_region smc_mem;
	struct mtgpu_region pcie_dma;
	struct mtgpu_region gpu_mem;
	struct mtgpu_region cursor_mem;
	struct mtgpu_region efifb_mem;
	struct mtgpu_io_region llc_reg;
	struct mtgpu_io_region intc_reg;
	struct mtgpu_io_region pcie_config_reg;
	struct mtgpu_io_region pcie_phy_reg;
	struct mtgpu_io_region distributor_reg;
	struct mtgpu_io_region sram_shared_region;
	struct mtgpu_io_region gpu_ss_reg;
	int disp_cnt;
	struct platform_device *disp_dev[MTGPU_DISP_DEV_NUM];
	struct platform_device *drm_dev[MTGPU_CORE_COUNT_MAX];
	struct platform_device *video_dev;
	struct platform_device *audio_dev;

	u64 real_vram_size;

	struct dp_phy_cfg_hdr *dp_phy_cfg_hdr;

	/*cursor widht/height in pixel*/
	u32 cursor_size;
	int gpu_cnt;
	struct platform_device *gpu_dev[MTGPU_CORE_COUNT_MAX];

	struct mtgpu_chip_info *chip_info;
	struct mtgpu_intc_ops *intc_ops;
	struct mtgpu_pcie_local_mgmt_ops *pcie_local_mgmt_ops;
	struct mtgpu_intd_ops *intd_ops;
	struct mtgpu_dma_ops *dma_ops;
	struct mtgpu_sriov_ops *sriov_ops;
	struct mtgpu_pcie_perf_ops *pcie_perf_ops;
	struct mtgpu_display_ops *disp_ops;
	const struct mtgpu_smc_ops *smc_ops;
	const struct mtgpu_fec_ops *fec_ops;
	struct mtgpu_llc_ops *llc_ops;
	struct mtlink_ops *link_ops;
	struct mtgpu_ob_ops *ob_ops;
	struct mtgpu_gpu_ss_ops *gpu_ss_ops;

	int mtrr;
	spinlock_t *interrupt_handler_lock;
	spinlock_t *interrupt_enable_lock;
	struct mtgpu_interrupt_desc interrupt_desc[MTGPU_INTERRUPT_COUNT];
	u32 irq_type;
	int irq_count;
	struct mtgpu_irq_data irq_data[MTGPU_PCIE_IRQ_NUM];

	void *dma_private;
	void *ipc_private;
	void *pstate_private;
	void *fec_private;
	void *smc_private;
	struct mtgpu_ob_conf *ob_conf;
	struct mtlink_private_data *link_private_data;
	struct mtgpu_board_configs *board_configs;
	struct mtgpu_local_mgmt_info *local_mgmt;
	struct mtgpu_misc_info *miscinfo[MTGPU_CORE_COUNT_MAX];

#if defined(CONFIG_VPS)
	struct vps_dma *vps_dma;
#endif
#if defined(SUPPORT_ION)
	struct ion_device *ion_dev[MTGPU_CORE_COUNT_MAX];
#endif

	/* virtualization related members */

	/* mmu_heap_card_base store the card address of MMU table of VM */
	resource_size_t mmu_heap_card_base;

	/* gpu_mem_card_base store the card address of GPU memory base */
	resource_size_t gpu_mem_card_base;

	/* mmu_heap store the region of BAR4 in VM */
	struct mtgpu_region mmu_heap;

	/* virtual custom register store the region of BAR1 in VM */
	struct mtgpu_io_region vgpu_custom_reg;

	void *mdev_device_state;

	/*
	 * host os reserved memory size
	 * obtain: [ SMC | DMA | CURSOR | MPC 0,1,2.. FW and MMU memory |
	 * don't contain vpu host memory.
	 */
	u32 host_reserved_mem_size;

	/* struct mtgpu_sriov for sriov virtualization */
	void *sriov;
	/* How many vgpus does the current device support */
	u32 max_vgpu_supported;
	/*
	 * The starting OSID of the guest mapped by the MMU in the FW layout, not include osid0.
	 *  eg:
	 *     QY1 SR-IOV:    osid_start == 4.
	 *     QY1 mdev soft: osid_start == 1.
	 *     QY2 SR-IOV:    osid_start == 2.
	 *     QY2 mdev soft: osid_start == 1.
	 */
	u32 osid_start;
	/*
	 * How many OSIDs are mapped to the MMU in the FW layout, include osid0.
	 * eg:
	 *     max_supported_vm:14:
	 *     QY1 SR-IOV MC8*1:  osid_count = 5; (osid0, 4, 5, 6, 7)
	 *     QY1 SR-IOV MC1*8:  osid_count = 3; (osid0, 4, 5)
	 */
	u32 osid_count;

	/* store vgpu_info passed by host */
	struct vgpu_info *vgpu_info;

	/* shared memory part of guest and host */
	struct vgpu_share_mem *vgpu_shm;

	void *pm_vddr;
	resource_size_t pm_vddr_size;

	/* There may be multiple discontinuous regions
	   that need to be backed up on s3/s4
	*/
	MEM_REGION_INFO *alloced_mem_region[MTGPU_CORE_COUNT_MAX];
	int alloced_mem_region_cnt[MTGPU_CORE_COUNT_MAX];
	struct notifier_block *mtgpu_nb;
	bool pm_restore_prepare;
	enum mtgpu_type_t gpu_type;
	struct gpu_cfg_info gpu_info;
	bool mpc_is_enabled;
	struct mgmt_mpc_conf *mpc_conf;
	u32 dev_id;

	struct mtgpu_segment_info *segment_info;
	int segment_info_cnt;
	u8 video_mpc_group_ids[MTGPU_CORE_COUNT_MAX];
	u8 video_group_cnt;

	/* related to the procfs file directory */
	struct proc_dir_entry *proc_gpu_dir;
	struct proc_dir_entry *proc_vram_info;
	struct proc_dir_entry *proc_ctrl_devname;
	struct proc_dir_entry *proc_memory;
	struct proc_dir_entry *proc_status;
	struct proc_dir_entry *proc_mpc_enable;
	struct proc_dir_entry *proc_gpu_instance_dir[MTGPU_CORE_COUNT_MAX];
	struct proc_dir_entry *proc_mpc_dir;
	struct proc_dir_entry *proc_gpu_process_util;

	/*get platform device information including platform data and recoueses*/
	int (*get_platform_device_info)(struct mtgpu_device *mtdev, u32 hw_module, u32 hw_id,
					struct mtgpu_resource **mtgpu_res, u32 *num_res,
					void **data, size_t *size_data);

	/* query if the driver can access registers */
	int (*register_access_check)(struct mtgpu_device *mtdev);

	struct pci_dev_config *pci_dev_config_data;

	bool pstate_supported;

	struct mtgpu_pcie_perf_bw *pcie_perf_data;

	struct mtgpu_pcie_link_monitor *pcie_link_monitor;

};

extern struct device_ops sudi_ops;
extern struct mtgpu_chip_info sudi_chip_info;

extern struct device_ops quyuan1_ops;
extern struct mtgpu_chip_info quyuan1_chip_info;

extern struct dentry *mtgpu_dentry;

#endif /* __MTGPU_H__ */
