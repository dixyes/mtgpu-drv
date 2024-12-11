/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */
#include <linux/io.h>
#include <linux/component.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

#include <drm/drm_print.h>
#include <drm/drm_atomic.h>
#include <video/videomode.h>
#if defined(OS_DRM_DRM_DSC_H_EXIST)
#include <drm/drm_dsc.h>
#elif defined(OS_DRM_DISPLAY_DRM_DSC_HELPER_H_EXIST)
#include <drm/display/drm_dsc_helper.h>
#endif

#include "mtgpu_drv.h"
#include "mtgpu_drm_dsc.h"
#include "mtgpu_dsc_common.h"
#include "mtgpu_dp_common.h"
#include "mtgpu_dispc_common.h"

#if defined(OS_FUNC_DRM_DSC_COMPUTE_RC_PARAMETERS_EXIST)
static void mtgpu_dsc_config_to_drm_dsc_config(struct drm_dsc_config *dsc_config,
					       struct mtgpu_dsc_config *mtgpu_dsc_confg)
{
	/* struct mtgpu_dsc_confg is same as struct drm_dsc_config */
	memcpy(dsc_config, mtgpu_dsc_confg, sizeof(*mtgpu_dsc_confg));
}
#endif

void mtgpu_compute_dsc_params(struct drm_encoder *encoder,
			      struct drm_crtc *crtc)
{
	struct mtgpu_dp *dp = encoder_to_mtgpu_dp(encoder);
	struct mtgpu_dsc *dsc = dp->dsc;
	struct mtgpu_dsc_config mtgpu_dsc_cfg = {0};
#if defined(OS_FUNC_DRM_DSC_COMPUTE_RC_PARAMETERS_EXIST)
	struct drm_dsc_picture_parameter_set pps_payload = {0};
	struct drm_dsc_config dsc_cfg = {0};
#else
	struct mtgpu_dsc_picture_parameter_set pps_packed = {0};
#endif

	DRM_DEV_INFO(dp->dev, "compute dsc params.\n");

	mtgpu_get_dsc_config(encoder, crtc, &mtgpu_dsc_cfg);

	memcpy(dsc->ctx.vm, dp->ctx.vm, sizeof(struct videomode));

#if defined(OS_FUNC_DRM_DSC_COMPUTE_RC_PARAMETERS_EXIST)
	mtgpu_dsc_config_to_drm_dsc_config(&dsc_cfg, &mtgpu_dsc_cfg);
	drm_dsc_compute_rc_parameters(&dsc_cfg);
	drm_dsc_pps_payload_pack(&pps_payload, &dsc_cfg);
	mtgpu_dsc_param_save(dp, dsc, &mtgpu_dsc_cfg, (u32 *)&pps_payload);
#else
	mtgpu_dsc_compute_rc_parameters(&mtgpu_dsc_cfg);
	mtgpu_dsc_pps_payload_pack(&pps_packed, &mtgpu_dsc_cfg);
	mtgpu_dsc_param_save(dp, dsc, &mtgpu_dsc_cfg, (u32 *)&pps_packed);
#endif
}

void mtgpu_dsc_param_restore(struct drm_encoder *encoder,
			     struct drm_crtc *crtc)
{
	struct mtgpu_dp *dp = encoder_to_mtgpu_dp(encoder);

	DRM_DEV_INFO(dp->dev, "restore dsc params.\n");

	mtgpu_dsc_param_clear(encoder, crtc);
}

static int mtgpu_dsc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtgpu_dsc_platform_data *pdata = dev_get_platdata(dev);
	struct mtgpu_dsc_chip *chip;
	struct mtgpu_dsc *dsc;
	struct resource *res;
	int ret;

	dsc =  kzalloc(sizeof(*dsc), GFP_KERNEL);
	if (!dsc)
		return -ENOMEM;

	dsc->ctx.vm = kzalloc(sizeof(*dsc->ctx.vm), GFP_KERNEL);
	if (!dsc->ctx.vm) {
		ret = -ENOMEM;
		goto err_free_dsc;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dsc-regs");
	if (!res) {
		dev_err(dev, "failed to get dispc-regs\n");
		ret = -EIO;
		goto err_free_dsc;
	}

	dsc->ctx.regs = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(dsc->ctx.regs)) {
		ret = PTR_ERR(dsc->ctx.regs);
		goto err_free_dsc;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "glb-regs");
	if (!res) {
		dev_err(dev, "failed to get display glb-regs\n");
		ret = -EIO;
		goto err_free_dsc;
	}

	dsc->ctx.glb_regs = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(dsc->ctx.glb_regs)) {
		ret = PTR_ERR(dsc->ctx.glb_regs);
		goto err_free_dsc;
	}

	if (pdata->soc_gen == GPU_SOC_GEN3) {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "crg-regs");
		if (!res) {
			dev_err(dev, "failed to get display crg-regs\n");
			ret = -EIO;
			goto err_free_dsc;
		}

		dsc->ctx.crg_regs = devm_ioremap(dev, res->start, resource_size(res));
		if (IS_ERR(dsc->ctx.crg_regs)) {
			ret = PTR_ERR(dsc->ctx.crg_regs);
			goto err_free_dsc;
		}
	}

	dsc->ctx.id = pdata->id;
	dsc->dev = dev;
	dev_set_drvdata(dev, dsc);

	switch (pdata->soc_gen) {
	case GPU_SOC_GEN2:
		chip = &mtgpu_dsc_qy1;
		dsc->ctx.version = MT_DSC_VERSION_01;
		break;
	case GPU_SOC_GEN3:
		chip = &mtgpu_dsc_qy2;
		dsc->ctx.version = MT_DSC_VERSION_02;
		break;
	default:
		dev_err(dev, "current SOC_GEN%d is not supported\n", pdata->soc_gen);
		ret = -ENOTSUPP;
		goto err_free_dsc;
	}

	dsc->core = chip->core;
	dsc->glb = chip->glb;
	if (!dsc->core) {
		dev_err(dev, "dsc ops is null\n");
		ret = -EINVAL;
		goto err_free_dsc;
	}

	dev_info(dev, "%s() mtgpu dsc driver probe successfully\n", __func__);

	return 0;

err_free_dsc:
	kfree(dsc->ctx.vm);
	kfree(dsc);
	return ret;
}

static int mtgpu_dsc_remove(struct platform_device *pdev)
{
	struct mtgpu_dsc *dsc = dev_get_drvdata(&pdev->dev);

	kfree(dsc->ctx.vm);
	kfree(dsc);

	dev_info(&pdev->dev, "unload mtgpu dsc driver\n");
	return 0;
}

static struct platform_device_id mtgpu_dsc_device_id[] = {
	{ .name = MTGPU_DEVICE_NAME_DSC, },
	{ },
};


#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void _mtgpu_dsc_remove(struct platform_device *pdev)
{
	mtgpu_dsc_remove(pdev);
}
#endif // KERNEL_VERSION

struct platform_driver mtgpu_dsc_driver = {
	.probe    = mtgpu_dsc_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
	.remove   = _mtgpu_dsc_remove,
#else
	.remove   = mtgpu_dsc_remove,
#endif // KERNEL_VERSION
	.driver   = {
		.owner  = THIS_MODULE,
		.name   = "mtgpu-dsc-drv",
	},
	.id_table = mtgpu_dsc_device_id,
};
