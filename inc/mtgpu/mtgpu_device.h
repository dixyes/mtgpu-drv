/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DEVICE_H__
#define __MTGPU_DEVICE_H__
#include "mtgpu.h"
#define MTGPU_PCIE_DMA_DESC_MEM_SIZE		0x800000

struct mtlink_ops;
struct mtlink_device;
struct mtgpu_device;
struct mtgpu_platform_data;
struct crypto_shash;

enum hw_module_type {
	MTGPU_HW_MODULE_GPU = 0,
	MTGPU_HW_MODULE_DISPC,
	MTGPU_HW_MODULE_DP,
	MTGPU_HW_MODULE_DP_PHY,
	MTGPU_HW_MODULE_HDMI,
	MTGPU_HW_MODULE_VPU,
	MTGPU_HW_MODULE_AUDIO,
};

struct mtgpu_resource {
	resource_size_t start;
	resource_size_t end;
	const char *name;
	bool is_mem;
};

struct mtgpu_chip_info {
	u32 sys_region_size;

	u32 clk_reset_reg_offset;
	u32 clk_reset_reg_size;

	u32 dsp_reg_offset;
	u32 dsp_reg_size;

	u32 fedma_reg_offset;
	u32 fedma_reg_size;

	u32 noc_reg_offset;
	u32 noc_reg_size;

	u32 vid_reg_offset;
	u32 vid_reg_size;

	u32 socif_offset;
	u32 socif_size;

	u32 intc_regs_offset;
	u32 intc_regs_size;

	u32 distributor_regs_offset;
	u32 distributor_regs_size;

	u32 pcie_config_regs_offset;
	u32 pcie_config_regs_size;

	u32 gpu_ss_reg_offset;
	u32 gpu_ss_reg_size;

	u32 pcie_ss_cfg_reg_offset;
	u32 pcie_ss_cfg_reg_size;

	u32 pcie_vf_msi_regs_offset;
	u32 pcie_vf_msi_regs_size;

	u32 pcie_phy_cfg_reg_offset;
	u32 pcie_phy_cfg_reg_size;

	u32 pcie_sram_shared_offset;
	u32 pcie_sram_shread_size;

	u32 ddr_mem_offset;
	u32 default_ddr_bar_size;

	u32 llc_reg_offset;
	u32 llc_reg_size;

	u32 cursor_reserved_size;

	u32 vps_debug_bar;

	/* gpu core num per chip*/
	u32 gpu_core_num;

	u64 ob_addr_flag;
};

struct mtlink_private_data {
	/* TODO: maybe capability is here */

	struct mtlink_device *link_device;
};

struct mtgpu_driver_data {
	struct mtgpu_chip_info *chip_info;
	struct mtgpu_dma_ops *dma_ops;
	struct mtgpu_intc_ops *intc_ops;
	struct mtgpu_intd_ops *intd_ops;
	struct mtgpu_display_ops *disp_ops;
	struct mtgpu_pcie_local_mgmt_ops *pcie_local_mgmt_ops;
	struct mtgpu_sriov_ops *sriov_ops;
	struct mtgpu_pcie_perf_ops *pcie_perf_ops;
	const struct mtgpu_smc_ops *smc_ops;
	struct mtgpu_llc_ops *llc_ops;
	struct mtlink_ops *link_ops;
	struct mtgpu_ob_ops *ob_ops;
	struct mtgpu_gpu_ss_ops *gpu_ss_ops;
	int (*get_platform_device_info)(struct mtgpu_device *mtdev, u32 hw_module, u32 hw_id,
					struct mtgpu_resource **mtgpu_res, u32 *num_res,
					void **data, size_t *size_data);
	void (*get_vendor_info)(struct mtgpu_device *mtdev, u32 *vendor);
};

#define MT_OPS_DECLARE(name) \
extern struct mtgpu_intc_ops name##_intc_ops; \
extern struct mtgpu_intd_ops name##_intd_ops; \
extern struct mtgpu_llc_ops name##_llc_ops; \
extern struct mtgpu_dma_ops name##_dma_ops; \
extern struct mtgpu_sriov_ops name##_sriov_ops; \
extern struct mtgpu_pcie_perf_ops name##_pcie_perf_ops; \
extern struct mtgpu_display_ops name##_display_ops; \
extern struct mtgpu_pcie_local_mgmt_ops name##_pcie_local_mgmt_ops; \
extern struct mtgpu_smc_ops name##_smc_ops; \
extern struct mtlink_ops name##_mtlink_ops; \
extern struct mtgpu_ob_ops name##_ob_ops; \
extern struct mtgpu_gpu_ss_ops name##_gpu_ss_ops;

