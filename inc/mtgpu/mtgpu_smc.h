/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_SMC_H__
#define __MTGPU_SMC_H__

#include "linux-types.h"
#include "mtgpu.h"

enum mtgpu_clk_domain {
	CLK_DOMAIN_GPU,
	CLK_DOMAIN_XPU,
	CLK_DOMAIN_VID,
	CLK_DOMAIN_AUD,
	CLK_DOMAIN_PCIE,
	CLK_DOMAIN_NOC,
	CLK_DOMAIN_DDR,
	CLK_DOMAIN_DISP,
	CLK_DOMAIN_HDMI,
	CLK_DOMAIN_DP,
	CLK_DOMAIN_SM,
	CLK_DOMAIN_FE,
	CLK_DOMAIN_MTLINK,
	CLK_DOMAIN_MAX
};

int mtgpu_smc_get_mem_clk(struct device *dev, u32 *ddr_clk, u32 *max_ddr_clk);
s64 mtgpu_smc_get_frequence(struct device *dev, enum mtgpu_clk_domain domain, u16 sub_id);
s64 mtgpu_smc_get_max_frequence(struct device *dev, enum mtgpu_clk_domain domain, u16 sub_id);
int mtgpu_smc_set_frequence(struct device *dev, enum mtgpu_clk_domain domain, u16 sub_id, u64 freq);

enum reset_type {
	RESET_TYPE_ASSERT = 1,
	RESET_TYPE_DEASSERT,
	RESET_TYPE_PULSE,
};

enum mtgpu_subsys_id {
	/*
	 * In order to support QY and further arch, function prototype has changed,
	 * But this is an exported symbol, usually invoked by mtsnd.ko.
	 * To avoid mismatch between KMD version and the caller's version,
	 * Let ID start from 20 can makesure old caller gets an error return.
	 */
	SS_ID_MIN = 20,
	SS_ID_VID = SS_ID_MIN,
	SS_ID_AUD,
	SS_ID_GPU,
	SS_ID_DISP,
	SS_ID_FEC,
	SS_ID_DRAM,
	SS_ID_MAX
};

/*
 * The u64 bit_mask definition is platform-dependent. For example when ssid=SS_ID_GPU:
 *  - There are 4 gpu cores on SUDI, so the core mask is bit[3:0]
 *  - There are 2*4 gpu cores on QUYAUN, and an GPU_PERI for each cluster
 *       1. GPU0 mask is bit[3:0]
 *       2. GPU0_PERI is bit4
 *       3. GPU1 mask is bit[8:5]
 *       4. GPU1_PERI is bit9
 * For more information, visit `https://confluence.mthreads.com/display/FW/subsystem+reset+function`
 */
int mtgpu_smc_reset_subsystem(struct device *dev, enum mtgpu_subsys_id ss_id,
			      u64 bit_mask, enum reset_type type);

/**
 * enum pstate_lvl  - Describe the pstate level
 * @PSTATE_LVL_P0: graphic(full perf):PC gaming/maximum 3D
 * @PSTATE_LVL_P12: short idle(static screen)
 */
enum pstate_lvl {
	PSTATE_LVL_P0       = 0,
	PSTATE_LVL_P1       = 1,
	PSTATE_LVL_P2       = 2,
	PSTATE_LVL_P3       = 3,
	PSTATE_LVL_P4       = 4,
	PSTATE_LVL_P5       = 5,
	PSTATE_LVL_P6       = 6,
	PSTATE_LVL_P7       = 7,
	PSTATE_LVL_P8       = 8,
	PSTATE_LVL_P9       = 9,
	PSTATE_LVL_P10      = 10,
	PSTATE_LVL_P11      = 11,
	PSTATE_LVL_P12      = 12,
	PSTATE_LVL_P13      = 13,
	PSTATE_LVL_P14      = 14,
	PSTATE_LVL_P15      = 15,
	PSTATE_LVL_MAX      = 0xFFFF,
};

enum pstate_os_flag {
	PSTATE_OS_LINUX     = 0,
	PSTATE_OS_WINDOWS   = 1,
	PSTATE_OS_MAX,
};

enum dsp_register {
	DSP_REGISTER_PRID,
	DSP_REGISTER_RST_VEC,
	DSP_REGISTER_PCONTROL,
};

enum smc_report_event_id {
	IPC_EVENT_REPORT_EVENT_TEMPERATURE = 0,
	IPC_EVENT_REPORT_EVENT_POWER_LIMIT = 1,
	IPC_EVENT_REPORT_EVENT_FAN         = 2,
	IPC_EVENT_REPORT_EVENT_SHUTDOWN    = 3,
	IPC_EVENT_REPORT_EVENT_EATA_ERR    = 4,
	IPC_EVENT_REPORT_EVENT_TRAP        = 5,

	IPC_EVENT_REPORT_EVENT_MAX
};

struct pstate_pcie_desc {
	u8 pcie_mode;
	u8 pcie_speed;
	u8 pcie_width;
	u8 rsv;
};

struct pstate_entry_info {
	u8 supported;
	u8 version;
	u8 flag;
	u8 count;
	u32 bitmap;
	struct pstate_pcie_desc pcie_desc[PSTATE_LVL_P15 + 1];
};

int mtgpu_smc_set_pstate(struct device *dev, enum pstate_lvl pstate_lvl);
int mtgpu_smc_get_pstate_entry_info(struct device *dev,
				    struct pstate_entry_info *entry);
int mtgpu_smc_get_rtos_version(struct device *dev);
int mtgpu_smc_get_dp_phy_cfg(struct device *dev, void **dp_phy_cfg);
bool mtgpu_smc_get_secure_bit(struct device *dev);
int mtgpu_smc_get_board_config(struct device *dev);
void mtgpu_smc_release_board_config(struct device *dev);
int mtgpu_smc_set_gpu_cfg(struct device *dev, struct gpu_cfg_req *gpu_req);
int mtgpu_smc_get_gpu_cfg(struct device *dev, struct gpu_cfg_info *gpu_info);
int mtgpu_smc_get_vpu_core_info(struct device *dev, u32 *vpu_core_info);
int mtgpu_smc_set_gpu_eata_cfg(struct device *dev, struct gpu_eata_cfg_info *eata_cfg_info);
int mtgpu_smc_read_dsp_register(struct device *dev, enum dsp_register reg, u32 *value);
int mtgpu_smc_write_dsp_register(struct device *dev, enum dsp_register reg, u32 value);
int mtgpu_smc_init(struct mtgpu_device *mtdev);
void mtgpu_smc_exit(struct mtgpu_device *mtdev);

#endif /* __MTGPU_SMC_H__ */
