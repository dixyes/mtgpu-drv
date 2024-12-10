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
#include <linux/errno.h>

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
int mtgpu_drm_major = 2;
#else
int mtgpu_drm_major = 1;
#endif
module_param(mtgpu_drm_major, int, 0444);
MODULE_PARM_DESC(mtgpu_drm_major, "1 - ddk1.0, 2 - ddk2.0. The default value is 1");

#if (RGX_NUM_OS_SUPPORTED > 1)
long mtgpu_driver_mode = MTGPU_DRIVER_MODE_HOST;
#else
long mtgpu_driver_mode = MTGPU_DRIVER_MODE_NATIVE;
#endif

static int driver_mode_set(const char *arg, const struct kernel_param *kp)
{
	int ret = 0;
	long new_driver_mode;

	ret = kstrtol(arg, 10, &new_driver_mode);
	if (ret) {
		pr_err("mtgpu: driver mode setting error(kstrtol)\n");
		goto exit;
	}

#if (RGX_NUM_OS_SUPPORTED > 1)
	if (!(new_driver_mode == MTGPU_DRIVER_MODE_HOST ||
	      new_driver_mode == MTGPU_DRIVER_MODE_GUEST)) {
		pr_err("mtgpu: mtgpu_driver_mode(%ld), setting error in this mode(host = 0,  guest = 1)\n",
		       new_driver_mode);
		ret = -EINVAL;
		goto exit;
	}
#else
	if (new_driver_mode != MTGPU_DRIVER_MODE_NATIVE) {
		pr_err("mtgpu: mtgpu_driver_mode(%ld), setting error in this mode(native = -1)\n",
		       new_driver_mode);
		ret = -EINVAL;
		goto exit;
	}
#endif

	mtgpu_driver_mode = new_driver_mode;
	/* Ensure mtgpu_driver_mode is flushed */
	smp_mb();

exit:
	return ret;
}

static const struct kernel_param_ops driver_mode_ops = {
	.set = driver_mode_set,
	.get = param_get_int,
};

module_param_cb(mtgpu_driver_mode, &driver_mode_ops, &mtgpu_driver_mode, 0444);
MODULE_PARM_DESC(mtgpu_driver_mode,
		 "mtgpu driver mode (native = -1, host = 0, guest = 1)");

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

char *fec_kernel_params;
module_param(fec_kernel_params, charp, 0444);
MODULE_PARM_DESC(fec_kernel_params, "mtgpu fec kernel extra params");

unsigned long fec_image_base;
module_param(fec_image_base, ulong, 0444);
MODULE_PARM_DESC(fec_image_base,
		 "mtgpu fec image base address, larger than vram size is relocated in host mem region");

unsigned long fec_image_size;
module_param(fec_image_size, ulong, 0444);
MODULE_PARM_DESC(fec_image_size, "mtgpu fec image reserved size");

int enable_mtlink;
module_param(enable_mtlink, int, 0444);
MODULE_PARM_DESC(enable_mtlink, "1:enable mtlink, 0:disable mtlink");

unsigned long mtlink_timer_expires = 25000;
module_param(mtlink_timer_expires, ulong, 0444);
MODULE_PARM_DESC(mtlink_timer_expires,
		 "timer expires(ms) from pcie probe to mtlink init, default 25000ms");

int mtlink_topo_type;
module_param(mtlink_topo_type, int, 0444);
MODULE_PARM_DESC(mtlink_topo_type,
		 "0:default, normal topology, 1:eight-card fc topology, 2:two groups of four-card fc topology");

int enable_event_report = 1;
module_param(enable_event_report, int, 0444);
MODULE_PARM_DESC(enable_event_report,
		 "1:default, enable mtgpu_event_report, 0:disable mtgpu_event_report");

/**
 * pstate mode
 * To control mtgpu pstate mode (0 = disable, 1 = enable).
 * The default is 1 (enabled).
 */
unsigned char mtgpu_pstate_mode = PSTATE_ENABLED;
module_param(mtgpu_pstate_mode, byte, 0444);
MODULE_PARM_DESC(mtgpu_pstate_mode,
		 "mtgpu pstate mode: 0 = disable, 1 = enable");

int enable_vpu_test = 0;
module_param(enable_vpu_test, int, 0444);
MODULE_PARM_DESC(enable_vpu_test, "0: default, 1: enable vpu test mode");

#if defined(CONFIG_VPS)
int disable_vpu = 1;
#else
int disable_vpu = 0;
#endif
module_param(disable_vpu, int, 0444);
MODULE_PARM_DESC(disable_vpu, "0: default, 1: disable vpu module.");

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

bool enable_reserved_memory = true;
module_param(enable_reserved_memory, bool, 0444);
MODULE_PARM_DESC(enable_reserved_memory, "enable reserved memory for igpu, default 0.");

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

int vgpu_mm_mapping_mode = 2;
module_param(vgpu_mm_mapping_mode, int, 0444);
MODULE_PARM_DESC(vgpu_mm_mapping_mode,
	"vGPU system memory mapping mode(0: non_linear, 1: linear, 2: linear_prealloc(default))");

int vgpu_iommu_mode = 2;
module_param(vgpu_iommu_mode, int, 0444);
MODULE_PARM_DESC(vgpu_iommu_mode,
		 "vGPU iommu mode(0: force disable, 1: auto, 2: follow system(default))");

#if defined(VDI_PLATFORM_SANGFOR)
#define MTGPU_VGPU_VDMA_DEFAULT  1
#else
#define MTGPU_VGPU_VDMA_DEFAULT  0
#endif

int mtgpu_vdma_enable = MTGPU_VGPU_VDMA_DEFAULT;
module_param(mtgpu_vdma_enable, int, 0444);
MODULE_PARM_DESC(mtgpu_vdma_enable, "vdma enable mode (0: not enable, 1: enable)");

unsigned long mtgpu_win_fw_context_switch_value = 0x1E;
module_param(mtgpu_win_fw_context_switch_value, ulong, 0444);
MODULE_PARM_DESC(mtgpu_win_fw_context_switch_value,
	"windows firmware DM(TDM, TA, 3D, CDM) context switch. Each bit represents each DM (0: not enable, 1: enable)");

int mtgpu_vgpu_dyn_mpc_mode = 1;
module_param(mtgpu_vgpu_dyn_mpc_mode, int, 0444);
MODULE_PARM_DESC(mtgpu_vgpu_dyn_mpc_mode, "vgpu dynamic change mpc mode (0: disable, 1: enable(default))");

bool vgpu_qos = 1;
module_param(vgpu_qos, bool, 0444);
MODULE_PARM_DESC(vgpu_qos,"vGPU PCIe bandwidth QoS(0: disable, 1: enable)");

bool vgpu_1g_support_4k = 1;
module_param(vgpu_1g_support_4k, bool, 0444);
MODULE_PARM_DESC(vgpu_1g_support_4k,"1G VGPU support 4K resolution(0: disable, 1: enable)");

#if defined(VGPU_COMPAT_CHECK_MODE_VERSION_LIST_FORCED)
/**
 * For CI release versions, this value is
 * immutable by default to prevent user modifications.
 *
 * @note This Mode version list(1) is not available on the develop branch.
 */
int vgpu_compat_check_mode = 1;
#else
int vgpu_compat_check_mode = 2;
module_param(vgpu_compat_check_mode, int, 0444);
MODULE_PARM_DESC(vgpu_compat_check_mode,
		 "vGPU compatibility check mode(0: disable, 1: version list, 2: strictly match)");
#endif

#endif
