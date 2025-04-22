/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_MODULE_PARAM_H__
#define __MTGPU_MODULE_PARAM_H__

extern int mtgpu_cnt;
extern unsigned long mtgpu_vram_size_total;
extern unsigned long mtgpu_pb_total_size;
extern unsigned long mtgpu_smc_mem_size;
extern unsigned long mtgpu_reserved_mem_size;
extern int mtgpu_resize_enable;
extern char *display;
extern bool display_bringup_en;
extern unsigned long mtgpu_cursor_size;
extern int enable_sriov;
extern bool force_cpu_copy;
extern int mtgpu_dma_debug;
extern int dma_desc_sysmem_enable;
extern int mtgpu_driver_mode;
extern unsigned long mtvpu_reserved_mem_size;
extern int mtgpu_ipc_debug;
extern unsigned long mtgpu_ipc_timeout_ms;
extern int mtgpu_ipc_tty_support;
extern int mtgpu_fec_enable;
extern bool fec_display_enable;
extern char *fec_kernel_params;
extern unsigned long fec_image_base;
extern unsigned long fec_image_size;
extern int enable_mtlink;
extern unsigned long mtlink_timer_expires;
extern int mtlink_topo_type;
extern bool mtlink_disable_hwr;
extern int enable_event_report;
extern unsigned char mtgpu_pstate_mode;
extern unsigned int mtgpu_page_size;
extern int enable_vpu_test;
extern int disable_vpu;
extern bool disable_gpu;
extern int vpu_group_max;
extern bool disable_driver;
extern bool disable_watchdog;
extern unsigned int watchdog_timeout_ms;
extern int irq_vector_cnt;
extern bool disable_pcie_link_monitor;
extern bool enable_reserved_memory;
extern bool bypass_igpu_smmu;
extern bool enable_gpu_dvfs;
extern bool enable_rpm;
extern unsigned long mtgpu_rpm_suspend_delay_ms;
extern int mtgpu_drm_major;
extern int mtgpu_sched_mode;
extern bool mtgpu_sched_ccbq_optimize;
extern bool mtgpu_sched_early_submit;
extern int mtgpu_sched_job_hang_limit_ms;
extern int mtgpu_shadow_sem_sysmem_enable;
extern bool mtgpu_enable_poison;
extern int rb_switch_enable;
extern unsigned int disable_ipc_bitmask;
extern int ignore_mpc_config;
extern int mtgpu_sched_host_policy;
extern bool mtgpu_riscv_fw_enabled;
extern int mtgpu_d2d_wait_timeout;
extern int mtgpu_hwperf_debug;
extern unsigned long long mtgpu_hash_vec_array[];
extern int mtgpu_hash_vec_array_size;
extern bool mtgpu_enable_sysmem_stats;
extern int rm_feature_level;

#if (RGX_NUM_OS_SUPPORTED > 1)
extern int mtgpu_vgpu_scheduling_policy;
extern int mtgpu_vgpu_time_sliced_value;
extern unsigned long mtgpu_vgpu_host_mem_size;
extern bool mtgpu_vgpu_force_mmio_in_4g;
extern bool mtgpu_load_windows_firmware;
extern bool mtgpu_vgpu_dyn_mpc_mode;
extern bool mtgpu_vgpu_full_vgpu_types;
extern int vgpu_mm_mapping_mode;
extern int vgpu_iommu_mode;
extern int mtgpu_vdma_enable;
extern unsigned long mtgpu_win_fw_context_switch_value;
extern bool vgpu_qos;
extern bool vgpu_1g_support_4k;
extern bool vgpu_types_tune;
extern bool vgpu_host_trigger_hwr;
extern int vgpu_compat_check_mode;
extern bool vgpu_time_corr;
extern int vgpu_hotplug_upgrade_vram_mode;
extern int vgpu_upgrade_mode;
extern bool vgpu_fw_hot_upgrade;
extern bool vgpu_allow_pull_driver;
extern u64 vgpu_guest_pkg_tag;
#endif

#endif /*__MTGPU_MODULE_PARAM_H__*/
