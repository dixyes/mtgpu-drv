/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/module.h>
#include <linux/component.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/acpi.h>
#if defined(OS_LINUX_DEVICE_BUS_H_EXIT)
#include <linux/device/bus.h>
#endif
#include <linux/iommu.h>

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_ioctl.h>
#endif
#if defined(OS_DRM_DRM_APERTURE_H_EXIST)
#include <drm/drm_aperture.h>
#endif
#include <linux/dma-mapping.h>
#include <drm/drm_of.h>
#include <drm/drm_drv.h>
#include <drm/drm_vblank.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_blend.h>
#if defined(OS_DRM_DRM_PROBE_HELPER_H_EXIST)
#include <drm/drm_probe_helper.h>
#endif
#include <drm/drm_fb_helper.h>
#if defined(OS_DRM_DRM_FBDEV_GENERIC_H_EXIST)
#include <drm/drm_fbdev_generic.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
#include <drm/drm_fbdev_shmem.h>
#endif // LINUX_VERSION
#if defined(OS_DRM_DRM_SELF_REFRESH_HELPER_H_EXIST)
#include <drm/drm_self_refresh_helper.h>
#endif
#include <drm/drm_blend.h>

#include "mtgpu_drv.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drm_gem.h"
#include "mtgpu_drm_atomic.h"
#include "mtgpu_drm_internal.h"
#include "pvr_drv.h"
#include "mtgpu_vpu.h"
#include "mtgpu_module_param.h"

#define DRIVER_DESC	"Moorethreads DRM/KMS Driver"
#define DRIVER_DATE	"20210320"
#define DRIVER_MAJOR	1
#define DRIVER_MINOR	0

enum mtgpu_of_component_type {
	MTGPU_COMP_TYPE_DC,
	MTGPU_COMP_TYPE_DP,
	MTGPU_COMP_TYPE_VIDEO,
};

/* disable_fbdev is used for kick the fb node. */
#if (RGX_NUM_OS_SUPPORTED > 1)
static bool disable_fbdev = true;
#else
static bool disable_fbdev;
#endif
module_param(disable_fbdev, bool, 0444);
MODULE_PARM_DESC(disable_fbdev, "0 - enable fbdev, 1 - disable fbdev. The default value is 0");

/**
 * display_debug - A master switch is used to control debug.
 * All display components print info, warning, error by default,
 * each Byte controls one component, as follows:
 * |<-- 24bits -->|<-- 8bits -->|<-- 8bits -->|<-- 8bits -->|<-- 8bits -->|<-- 8bits -->|
 * |   reserved   |  dsc_debug  |global_debug | dispc_debug | hdmi_debug  |  dp_debug   |
 */
ulong display_debug = 0x0707070707;
module_param(display_debug, ulong, 0600);
MODULE_PARM_DESC(display_debug,
		 " H->L: dsc[8] global[8] dispc[8] hdmi[8] dp[8]. Default: 0x0707070707.");

bool dsc_enable = true;
module_param(dsc_enable, bool, 0444);
MODULE_PARM_DESC(dsc_enable, "0 - disable dsc, 1 - enable dsc. The default value is 1");

static int mtgpu_kick_out_firmware_fb(resource_size_t base, resource_size_t size);

static void mtgpu_atomic_helper_commit_tail_rpm(struct drm_atomic_state *old_state)
{
	struct drm_device *dev = old_state->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	u16 device_id = pdev->device & 0xff00;

	if (device_id == DEVICE_ID_SUDI104) {
		drm_atomic_helper_commit_modeset_disables(dev, old_state);
		drm_atomic_helper_commit_modeset_enables(dev, old_state);
	} else {
		mtgpu_atomic_helper_encoder_detect(dev, old_state);
		mtgpu_atomic_helper_commit_modeset_disables(dev, old_state);
		mtgpu_atomic_helper_encoder_detect(dev, old_state);
		mtgpu_atomic_helper_commit_modeset_enables(dev, old_state);
	}

	drm_atomic_helper_commit_planes(dev, old_state,
					DRM_PLANE_COMMIT_ACTIVE_ONLY);
#if defined(OS_FUNC_DRM_ATOMIC_HELPER_FAKE_VBLANK_EXIST)
	drm_atomic_helper_fake_vblank(old_state);
#endif
	drm_atomic_helper_commit_hw_done(old_state);

	drm_atomic_helper_cleanup_planes(dev, old_state);
}

