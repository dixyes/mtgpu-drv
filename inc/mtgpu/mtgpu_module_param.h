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
extern int enable_mtlink;
extern unsigned long mtlink_timer_expires;
extern unsigned char mtgpu_pstate_mode;
extern unsigned int mtgpu_page_size;
extern int mtgpu_vpu_mode;
extern int disable_vpu;
extern bool disable_driver;
extern int irq_vector_cnt;
extern bool disable_pcie_link_monitor;

#endif /*__MTGPU_MODULE_PARAM_H__*/
