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
struct mtgpu_softirq_info;
struct mtgpu_softirq_ctrl;
struct mtgpu_vdma_buffer;
struct wait_queue_head;
struct mtgpu_fec_umd_init_info;

#if defined(CONFIG_VPS)
struct vps_dma;
#endif

enum mtgpu_type_t {
	MTGPU_TYPE_SUDI104 = 0,
	MTGPU_TYPE_QUYUAN1,
	MTGPU_TYPE_QUYUAN2,
	MTGPU_TYPE_INVALID = -1,
};

struct mtgpu_irq_desc {
	/* logic interrupt id */
	u32 int_id;
	const char *name;
	bool enabled;
	bool suspended;
	void (*handler)(void *data);
	void *handler_data;
	u64 received_total;
};

struct mtgpu_int_mon_stat {
	bool pending;
	u64 intr_count;
};

struct mtgpu_irq_data {
	/* The interrupt vector sequence number applied by msi-x, which represents the number
	 * of interrupts applied.
	 */
	int target;
	int host_irq;
	int claim_int_times;
	int complete_int_times;
	int last_claim_int_id;
	struct mtgpu_device *mtdev;
};

struct mtgpu_irq_info {
	u32 irq_type;
	u32 irq_cnt;
	u32 desc_cnt;
	struct mtgpu_int_mon_stat *int_monitor_stat;
	struct mtgpu_irq_desc **desc_table;
	struct mtgpu_irq_data *irq_data;
	spinlock_t *irq_handler_lock;
	spinlock_t *irq_enable_lock;
	struct timer_list *int_monitor_timer;
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

struct mtgpu_int_ops {
	int (*init)(struct mtgpu_device *mtgpu);
	int (*enable)(struct mtgpu_device *mtgpu, u32 int_vec, bool enable);
	int (*claim)(struct mtgpu_device *mtgpu, int target);
	int (*complete)(struct mtgpu_device *mtgpu, int int_vec, int target);
	void (*exit)(struct mtgpu_device *mtgpu);
	bool (*pending)(struct mtgpu_device *mtgpu, int intc_id);
	int (*mask)(struct mtgpu_device *mtgpu, u32 int_vec);
	int (*unmask)(struct mtgpu_device *mtgpu, u32 int_vec);
};

struct mtgpu_pcie_local_mgmt_ops {
	int (*init)(struct mtgpu_device *mtgpu);
	void (*exit)(struct mtgpu_device *mtgpu);
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
	int (*ob_reset_map_cfg)(struct mtgpu_device *mtdev);
	void (*ob_dump_map_cfg)(struct mtgpu_device *mtdev);
};

struct mtgpu_gpu_ss_ops {
	void (*soc_timer_enable)(struct mtgpu_device *mtdev);
	void (*soc_timer_disable)(struct mtgpu_device *mtdev);
	void (*usc_timer_enable)(struct mtgpu_device *mtdev);
	void (*usc_timer_disable)(struct mtgpu_device *mtdev);
};

struct mtgpu_pfm_ops {
	int (*mss_set)(struct mtgpu_device *mtdev, void *mss_cfg, void *pfm_mss_mmu);
	void (*mss_clear)(struct mtgpu_device *mtdev);
	void (*mss_dump)(struct mtgpu_device *mtdev, void *mss_dump_trig, void *pfm_mss_mmu);
	void (*mss_get_wrap)(struct mtgpu_device *mtdev, void *buf_addr);
	void (*mss_get_mmu)(struct mtgpu_device *mtdev, void *buf_addr);

};

struct pci_dev_config {
	u8 pci_config_data[256];
};

struct mtgpu_device;

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
	struct mtgpu_io_region intd_cd_reg;
	struct mtgpu_io_region pcie_mhi_reg;
	struct mtgpu_io_region pfm_mmu_reg;
	struct mtgpu_io_region pfm_ddrc_reg;
	struct mtgpu_io_region pfm_d2d_1_reg;
	struct mtgpu_io_region pfm_d2d_0_reg;
	struct mtgpu_io_region pfm_llc_reg;
	struct mtgpu_io_region gpu_daa_reg;
	struct mtgpu_io_region gpu_misc_reg;
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
	int mtrr;
	struct platform_device *gpu_dev[MTGPU_CORE_COUNT_MAX];

	struct mtgpu_chip_info *chip_info;
	struct mtgpu_int_ops *int_ops;
	struct mtgpu_pcie_local_mgmt_ops *pcie_local_mgmt_ops;
	struct mtgpu_dma_ops *dma_ops;
	struct mtgpu_sriov_ops *sriov_ops;
	struct mtgpu_pcie_perf_ops *pcie_perf_ops;
	struct mtgpu_display_ops *disp_ops;
	const struct mtgpu_smc_ops *smc_ops;
	const struct mtgpu_fec_ops *fec_ops;
	struct mtgpu_llc_ops *llc_ops;
	struct mtgpu_pfm_ops *pfm_ops;
	struct mtlink_ops *link_ops;
	struct mtgpu_ob_ops *ob_ops;
	struct mtgpu_gpu_ss_ops *gpu_ss_ops;

	struct mtgpu_softirq_info *softirq_info;
	struct mtgpu_irq_info *irq_info;
	void *dma_private;
	void *ipc_private;
	void *pstate_private;
	void *fec_private;
	void *smc_private;
	void *event_report_private;
	struct mtgpu_ob_conf *ob_conf;
	struct mtlink_private_data *link_private_data;
	struct mtgpu_board_configs *board_configs;
	struct mtgpu_local_mgmt_info *local_mgmt;
	struct mtgpu_misc_info *miscinfo[MTGPU_CORE_COUNT_MAX];
	struct mtgpu_vdma_buffer *vdma_buffer;
	struct wait_queue_head *vdma_wqh;
	struct mutex *vdma_lock;

#if defined(CONFIG_VPS)
	struct vps_dma *vps_dma;
#endif
#if defined(SUPPORT_ION)
	struct ion_device *ion_dev[MTGPU_CORE_COUNT_MAX];
#endif

	/* virtualization related members */

	struct mtgpu_region fw_heap;

	/* virtual custom register store the region of BAR1 in VM */
	struct mtgpu_io_region vgpu_custom_reg;

	void *mdev_device_state;

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
	struct proc_dir_entry *proc_event_report;
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

	int (*get_softirq_ctrls)(struct mtgpu_softirq_ctrl **softirq_ctrls, int *ctrls_cnt);

	struct pci_dev_config *pci_dev_config_data;

	bool pstate_supported;
	u32 pstate_p0_count;
	u32 pstate_p12_count;

	struct mtgpu_pcie_perf_bw *pcie_perf_data;

	int pcie_des_pos;

	struct mtgpu_pcie_link_monitor *pcie_link_monitor;

#if defined(SUPPORT_VGPU_HWPERF)
	void *vgpu_perf_data;
#endif
	struct mtgpu_fec_umd_init_info *fec_umd_init_info;

	u64 hw_capability;
};

extern struct device_ops sudi_ops;
extern struct mtgpu_chip_info sudi_chip_info;

extern struct device_ops quyuan1_ops;
extern struct mtgpu_chip_info quyuan1_chip_info;

extern struct dentry *mtgpu_dentry;

#endif /* __MTGPU_H__ */