static int mtgpu_atomic_helper_check(struct drm_device *dev,
				     struct drm_atomic_state *state)
{
	int ret;

	ret = drm_atomic_helper_check_modeset(dev, state);
	if (ret)
		return ret;

	if (dev->mode_config.normalize_zpos) {
		ret = drm_atomic_normalize_zpos(dev, state);
		if (ret)
			return ret;
	}

	ret = drm_atomic_helper_check_planes(dev, state);
	if (ret)
		return ret;

	if (state->legacy_cursor_update)
		state->async_update = !mtgpu_atomic_helper_async_check(dev, state);

#if defined(OS_DRM_DRM_SELF_REFRESH_HELPER_H_EXIST)
	drm_self_refresh_helper_alter_state(state);
#endif
	return ret;
}

static const struct drm_mode_config_helper_funcs mtgpu_mode_config_helpers = {
	.atomic_commit_tail = mtgpu_atomic_helper_commit_tail_rpm,
};

static const struct drm_mode_config_funcs mtgpu_mode_config_funcs = {
	.fb_create = drm_gem_fb_create,
	.atomic_check = mtgpu_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static int mtgpu_mode_config_init(struct drm_device *drm)
{
	drm_mode_config_init(drm);

	drm->mode_config.min_width = 0;
	drm->mode_config.min_height = 0;
	drm->mode_config.max_width = 32768;
	drm->mode_config.max_height = 32768;
#if defined(OS_STRUCT_DRM_MODE_CONFIG_HAS_ALLOW_FB_MODIFIERS)
	drm->mode_config.allow_fb_modifiers = true;
#endif
	drm->mode_config.normalize_zpos = true;
	drm->mode_config.funcs = &mtgpu_mode_config_funcs;
	drm->mode_config.helper_private = &mtgpu_mode_config_helpers;

	return 0;
}

static int mtgpu_drm_open(struct drm_device *ddev, struct drm_file *dfile)
{
	int err;

	if (!dfile->driver_priv) {
		dfile->driver_priv = kzalloc(sizeof(struct mtgpu_drm_file), GFP_KERNEL);
		if (!dfile->driver_priv) {
			err = -ENOMEM;
			goto out;
		}
	}

	err = pvr_drm_open(ddev, dfile);
	if (err) {
		err = -EFAULT;
		goto fail_pvr_open;
	}

	err = mtvpu_drm_open(ddev, dfile);
	if (err) {
		err = -EFAULT;
		goto fail_vpu_open;
	}

	goto out;

fail_vpu_open:
	pvr_drm_release(ddev, dfile);
fail_pvr_open:
	kfree(dfile->driver_priv);
out:
	return err;
}

static void mtgpu_drm_release(struct drm_device *ddev, struct drm_file *dfile)
{
	mtvpu_drm_release(ddev, dfile);

	/* drm_file driver_priv is freed in this call */
	pvr_drm_release(ddev, dfile);
}

const struct file_operations mtgpu_drm_driver_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.release	= drm_release,
	.unlocked_ioctl	= drm_ioctl,
	.compat_ioctl	= drm_compat_ioctl,
	.poll		= drm_poll,
	.read		= drm_read,
	.llseek		= noop_llseek,
	.mmap		= mtgpu_mmap,
#ifdef FOP_UNSIGNED_OFFSET
	.fop_flags = FOP_UNSIGNED_OFFSET,
#endif // FOP_UNSIGNED_OFFSET
};

static struct drm_driver mtgpu_drm_driver = {
#if defined(OS_ENUM_DRIVER_PRIME_EXIST)
	.driver_features		= DRIVER_MODESET | DRIVER_GEM | DRIVER_ATOMIC |
					  DRIVER_RENDER | DRIVER_PRIME,
#else
	.driver_features		= DRIVER_MODESET | DRIVER_GEM | DRIVER_ATOMIC |
					  DRIVER_RENDER,
#endif

