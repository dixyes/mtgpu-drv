/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

/*
 * This is a device driver for the mtgpu framework. It creates platform
 * devices inside the MT GPU, and exports functions to manage the
 * shared interrupt handling
 */

#include <linux/moduleparam.h>

#include "mtgpu_defs.h"

/* TODO: Currently assume the MTGPU default video ram size is 1G.
 * It should get the video ram size from SMC in the future for each
 * mtgpu graphics card
 */
unsigned long mtgpu_vram_size_total = MTGPU_VRAM_DEFAULT_SIZE;
module_param(mtgpu_vram_size_total, ulong, 0444);
MODULE_PARM_DESC(mtgpu_vram_size_total,
		 "mtgpu video memory total size, default to 1GB ");

unsigned long mtgpu_smc_mem_size = MTGPU_SMC_MEM_SIZE;
module_param(mtgpu_smc_mem_size, ulong, 0444);
MODULE_PARM_DESC(mtgpu_smc_mem_size,
		 "mtgpu smc memory total size, default to 2M ");

unsigned long mtgpu_reserved_mem_size = MTGPU_RESERVED_MEM_SIZE;
module_param(mtgpu_reserved_mem_size, ulong, 0444);
MODULE_PARM_DESC(mtgpu_reserved_mem_size,
		 "mtgpu reserved memory size for smc/dma/fw/mmu etc, to meet system requirements, default to 256MB ");

/*
 * mtgpu_resize_enable is used for the option to control the resize bar
 * functionality is enabled or disabled.
 */
int mtgpu_resize_enable = 1;
module_param(mtgpu_resize_enable, int, 0444);
MODULE_PARM_DESC(mtgpu_resize_enable,
		 "mtgpu resize function enable(1)/disable(0) ");

#if defined(CONFIG_VPS)
char *display = "dummy";
#elif (RGX_NUM_OS_SUPPORTED > 1)
char *display = "none";
#else
char *display = "mt";
#endif
module_param(display, charp, 0444);
MODULE_PARM_DESC(display, " <dummy>, <mt> The default value is mt");

unsigned long mtgpu_cursor_size = MTGPU_MAX_CURSOR_SIZE;
module_param(mtgpu_cursor_size, ulong, 0444);
MODULE_PARM_DESC(mtgpu_cursor_size, "max cursor size in pixel");

/* Enable_sriov is used for the sriov capability. */
int enable_sriov = MTGPU_DISABLE_SRIOV;
module_param(enable_sriov, int, 0444);
MODULE_PARM_DESC(enable_sriov, "enable sriov, default to 0 is disable ");

bool force_cpu_copy;
module_param(force_cpu_copy, bool, 0644);
MODULE_PARM_DESC(force_cpu_copy,
		 "dma transfer use cpu copy force, enable(1)/disable(0) ");

int mtgpu_dma_debug;
module_param(mtgpu_dma_debug, int, 0444);
MODULE_PARM_DESC(mtgpu_dma_debug,
		 "mtgpu udma and hdma debug information enable(1)/disable(0) ");

#if (RGX_NUM_OS_SUPPORTED > 1)
int mtgpu_driver_mode = MTGPU_DRIVER_MODE_HOST;
module_param(mtgpu_driver_mode, int, 0444);
MODULE_PARM_DESC(mtgpu_driver_mode,
		 "mtgpu driver mode (native = -1, host = 0, guest = 1 )");
#else
int mtgpu_driver_mode = MTGPU_DRIVER_MODE_NATIVE;
#endif

unsigned long mtvpu_reserved_mem_size;
module_param(mtvpu_reserved_mem_size, ulong, 0444);
MODULE_PARM_DESC(mtvpu_reserved_mem_size, "reserved gpu memory for VPU");

int mtgpu_ipc_debug;
module_param(mtgpu_ipc_debug, int, 0664);
MODULE_PARM_DESC(mtgpu_ipc_debug,
		 "mtgpu ipc debug information enable(1)/disable(0)");

unsigned long mtgpu_ipc_timeout_ms = 1000 * 60;
module_param(mtgpu_ipc_timeout_ms, ulong, 0664);
MODULE_PARM_DESC(mtgpu_ipc_timeout_ms,
		 "mtgpu ipc timeout(unit:ms, default:1 minute/60000 ms)");

int mtgpu_ipc_tty_support;
module_param(mtgpu_ipc_tty_support, int, 0444);
MODULE_PARM_DESC(mtgpu_ipc_tty_support,
		 "mtgpu ipc tty console enable(1)/disable(0)");

bool mtgpu_fec_enable;
module_param(mtgpu_fec_enable, bool, 0444);
MODULE_PARM_DESC(mtgpu_fec_enable, "mtgpu fec enable(1)/disable(0)");

