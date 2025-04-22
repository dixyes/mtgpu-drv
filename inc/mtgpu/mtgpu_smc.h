/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_SMC_H__
#define __MTGPU_SMC_H__

#include "linux-types.h"
#include "mtgpu.h"
#include "mtgpu_pstate.h"

#define SMC_EVENT_LOG_BUFF_LEN			(4096)
#define MTGPU_SMC_RB_SIZE			(4096)
#define MTGPU_SMC_IPC_RETRY_TIMES		(10)
#define MTGPU_SMC_IPC_RETRY_INTERVAL		(100)
#define MTGPU_SMC_POLLING_WAIT_USEC		(10)
#define MTGPU_SMC_IPC_MSG_STATUS_NOT_SUPPORT	(64)
#define FW_MAX_PARTITION_COUNT			(9)
#define DDR_MAX_PHY_NUM				(12)

#define SUDI_RTOS_VERSION_MIN_WITH_PSTATE	10202 /* rtos v1.2.2 */
#define QY1_RTOS_VERSION_MIN_WITH_PSTATE	20102 /* rtos v2.1.2 */

struct smc_event_log_ctrl {
	u32  read_index;
	u32  write_index;
	u32  buff_size;
	u32  remain_size;
	u8   buff[SMC_EVENT_LOG_BUFF_LEN];
};

struct smc_event_info {
	const char *name;
	struct mutex *event_lock;/* lock the buff when write and load */
	struct smc_event_log_ctrl *event_log_ctrl;
	struct device_attribute *event_attr;
	u32 highest_temperature;
	u32 over_temperature_count;
};

struct mtgpu_smc_info {
	dma_addr_t local_buf_smc_pa;
	dma_addr_t local_buf_cpu_pa;
	void *local_buf_cpu_va;
	struct smc_event_info event_info;
	/*
	 * only for qy1 
	 * lock the idle register set by IPC_EVENT_SET_REG or IPC_EVENT_SS_RESET
	 */
	struct mutex *idle_reg_lock;
};

struct mtgpu_smc_rb_switch_event {
	u64 host_rb_base;
	u64 host_rb_size;
};

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

int mtgpu_smc_ipc_retcode_to_errno(int ipc_retcode);
int mtgpu_smc_ipc_transmit(struct device *dev,
			   struct ipc_msg *ipc_msg,
			   u64 command_id,
			   u8 data_size);

int mtgpu_smc_get_mem_clk(struct device *dev, u32 *ddr_clk, u32 *max_ddr_clk);
u32 mtgpu_smc_get_gpu_core_clk(struct mtgpu_device *mtdev);
s64 mtgpu_smc_get_frequence(struct device *dev, enum mtgpu_clk_domain domain, u16 sub_id);
s64 mtgpu_smc_get_max_frequence(struct device *dev, enum mtgpu_clk_domain domain, u16 sub_id);
int mtgpu_smc_set_frequence(struct device *dev, enum mtgpu_clk_domain domain, u16 sub_id, u64 freq);
int mtgpu_smc_efuse_check(struct mtgpu_device *mtdev);

#define SM_PMUC_RESET_NOT_SUPPORT   -1

enum reset_type {
	RESET_TYPE_ASSERT = 1,
	RESET_TYPE_DEASSERT,
	RESET_TYPE_PULSE,
	RESET_TYPE_DISABLE_CLOCK,
	RESET_TYPE_ENABLE_CLOCK,
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
	SS_ID_MTLINK,
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

enum smc_fan_speed_fmt {
	FAN_SPEED_RPM = 0,	/* rotations per minute */
	FAN_SPEED_PERCENT,	/* fan speed percent */

	/* Keep this on the last line */
	FAN_SPEED_MAX
};

enum smc_power_domain {
	POWER_DOMAIN_TOTAL,
	POWER_DOMAIN_PCIE,
	POWER_DOMAIN_EXT12V,

	POWER_DOMAIN_MAX
};

enum smc_ddr_error_type {
	DDR_ERROR_TYPE_EDC,