	.open				= mtgpu_drm_open,
	.postclose			= mtgpu_drm_release,

	.ioctls				= pvr_drm_ioctls,
	.num_ioctls			= ARRAY_SIZE(pvr_drm_ioctls),

	.fops				= &mtgpu_drm_driver_fops,
	.dumb_create			= mtgpu_gem_dumb_create,

#ifdef OS_FUNC_DRM_GEM_PRIME_HANDLE_TO_FD_EXIST
	.prime_handle_to_fd		= drm_gem_prime_handle_to_fd,
#endif
#ifdef OS_FUNC_DRM_GEM_PRIME_FD_TO_HANDLE_EXIST
	.prime_fd_to_handle		= drm_gem_prime_fd_to_handle,
#endif
	.gem_prime_import		= mtgpu_gem_prime_import,
	.gem_prime_import_sg_table	= mtgpu_gem_prime_import_sg_table,
#if defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
	.gem_vm_ops			= &mtgpu_gem_vm_ops,
	.gem_free_object_unlocked	= mtgpu_gem_object_free,
	.gem_prime_export		= mtgpu_gem_prime_export,
	.gem_prime_vmap			= mtgpu_gem_prime_vmap,
	.gem_prime_vunmap		= mtgpu_gem_prime_vunmap,
	.gem_prime_get_sg_table		= mtgpu_gem_prime_get_sg_table,
#endif

	.name				= DRIVER_NAME,
	.desc				= DRIVER_DESC,
	.date				= DRIVER_DATE,
	.major				= DRIVER_MAJOR,
	.minor				= DRIVER_MINOR,
};

