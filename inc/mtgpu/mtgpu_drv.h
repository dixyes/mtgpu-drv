/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRV_H__
#define __MTGPU_DRV_H__

#include "mtgpu_defs.h"
#include "mtgpu.h"

#define DRIVER_NAME		"mtgpu"
#define DEV_UUID_LEN		16

#define GPU_SOC_GEN1		1
#define GPU_SOC_GEN2		2
#define GPU_SOC_GEN3		3

struct vgpu_info;
struct mtgpu_vz_data {
	/* Heap for gpu mmu table */
	resource_size_t mmu_heap_base;
	resource_size_t mmu_heap_size;
	/* Real vram address */
	resource_size_t mmu_heap_card_base;
	resource_size_t fw_heap_card_base;

	void __iomem *virt_regs;
	/* The numerical order of all the multicores. */
	u32 mpc_id;
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

	struct vgpu_info *vgpu_info;

	void (*vgpu_int_cb)(u32 int_id, bool is_osid0, void *priv_data, u32 mpc_id);
	void *priv_data;

#if defined(SUPPORT_SW_OSID_EXTENSION)
	/*
	 * TODO:
	 * 1. Change callback set_master_kick_reg to a __iomem pointer
	 * 2. Remove callback vgpu_kick, kick the OSID0 directly
	 */
	void (*set_master_kick_reg)(void *master_kick_reg, void *priv_data, u32 mpc_id);
	void (*vgpu_kick)(int osid, u32 kick_value, void *priv_data);
#endif
};

struct mtgpu_platform_data {
	u32 primary_core_id;
	/* The mtgpu memory mode (LOCAL, HOST or HYBRID) */
	int mem_mode;

	/* The base address of the video ram (CPU physical address) -
	 * used to convert from CPU-Physical to device-physical addresses
	 */
	resource_size_t pcie_memory_base;

	/* Heap for pvr gpu using */
	resource_size_t gpu_memory_base;
	resource_size_t gpu_memory_size;

	/* DMA channel names for MT usage */
	char *mtgpu_dma_tx_chan_name;
	char *mtgpu_dma_rx_chan_name;

	struct mtgpu_segment_info *segment_info;

	struct mtgpu_vz_data vz_data;
#if defined(SUPPORT_ION)
	struct ion_device *ion_dev;
#endif

	char uuid[DEV_UUID_LEN];
};

struct mtgpu_video_platform_data {
	resource_size_t video_mem_base;
	resource_size_t video_mem_size;
	resource_size_t pcie_mem_base;
#if defined(SUPPORT_ION)
	struct ion_device **ion_dev;
#endif
	u16 pcie_dev_id;
};

struct mtgpu_jpu_platform_data {
#if defined(SUPPORT_ION)
	struct ion_device **ion_dev;
#endif
	u16 pcie_dev_id;
};

struct mtgpu_drm_platform_data {
	resource_size_t fb_base;
	resource_size_t fb_size;
};

struct mtgpu_dispc_platform_data {
	u8 id;
	resource_size_t pcie_mem_base;
	resource_size_t cursor_mem_base;
	resource_size_t cursor_mem_size;
	u8 soc_gen;
};

struct mtgpu_dp_platform_data {
	u8 id;
	u16 max_hres;
	u16 max_vres;
	u16 max_pclk_100khz;
	u8 port_type;
	u8 soc_gen;
};

struct mtgpu_dp_phy_platform_data {
	u8 id;
	struct dp_phy_cfg_hdr *phy_cfg_hdr;
	u8 soc_gen;
};

struct mtgpu_hdmi_platform_data {
	u16 max_hres;
	u16 max_vres;
	u16 max_pclk_100khz;
	u8 port_type;
};

extern struct mtgpu_driver_data sudi_drvdata;
extern struct mtgpu_driver_data quyuan1_drvdata;
extern struct mtgpu_driver_data quyuan2_drvdata;
#if defined(OS_STRUCT_PROC_OPS_EXIST)
extern const struct proc_ops config_proc_ops;
extern const struct proc_ops mpc_enable_proc_ops;
extern const struct proc_ops mtlink_test_proc_ops;
extern const struct proc_ops process_util_proc_ops;
#else
extern const struct file_operations config_proc_ops;
extern const struct file_operations mpc_enable_proc_ops;
extern const struct file_operations mtlink_test_proc_ops;
extern const struct file_operations process_util_proc_ops;
#endif

bool mtgpu_display_is_dummy(void);
bool mtgpu_display_is_none(void);
bool mtgpu_card_support_display(struct mtgpu_device *mtdev);
bool mtgpu_card_is_server(struct pci_dev *pdev);

int mtgpu_enable_interrupt(struct device *mtdev, int interrupt_id);
int mtgpu_disable_interrupt(struct device *mtdev, int interrupt_id);
int mtgpu_set_interrupt_handler(struct device *mtdev, int interrupt_id,
				void (*handler_function)(void *),
				void *handler_data);
int mtgpu_get_driver_mode(void);
u64 mtgpu_get_vram_size(struct mtgpu_device *mtdev);
bool mtgpu_sriov_enabled(struct pci_dev *pdev);
bool mtgpu_pstate_is_enabled(void);

#endif /* __MTGPU_DRV_H__ */