int enable_mtlink;
module_param(enable_mtlink, int, 0444);
MODULE_PARM_DESC(enable_mtlink, "1:enable mtlink, 0:disable mtlink");

unsigned long mtlink_timer_expires = 5000;
module_param(mtlink_timer_expires, ulong, 0444);
MODULE_PARM_DESC(mtlink_timer_expires,
		 "timer expires(ms) from pcie probe to mtlink init, default 5000ms");

/**
 * pstate mode
 * To control mtgpu pstate mode (0 = disable, 1 = enable).
 * The default is 1 (enabled).
 */
unsigned char mtgpu_pstate_mode = PSTATE_ENABLED;
module_param(mtgpu_pstate_mode, byte, 0444);
MODULE_PARM_DESC(mtgpu_pstate_mode,
		 "mtgpu pstate mode: 0 = disable, 1 = enable");

#if defined(CONFIG_VPS)
int mtgpu_vpu_mode = MTGPU_VPU_MODE_DISABLE;
#else
int mtgpu_vpu_mode = MTGPU_VPU_MODE_DEFAULT;
#endif
module_param(mtgpu_vpu_mode, int, 0444);
MODULE_PARM_DESC(mtgpu_vpu_mode, "0: default, 1: test mode, 2: disable all vpu modules.");

int disable_vpu;
module_param(disable_vpu, int, 0444);
MODULE_PARM_DESC(disable_vpu, "0: default, 1: disable vpu module.");

int disable_jpu;
module_param(disable_jpu, int, 0444);
MODULE_PARM_DESC(disable_jpu, "0: default, 1: disable jpu module.");

unsigned int mtgpu_page_size = 0;
module_param(mtgpu_page_size, uint, 0444);
MODULE_PARM_DESC(mtgpu_page_size, "gpu page size, default to 0.");

bool disable_driver;
module_param(disable_driver, bool, 0444);
MODULE_PARM_DESC(disable_driver, "disable mtgpu driver, default 0.");

int vpu_group_max;
module_param(vpu_group_max, int, 0444);
MODULE_PARM_DESC(vpu_group_max, "0: default, 1: there are 6 vpu segments.");

int irq_vector_cnt = 8;
module_param(irq_vector_cnt, int, 0444);
MODULE_PARM_DESC(irq_vector_cnt, "manually modify the number of interrupt vectors supported");

bool disable_pcie_link_monitor;
module_param(disable_pcie_link_monitor, bool, 0444);
MODULE_PARM_DESC(disable_pcie_link_monitor, "disable pcie link monitor, default 0.");

#if (RGX_NUM_OS_SUPPORTED > 1)
int mtgpu_vgpu_scheduling_policy;
module_param(mtgpu_vgpu_scheduling_policy, int, 0444);
MODULE_PARM_DESC(mtgpu_vgpu_scheduling_policy,
		 "mtgpu vgpu scheduling policy(best effort = 0, time sliced = 1)");

int mtgpu_vgpu_time_sliced_value = 20;
module_param(mtgpu_vgpu_time_sliced_value, int, 0444);
MODULE_PARM_DESC(mtgpu_vgpu_time_sliced_value,
		 "mtgpu vgpu time sliced value in millisecond");

unsigned long mtgpu_vgpu_host_mem_size = 0x80000000;
module_param(mtgpu_vgpu_host_mem_size, ulong, 0444);
MODULE_PARM_DESC(mtgpu_vgpu_host_mem_size,
		 "mtgpu vgpu host os vRAM memory size: default to 2G");

bool mtgpu_vgpu_force_mmio_in_4g;
module_param(mtgpu_vgpu_force_mmio_in_4g, bool, 0444);
MODULE_PARM_DESC(mtgpu_vgpu_force_mmio_in_4g,
		 "force mmio space allocated in 4g range (disable = 0, enable = 1 )");

bool mtgpu_load_windows_firmware = true;
module_param(mtgpu_load_windows_firmware, bool, 0444);
MODULE_PARM_DESC(mtgpu_load_windows_firmware,
	"0 - load linux fw, 1 - load windows fw. The default value is 1");

int vgpu_mm_mapping_mode = 1;
module_param(vgpu_mm_mapping_mode, int, 0444);
MODULE_PARM_DESC(vgpu_mm_mapping_mode,
	"vGPU system memory mapping mode(0: non_linear, 1: linear(default), 2: linear_prealloc)");

#if defined(VDI_PLATFORM_SANGFOR)
#define MTGPU_VGPU_VDMA_DEFAULT  1
#else
#define MTGPU_VGPU_VDMA_DEFAULT  0
#endif

int mtgpu_vdma_enable = MTGPU_VGPU_VDMA_DEFAULT;
module_param(mtgpu_vdma_enable, int, 0444);
MODULE_PARM_DESC(mtgpu_vdma_enable, "vdma enable mode (0: not enable, 1: enable)");
#endif