static int mtgpu_component_bind(struct device *dev)
{
	struct mtgpu_drm_platform_data *pdata = dev_get_platdata(dev);
	struct drm_device *drm;
	struct mtgpu_drm_private *private;
	int ret;

	mtgpu_drm_driver.major = mtgpu_drm_major;

	drm = drm_dev_alloc(&mtgpu_drm_driver, dev->parent);
	if (IS_ERR(drm))
		return PTR_ERR(drm);

	dev_set_drvdata(dev, drm);
	dma_set_mask(dev, DMA_BIT_MASK(32));
#if defined(OS_STRUCT_DRM_DEVICE_HAS_PDEV)
	drm->pdev = to_pci_dev(dev->parent);
#endif

	private = kzalloc(sizeof(*private), GFP_KERNEL);
	if (!private) {
		ret = -ENOMEM;
		goto err_mode_config_cleanup;
	}
	drm->dev_private = private;

	mtgpu_mode_config_init(drm);

	ret = component_bind_all(dev, drm);
	if (ret) {
		DRM_ERROR("Failed to bind other components (ret=%d)\n", ret);
		goto err_free;
	}

	ret = drm_vblank_init(drm, drm->mode_config.num_crtc);
	if (ret) {
		DRM_ERROR("failed to complete vblank init (ret=%d)\n", ret);
		goto err_unbind_all;
	}

#if defined(OS_STRUCT_DRM_DEVICE_HAS_IRQ_ENABLED)
	drm->irq_enabled = true;
#endif

	drm_mode_config_reset(drm);

	drm_kms_helper_poll_init(drm);

	ret = drm_dev_register(drm, 0);
	if (ret)
		goto err_kms_helper_poll_fini;

	if (!disable_fbdev) {
		mtgpu_kick_out_firmware_fb(pdata->fb_base, pdata->fb_size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
		drm_fbdev_shmem_setup(drm, 32);
#else
		drm_fbdev_generic_setup(drm, 32);
#endif // KERNEL_VERSION
	}

	DRM_INFO("MooreThreads GPU drm driver loaded successfully\n");

	return 0;

err_kms_helper_poll_fini:
	drm_kms_helper_poll_fini(drm);
err_unbind_all:
	drm_mode_config_cleanup(drm);
	component_unbind_all(dev, drm);
err_free:
	kfree(private);
err_mode_config_cleanup:
	dev_set_drvdata(dev, NULL);
	drm_dev_put(drm);
	return ret;
}

static void mtgpu_component_unbind(struct device *dev)
{
	struct drm_device *drm = dev_get_drvdata(dev);

	drm_dev_unregister(drm);
	drm_kms_helper_poll_fini(drm);
	drm_atomic_helper_shutdown(drm);
	drm_mode_config_cleanup(drm);
	component_unbind_all(dev, drm);

	dev_set_drvdata(dev, NULL);
	drm_dev_put(drm);

	DRM_INFO("unload mtgpu drm kms driver\n");
}

static const struct component_master_ops mtgpu_component_ops = {
	.bind	= mtgpu_component_bind,
	.unbind = mtgpu_component_unbind,
};

static struct platform_driver *const component_drivers[] = {
	&dummy_crtc_driver,
	&dummy_connector_driver,
	&mtgpu_dispc_driver,
	&mtgpu_hdmi_driver,
	&mtgpu_dp_driver,
	&pvr_platform_driver,
#ifdef VPU_ENABLE
	&vpu_driver,
#endif
};

static int compare_dev(struct device *dev, void *data)
{
	return dev == data;
}

static void mtgpu_match_add_drivers(struct device *drm_dev,
				    struct component_match **match,
				    struct platform_driver *const *drivers,
				    int count)
{
	int i;

	for (i = 0; i < count; i++) {
		struct device_driver *drv = &drivers[i]->driver;
		struct device *dev_start = NULL, *component_dev;
		do {
#if defined(OS_FUNC_PLATFORM_FIND_DEVICE_BY_DRIVER_EXIST)
			component_dev = platform_find_device_by_driver(dev_start, drv);
#else
			component_dev = bus_find_device(&platform_bus_type, dev_start, drv,
							(void *)platform_bus_type.match);
#endif
			put_device(dev_start);
			dev_start = component_dev;

			if (!component_dev)
				break;

			if (drm_dev == component_dev->parent)
				component_match_add(drm_dev, match, compare_dev, component_dev);

		} while (true);
	}
}

static const struct of_device_id component_dev_of_ids[] = {
	{ .compatible = "apollo,dptx", .data = (void *)MTGPU_COMP_TYPE_DP },
	{ .compatible = "verisilicon,dc9x00", .data = (void *)MTGPU_COMP_TYPE_DC },
	{ .compatible = "mthreads,apollo-vpu", .data = (void *)MTGPU_COMP_TYPE_VIDEO },
	{ }
};

static int compare_node(struct device *dev, void *data)
{
	return dev->of_node == data;
}

static void mtgpu_match_add_device_nodes(struct device *drm_dev, struct component_match **match)
{
	struct device_node *node;
	struct device_node *phandle = drm_dev->parent->of_node;

	for_each_child_of_node(phandle->parent, node) {
		const struct platform_device *pdev;
		const struct of_device_id *of_id;
		enum mtgpu_of_component_type comp_type;

		of_id = of_match_node(component_dev_of_ids, node);
		if (!of_id)
			continue;

		if (mtgpu_display_is_dummy() || mtgpu_display_is_none()) {
			comp_type = (enum mtgpu_of_component_type)of_id->data;

			if (comp_type == MTGPU_COMP_TYPE_DP ||
			    comp_type == MTGPU_COMP_TYPE_DC) {
				dev_dbg(drm_dev, "Skipping display component: %s\n", node->name);
				continue;
			}
		}

		if (!of_device_is_available(node)) {
			dev_dbg(drm_dev, "Skipping disabled component: %s\n", node->name);
			continue;
		}

		pdev = of_find_device_by_node(node);
		if (!pdev || !pdev->dev.driver) {
			dev_dbg(drm_dev, "Skipping unbinded component: %s\n", node->name);
			continue;
		}

		drm_of_component_match_add(drm_dev, match, compare_node, node);
	}
}

static const struct acpi_device_id component_dev_acpi_ids[] = {
	{ .id = "DPTX0001", .driver_data = (kernel_ulong_t)MTGPU_COMP_TYPE_DP },
	{ .id = "DC9X0001", .driver_data = (kernel_ulong_t)MTGPU_COMP_TYPE_DC },
	{ .id = "MVPU0001", .driver_data = (kernel_ulong_t)MTGPU_COMP_TYPE_VIDEO },
	{ }, /* end of all entries */
};

struct match_info {
	struct device *drm_dev;
	struct component_match **match;
};

#if defined(OS_BUS_FIND_DEVICE_MATCH_USE_CONST_MODIFIER)
static int is_device_match_acpi_dev(struct device *dev, const void *adev)
#else
static int is_device_match_acpi_dev(struct device *dev, void *adev)
#endif
{
	return ACPI_COMPANION(dev) == adev;
}

static acpi_status register_acpi_device(acpi_handle handle, u32 level, void *data, void **ret)
{
	struct device *dev;
	struct acpi_device *adev;
	struct platform_device *pdev;
	struct match_info *match_info = data;
	const struct acpi_device_id *id;
	enum mtgpu_of_component_type comp_type;
	struct device *parent_dev = match_info->drm_dev->parent;
	struct mtgpu_device *mtdev = parent_dev->driver_data;

#if defined(OS_FUNC_ACPI_BUS_GET_DEVICE_EXIST)
	if (acpi_bus_get_device(handle, &adev))
		return AE_OK;
#else
	adev = acpi_fetch_acpi_dev(handle);
#endif
	/* Check if the device exists. */
	if (!adev || !adev->status.present)
		return AE_OK;

	/* Check if the compatible driver exists. */
	dev = bus_find_device(&platform_bus_type, NULL, adev, is_device_match_acpi_dev);
	pdev = dev ? to_platform_device(dev) : NULL;
	if (!pdev || !pdev->dev.driver)
		return AE_OK;

	/* Match ACPI device */
#if defined(OS_FUNC_ACPI_MATCH_ACPI_DEVICE_EXIST)
	id = acpi_match_acpi_device(component_dev_acpi_ids, adev);
#else
	id = acpi_match_device(component_dev_acpi_ids, dev);
#endif
	if (!id)
		return AE_OK;

	comp_type = (enum mtgpu_of_component_type)id->driver_data;
	if (mtgpu_display_is_dummy() || mtgpu_display_is_none()) {
		if (comp_type == MTGPU_COMP_TYPE_DP ||
		    comp_type == MTGPU_COMP_TYPE_DC) {
			dev_dbg(match_info->drm_dev, "Skipping display component: %s\n", id->id);
			return AE_OK;
		}
	}

	if (comp_type == MTGPU_COMP_TYPE_DC &&
	    !mtdev->display_device &&
	    iommu_present(dev->bus))
		mtdev->display_device = dev;

	/* Add compnent device. */
	component_match_add(match_info->drm_dev, match_info->match, compare_dev, dev);

	return AE_OK;
}

static void mtgpu_match_add_acpi_devices(struct device *drm_dev, struct component_match **match)
{
	struct match_info match_info = {drm_dev, match};

	acpi_walk_namespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
			    register_acpi_device, NULL, &match_info, NULL);
}

