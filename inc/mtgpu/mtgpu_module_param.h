/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_MODULE_PARAM_H__
#define __MTGPU_MODULE_PARAM_H__

extern unsigned long mtgpu_vram_size_total;
extern unsigned long mtgpu_smc_mem_size;
extern unsigned long mtgpu_reserved_mem_size;
extern int mtgpu_resize_enable;
extern char *display;
extern unsigned long mtgpu_cursor_size;
extern int enable_sriov;
extern bool force_cpu_copy;
extern int mtgpu_dma_debug;
extern int mtgpu_driver_mode;
extern unsigned long mtvpu_reserved_mem_size;
extern int mtgpu_ipc_debug;
extern unsigned long mtgpu_ipc_timeout_ms;
extern int mtgpu_ipc_tty_support;
extern bool mtgpu_fec_enable;
extern char *fec_kernel_params;
extern int enable_mtlink;
extern unsigned long mtlink_timer_expires;
extern int mtlink_topo_type;
extern int enable_event_report;
extern unsigned char mtgpu_pstate_mode;
extern unsigned int mtgpu_page_size;
extern int enable_vpu_test;
extern int disable_vpu;
extern int vpu_group_max;
extern bool disable_driver;
extern int irq_vector_cnt;
extern bool disable_pcie_link_monitor;
extern int mtgpu_drm_major;
#if (RGX_NUM_OS_SUPPORTED > 1)
extern int mtgpu_vgpu_scheduling_policy;
extern int mtgpu_vgpu_time_sliced_value;
extern unsigned long mtgpu_vgpu_host_mem_size;
extern bool mtgpu_vgpu_force_mmio_in_4g;
extern bool mtgpu_load_windows_firmware;
extern bool mtgpu_vgpu_full_vgpu_types;
extern bool mtgpu_vgpu_enable_share_vpu_mem;
extern int vgpu_mm_mapping_mode;
extern int vgpu_iommu_mode;
extern int mtgpu_vdma_enable;
extern unsigned long mtgpu_win_fw_context_switch_value;
#endif

#endif /*__MTGPU_MODULE_PARAM_H__*/