	DDR_ERROR_TYPE_TYPE_MAX
};

enum smc_power_monitor_event_id {
	POWER_MONITOR_EVENT_QUERY_FREQ_THROTTLE_REASON,
	POWER_MONITOR_EVENT_QUERY_POWER_LIMIT_INFO,
	POWER_MONITOR_EVENT_QUERY_THERMAL_LIMIT_INFO,
	POWER_MONITOR_EVENT_RSV = 60,
	POWER_MONITOR_EVENT_SET_POWER_LIMIT,
	POWER_MONITOR_EVENT_SET_POWER_CTRL_PID,

	POWER_MONITOR_EVENT_MAX,
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

struct ipc_msg_pstate {
	u32 os_flag;   /* OS flag, 0-Linux, 1-Windows */
	u32 pstate;    /* pstate level */
	u32 status;    /* pstate set/get status */
};

struct ipc_msg_pstate_entry {
	u32 status;                            /* command exec status */
	u32 os_flag;                           /* OS flag, 0-Linux, 1-Windows */
	struct pstate_entry_info entry_info;   /* entry info */
};

struct ipc_msg_clock {
	u32 clock_id;           /* clock type */
	u32 status;             /* set or get status */
	u64 freq;               /* frequence Hz*/
};

struct ipc_msg_fan_count {
	u32 status;		/* 0: command exec pass,
				 * 64:not support the command,
				 * other: command error
				 */
	u32 count;		/* number of fans */
};

struct ipc_msg_fan_rpm {
	u32 channel;
	u32 rpm;
	u32 status;
};

struct ipc_msg_fan_percent {
	u32 status;		/* 0: command exec pass,
				 * 64:not support the command,
				 * other: command error
				 */
	u32 channel;		/* index of the fans */
	u32 duty;		/* the duty value of a fan */
	u32 max_duty;		/* the max duty value of a fan.
				 * the fan speed percent = duty_ / maxDuty_ * 100%.
				 */
};

struct ipc_msg_temperature {
	u32 pvt_type;		/* pvt type */
	u32 temperature;	/* temperature */
	u32 status;		/* set or get status */
};

struct ipc_msg_power {
	u32 power_domain;
	u32 power;
	u32 status;
};

struct ipc_msg_gpu_voltage {
	u32 sensor_index;	/* [in] */
	u32 voltage;		/* [out] voltage (mV) */
	u32 status;		/* [out] 0: command exec pass,
				 *	64:not support the command,
				 *	other: command error
				 */
};

struct firmware_version {
	u8 name[8];
	u32 version;
};

struct ipc_msg_ddr_channel_sensor_temp {
	u32 ch0_sensor0;
	u32 ch0_sensor1;
	u32 ch1_sensor0;
	u32 ch1_sensor1;
};

struct ipc_msg_ddr_temp {
	/* Error code from ipc state. */
	u32 status;
	/* The mask of ddr channels. */
	u32 ddr_channel_mask;
	/* Temperature for every sensor. */
	struct ipc_msg_ddr_channel_sensor_temp sensor_temps[DDR_MAX_PHY_NUM];
};

struct ipc_msg_ddrc_rambus_error {
	u32 edc_retry_en        : 1;  /* EDC on/off*/
	u32 edc_retry_attempts  : 2;  /* retry times when error happens */
	u32 edc_retry_fifostat  : 7;  /* fifo status when retry */
	u32 rsv_1               : 22;
	u16 write_edc_error;          /* total num of EDC write error */
	u16 read_edc_error;           /* total num of EDC read error */
	u8 retry_rd_corr;             /* num of correctable EDC read error */
	u8 retry_rd_uncorr;           /* num of uncorrectable EDC read error */
	u8 retry_wr_corr;             /* num of correctable EDC write error */
	u8 retry_wr_uncorr;           /* num of uncorrectable EDC write error */
};

struct ipc_msg_ddr_error {
	/* [in] enum smc_ddr_error_type */
	u32 error_type;
	/* [out] */
	u32 msg_size;
	/* [in] the consective physical address of error info */
	u64 error_phy_addr;
};

struct ddr_edc_error_counter {
	u32 retry_rd_corr;             /* num of correctable EDC read error */
	u32 retry_rd_uncorr;           /* num of uncorrectable EDC read error */
	u32 retry_wr_corr;             /* num of correctable EDC write error */
	u32 retry_wr_uncorr;           /* num of uncorrectable EDC write error */
};

struct ipc_msg_clock_throttle_info {
	u32 clk_type;
	u32 throttle_bitmap;
	u32 effective_throttle_bit;
	u64 min_freq;
	u8 rsv[8];
};

struct ipc_msg_power_limit_pid_cfg {
	s64 kp;
	s64 ki;
	s64 kd;
};

struct ipc_msg_power_limit_cfg {
	s32 p_control_b;
	u32 power_limit_value;
	struct ipc_msg_power_limit_pid_cfg pid_cfg;
};

struct ipc_msg_power_limit_info {
	u32 cur_limit;
	u32 max_limit;
	u32 min_limit;
	u32 default_limit;
};

struct ipc_msg_thermal_limit_info {
	u32 cur_slowdown;
	u32 cur_shutdown;
	u32 def_slowdown;
	u32 def_shutdown;
};

union ipc_msg_pm_event_data {
	struct ipc_msg_clock_throttle_info info;
	struct ipc_msg_power_limit_cfg cfg;
	struct ipc_msg_power_limit_info pinfo;
	struct ipc_msg_thermal_limit_info tinfo;
};

struct ipc_msg_pm_event {
	u32 status;
	u32 version;
	u32 event_id;				/* [in] enum smc_power_monitor_event_id */
	union ipc_msg_pm_event_data data;	/* [out] */
};

struct mtgpu_smc_ops {
	int (*check_efuse)(struct mtgpu_device *mtdev);
	int (*get_board_cfg)(struct mtgpu_device *mtdev);
	int (*get_ddr_clock)(struct mtgpu_device *mtdev, u32 *ddr_clock, u32 *max_ddr_clk);
	int (*set_gpu_cfg)(struct mtgpu_device *mtdev, struct gpu_cfg_req *gpu_req);
	int (*get_gpu_cfg)(struct mtgpu_device *mtdev, struct gpu_cfg_info *gpu_info);
	int (*get_vpu_core_info)(struct mtgpu_device *mtdev, u32 *vpu_core_info);
	int (*reset_subsystem)(struct mtgpu_device *mtdev, enum mtgpu_subsys_id ss_id,
			       u64 bit_mask, enum reset_type cmd);
	int (*set_gpu_eata_cfg)(struct mtgpu_device *mtdev,
				struct gpu_eata_cfg_info *eata_cfg_info);
	s32 (*get_clk_id)(enum mtgpu_clk_domain domain, u16 sub_id);
	int (*get_fw_versions)(struct mtgpu_device *mtdev, struct mtgpu_fw_versions *versions);
	int (*get_freq)(struct mtgpu_device *mtdev, u32 clock_type, u64 *freq);
	int (*get_fan_count)(struct mtgpu_device *mtdev, u32 *count);
	int (*get_fan_speed)(struct mtgpu_device *mtdev, u32 fmt, u32 index, u32 *speed);
	int (*get_temp)(struct mtgpu_device *mtdev, u32 pvt_type, u32 *temp);
	int (*get_power)(struct mtgpu_device *mtdev, u32 power_domain, u32 *power);
	int (*get_voltage)(struct mtgpu_device *mtdev, u32 sensor_index, u32 *voltage);
	int (*get_ddr_temp)(struct mtgpu_device *mtdev, u32 *temperature);
	int (*get_ddr_edc_error)(struct mtgpu_device *mtdev,
				 struct ddr_edc_error_counter *ddr_edc_error);
	int (*get_temp_limit)(struct mtgpu_device *mtdev,
			      struct ipc_msg_thermal_limit_info *temp_limit);
	int (*get_power_limit)(struct mtgpu_device *mtdev, u32 pl_type, u32 *power_limit);
};

int mtgpu_smc_get_fw_versions(struct device *dev, struct mtgpu_fw_versions *versions);
int mtgpu_smc_get_freq(struct device *dev, u32 clock_type, u64 *freq);
int mtgpu_smc_get_fan_count(struct device *dev, u32 *count);
int mtgpu_smc_get_fan_speed(struct device *dev, u32 fmt, u32 index, u32 *speed);
int mtgpu_smc_get_temp(struct device *dev, u32 pvt_type, u32 *temp);
int mtgpu_smc_get_power(struct device *dev, u32 power_domain, u32 *power);
int mtgpu_smc_get_voltage(struct device *dev, u32 sensor_index, u32 *voltage);
int mtgpu_smc_get_ddr_temp(struct device *dev, u32 *temperature);
int mtgpu_smc_get_ddr_edc_error(struct device *dev,
				struct ddr_edc_error_counter *ddr_edc_error);
int mtgpu_smc_get_temp_limit(struct device *dev,
			     struct ipc_msg_thermal_limit_info *temp_limit);
int mtgpu_smc_get_power_limit(struct device *dev, u32 pl_type, u32 *pl_limit);
int mtgpu_smc_set_pstate(struct device *dev, enum pstate_lvl pstate_lvl);
int mtgpu_smc_get_pstate(struct mtgpu_device *mtdev);
int mtgpu_smc_get_pstate_entry_info(struct device *dev,
				    struct pstate_entry_info *entry);
int mtgpu_smc_get_rtos_version(struct device *dev);
int mtgpu_smc_get_dp_phy_cfg(struct device *dev, void **dp_phy_cfg);
bool mtgpu_smc_get_secure_bit(struct device *dev);
int mtgpu_smc_get_board_config(struct device *dev);
int mtgpu_smc_realtime_sync(struct device *dev);
void mtgpu_smc_release_board_config(struct device *dev);
int mtgpu_smc_set_gpu_cfg(struct device *dev, struct gpu_cfg_req *gpu_req);
int mtgpu_smc_get_gpu_cfg(struct device *dev, struct gpu_cfg_info *gpu_info);
int mtgpu_smc_get_vpu_core_info(struct device *dev, u32 *vpu_core_info);
int mtgpu_smc_set_gpu_eata_cfg(struct device *dev, struct gpu_eata_cfg_info *eata_cfg_info);
int mtgpu_smc_write_reg32(struct device *dev, u64 reg_addr, u32 value);
int mtgpu_smc_read_reg32(struct device *dev, u64 reg_addr, u32 *value);
int mtgpu_smc_read_dsp_register(struct device *dev, enum dsp_register reg, u32 *value);
int mtgpu_smc_write_dsp_register(struct device *dev, enum dsp_register reg, u32 value);
int mtgpu_smc_write_secure_reg8(struct device *dev, u16 region_id, u32 reg, u8 value);
int mtgpu_smc_write_secure_reg16(struct device *dev, u16 region_id, u32 reg, u16 value);
int mtgpu_smc_write_secure_reg32(struct device *dev, u16 region_id, u32 reg, u32 value);
int mtgpu_smc_write_secure_reg64(struct device *dev, u16 region_id, u32 reg, u64 value);
int mtgpu_smc_read_secure_reg8(struct device *dev, u16 region_id, u32 reg, u8 *value);
int mtgpu_smc_read_secure_reg16(struct device *dev, u16 region_id, u32 reg, u16 *value);
int mtgpu_smc_read_secure_reg32(struct device *dev, u16 region_id, u32 reg, u32 *value);
int mtgpu_smc_read_secure_reg64(struct device *dev, u16 region_id, u32 reg, u64 *value);
int mtgpu_smc_rb_switch_restore(struct mtgpu_device *mtdev);
int mtgpu_smc_init(struct mtgpu_device *mtdev);
void mtgpu_smc_exit(struct mtgpu_device *mtdev);

#endif /* __MTGPU_SMC_H__ */