static int mtgpu_drm_probe(struct platform_device *pdev)
{
	struct component_match *match = NULL;
	struct device *dev = &pdev->dev;

	/* This is just for MPC when unbind and rebind drm devices. */
	if (!platform_get_drvdata(pdev))
		return -EPROBE_DEFER;

	/* This is just for IGPU to add components. */
	if (!dev_is_pci(dev->parent)) {
		if (acpi_disabled)
			mtgpu_match_add_device_nodes(dev, &match);
		else
			mtgpu_match_add_acpi_devices(dev, &match);
	}

	mtgpu_match_add_drivers(dev, &match, component_drivers,
				ARRAY_SIZE(component_drivers));

	return component_master_add_with_match(dev, &mtgpu_component_ops, match);
}

static int mtgpu_drm_remove(struct platform_device *pdev)
{
	component_master_del(&pdev->dev, &mtgpu_component_ops);
	return 0;
}

static int mtgpu_drm_suspend(struct device *dev)
{
	struct drm_device *drm = dev_get_drvdata(dev);
	int ret;

	DRM_DEV_INFO(dev, "mtgpu drm device suspend enter\n");

	ret = drm_mode_config_helper_suspend(drm);

	DRM_DEV_INFO(dev, "mtgpu drm device suspend exit\n");

	return ret;
}