int mtgpu_resource_init_mem(struct mtgpu_resource *res, resource_size_t start,
			    int size, const char *name);
int mtgpu_resource_init_irq(struct mtgpu_resource *res, resource_size_t irq, const char *name);
int mtgpu_platform_data_vz_init(struct mtgpu_device *mtdev, struct mtgpu_platform_data *pdata);
int mtgpu_device_common_init(struct mtgpu_device *mtdev, struct mtgpu_module_param *param);
void mtgpu_device_common_exit(struct mtgpu_device *mtdev);
int mtgpu_get_primary_core_count(struct mtgpu_device *mtdev);
int mtgpu_register_devices(struct mtgpu_device *mtdev);
void mtgpu_unregister_devices(struct mtgpu_device *mtdev);
int mtgpu_device_pm_resume(struct mtgpu_device *mtdev);
int mtgpu_device_pm_suspend(struct mtgpu_device *mtdev);
void mtgpu_device_alloc_buffer_for_suspend(struct mtgpu_device *mtdev);
int mtgpu_device_memory_fixup(struct mtgpu_device *mtdev,
			      struct mtgpu_module_param *param);
int mtgpu_pcie_resize_videomem_bar(struct mtgpu_device *mtdev, int resize);

int mtgpu_device_common_init(struct mtgpu_device *mtdev, struct mtgpu_module_param *param);
int mtgpu_register_devices(struct mtgpu_device *mtdev);
void mtgpu_unregister_devices(struct mtgpu_device *mtdev);
void mtgpu_device_common_exit(struct mtgpu_device *mtdev);
int mtgpu_mtrr_setup(struct mtgpu_device *mtdev);
void mtgpu_mtrr_cleanup(struct mtgpu_device *mtdev);
int request_pci_io_addr(struct pci_dev *pdev, u32 index,
			resource_size_t offset, resource_size_t length);
void release_pci_io_addr(struct pci_dev *pdev, u32 index,
			 resource_size_t start, resource_size_t length);
int remap_io_region(struct pci_dev *pdev,
		    struct mtgpu_io_region *region, u32 index,
		    resource_size_t offset, resource_size_t size);
void cleanup_io_region(struct pci_dev *pdev,
		       struct mtgpu_io_region *region, u32 index);
int mtgpu_pcie_error_report_init(struct mtgpu_device *mtdev);
void mtgpu_pcie_error_report_exit(struct mtgpu_device *mtdev);
int mtgpu_register_audio_device(struct mtgpu_device *mtdev);
void mtgpu_unregister_audio_device(struct mtgpu_device *mtdev);
int mtgpu_register_video_device(struct mtgpu_device *mtdev);
void mtgpu_unregister_video_device(struct mtgpu_device *mtdev);
int mtgpu_register_gpu_device_with_coreid(struct mtgpu_device *mtdev, int logic_core_id,
					  int physical_core_id);
void mtgpu_unregister_gpu_device(struct mtgpu_device *mtdev);
void mtgpu_register_dummy_device_with_core_id(struct mtgpu_device *mtdev, u8 coreid);
void mtgpu_register_drm_device_with_core_id(struct mtgpu_device *mtdev, u8 coreid);
void mtgpu_unregister_drm_device(struct mtgpu_device *mtdev);
void mtgpu_unregister_display_device(struct mtgpu_device *mtdev);
void mtgpu_register_device_done(struct mtgpu_device *mtdev);
int mtgpu_device_register_pm_nb(struct mtgpu_device *mtdev);
int mtgpu_device_unregister_pm_nb(struct mtgpu_device *mtdev);
int mtgpu_register_dispc_device(struct mtgpu_device *mtdev, u8 hw_id);
int mtgpu_register_dp_device(struct mtgpu_device *mtdev, u8 hw_id);
int mtgpu_register_hdmi_device(struct mtgpu_device *mtdev);
void mtgpu_pci_dev_config_data_release(struct mtgpu_device *mtdev);
void mtgpu_gpu_timer_enable(struct mtgpu_device *mtdev);
void mtgpu_gpu_timer_disable(struct mtgpu_device *mtdev);
int mtgpu_guest_notify_host_vpu(struct device *dev);

#endif /* __MTGPU_DEVICE_H__ */
