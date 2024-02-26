/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/component.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <drm/drm_gem.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_ioctl.h>
#include <drm/drm_device.h>
#endif

#include "mtgpu_defs.h"
#include "mtgpu_device.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drv.h"

#include "mtvpu_drv.h"

#include "mtjpu_config.h"
#include "mtjpu_common.h"
#include "mtjpu_irq.h"
#include "mtjpu_dec.h"
#include "mtjpu_drv.h"

struct mtjpu_device *drm_dev_to_jpu_device(struct drm_device *drm)
{
	struct mtgpu_drm_private *drm_private = drm->dev_private;

	if (!drm_private)
		return NULL;

	return drm_private->chip->device_jpu;
}

static int mtjpu_component_bind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm;
	struct platform_device *plat_dev;
	struct mtgpu_drm_private *private;
	struct mtgpu_device *mtdev;
	struct mtjpu_device *jpu_device;
	int ret = 0;

	plat_dev = to_platform_device(dev);

	drm = (struct drm_device *)data;
	if (!drm)
		return -EINVAL;

	private = (struct mtgpu_drm_private *)drm->dev_private;
	if (!private)
		return -EINVAL;

	jpu_device = kzalloc(sizeof(struct mtjpu_device), GFP_KERNEL);
	if (!jpu_device)
		return -ENOMEM;

	mtdev = dev_get_drvdata(dev->parent->parent);
	if (!mtdev) {
		dev_err(dev, "failed to get mtdev\n");
		goto err_dev_get_drvdata;
	}

	private->chip->device_jpu = jpu_device;

	mtjpu_device_jpu_init(jpu_device, dev, drm, mtdev, plat_dev);

	mtjpu_core_init(jpu_device->jpu_core, jpu_device->pcie_io_base);

	ret = mtjpu_device_lock_init(jpu_device);
	if (ret) {
		dev_err(dev, "mtjpu lock init failed\n");
		goto err_mtjpu_device_lock_init;
	}

	ret = mtjpu_kfifo_init(&jpu_device->fifo, MAX_NUM_JPU_CORE);
	if (ret) {
		dev_err(dev, "mtjpu kfifo init failed\n");
		goto err_mtjpu_kfifo_init;
	}

	ret = mtjpu_core_sema_init(&jpu_device->core_sema, MAX_NUM_JPU_CORE);
	if (ret) {
		dev_err(dev, "mtjpu core sema init failed\n");
		goto err_mtjpu_core_sema_init;
	}

	mtjpu_device_init(jpu_device, jpu_device->idx);

	ret = mtjpu_irq_init(plat_dev, jpu_device->idx);
	if (ret) {
		dev_err(dev, "mtjpu irq init failed\n");
		goto err_mtjpu_irq_init;
	}

	ret = mtjpu_fill_drm_ioctls(pvr_drm_ioctls, 128);
	if (ret) {
		dev_err(dev, "mtjpu fill drm ioctls failed\n");
		goto err_mtjpu_irq_init;
	}

	return 0;

err_mtjpu_irq_init:
	mtjpu_irq_free(plat_dev, jpu_device->idx);

err_mtjpu_core_sema_init:
	kfree(jpu_device->core_sema);

err_mtjpu_kfifo_init:
	mtjpu_kfifo_free(jpu_device->fifo);

err_mtjpu_device_lock_init:
	mtjpu_device_lock_uninit(jpu_device);

err_dev_get_drvdata:
	kfree(jpu_device);

	return -EINVAL;
}

static void mtjpu_component_unbind(struct device *dev, struct device *master, void *data)
{
	struct mtjpu_device *jpu_device;
	struct platform_device *plat_dev;

	jpu_device = dev_get_drvdata(dev);
	plat_dev = to_platform_device(dev);

	mtjpu_irq_free(plat_dev, jpu_device->idx);

	mtjpu_kfifo_free(jpu_device->fifo);

	mtjpu_device_lock_uninit(jpu_device);

	mtjpu_core_uninit(jpu_device->jpu_core);

	mtjpu_device_uninit(jpu_device->idx);

	kfree(jpu_device->core_sema);

	kfree(jpu_device);
}

static const struct component_ops mtjpu_component_ops = {
	.bind = mtjpu_component_bind,
	.unbind = mtjpu_component_unbind,
};

static int mtjpu_probe(struct platform_device *plat_dev)
{
	return component_add(&plat_dev->dev, &mtjpu_component_ops);
}

static int mtjpu_remove(struct platform_device *plat_dev)
{
	component_del(&plat_dev->dev, &mtjpu_component_ops);

	return 0;
}

static struct platform_device_id mtjpu_id_tbl[] = {
	{ .name = MTGPU_DEVICE_NAME_JPU },
	{}
};

struct platform_driver jpu_driver = {
	.driver = {
		.name = MTGPU_DEVICE_NAME_JPU,
	},
	.probe = mtjpu_probe,
	.remove = mtjpu_remove,
	.id_table = mtjpu_id_tbl,
};