static int mtgpu_drm_resume(struct device *dev)
{
	struct drm_device *drm = dev_get_drvdata(dev);

	DRM_INFO("mtgpu drm device resume enter\n");

	drm_mode_config_helper_resume(drm);

	/* trigger user space to check if monitors connected during suspend */
	drm_kms_helper_hotplug_event(drm);

	DRM_INFO("mtgpu drm device resume exit\n");

	return 0;
}

static struct platform_device_id mtgpu_device_id[] = {
	{ .name = "mtgpu-drm-device", },
	{ },
};
MODULE_DEVICE_TABLE(platform, mtgpu_device_id);

const struct dev_pm_ops mtgpu_drm_pm_ops = {
	.suspend = mtgpu_drm_suspend,
	.resume  = mtgpu_drm_resume,
	.freeze  = mtgpu_drm_suspend,
	.restore = mtgpu_drm_resume,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void _mtgpu_drm_remove(struct platform_device *pdev)
{
	mtgpu_drm_remove(pdev);
}
#endif // KERNEL_VERSION

static struct platform_driver mtgpu_drm_platform_driver = {
	.probe		= mtgpu_drm_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
	.remove		= _mtgpu_drm_remove,
#else
	.remove		= mtgpu_drm_remove,
#endif // KERNEL_VERSION
	.driver		= {
		.owner  = THIS_MODULE,
		.name	= "mtgpu-drm-driver",
		.pm	= &mtgpu_drm_pm_ops,
	},
	.id_table	= mtgpu_device_id,
};

/* WARNING: fb base/size should be acquired from pcie bar before pcie resize. */
int mtgpu_kick_out_firmware_fb(resource_size_t base, resource_size_t size)
{
#if defined(OS_FUNC_REMOVE_CONFLICTING_FRAMEBUFFERS_EXIST)
	struct apertures_struct *ap;

	ap = alloc_apertures(1);
	if (!ap)
		return -ENOMEM;

	ap->ranges[0].base = base;
	ap->ranges[0].size = size;

	remove_conflicting_framebuffers(ap, "mtgpudrmfb", false);

	kfree(ap);
#elif defined(OS_DRM_APERTURE_REMOVE_CONFLICTING_FRAMEBUFFERS_HAS_FOUR_ARG)
	drm_aperture_remove_conflicting_framebuffers(base,
						     size,
						     false,
						     &mtgpu_drm_driver);
#else
	drm_aperture_remove_conflicting_framebuffers(base,
						     size,
						     &mtgpu_drm_driver);
#endif

	return 0;
}

int mtgpu_drm_init(void)
{
	int ret;

	ret = pvr_drv_init();
	if (ret)
		return ret;

	ret = platform_register_drivers(component_drivers,
					ARRAY_SIZE(component_drivers));
	if (ret)
		return ret;

	ret = platform_driver_register(&mtgpu_dsc_driver);
	if (ret)
		DRM_ERROR("mtgpu dsc driver register failed (%d)\n", ret);

	ret = platform_driver_register(&mtgpu_phy_driver);
	if (ret)
		DRM_ERROR("mtgpu phy driver register failed (%d)\n", ret);

	ret = platform_driver_register(&mtgpu_drm_platform_driver);
	if (ret)
		DRM_ERROR("mtgpu platform driver register failed (%d)\n", ret);

	return ret;
}

DEFINE_IDR(mtgpu_global_bo_handle_idr);

void mtgpu_drm_fini(void)
{
	platform_unregister_drivers(component_drivers,
				    ARRAY_SIZE(component_drivers));
	pvr_drv_exit();

	platform_driver_unregister(&mtgpu_dsc_driver);

	platform_driver_unregister(&mtgpu_phy_driver);

	platform_driver_unregister(&mtgpu_drm_platform_driver);

	idr_destroy(&mtgpu_global_bo_handle_idr);
}
