/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>

#include "mtgpu_phy_dp.h"
#include "mtgpu_drv.h"
#include "mtgpu_phy_common.h"
#include "os-interface.h"

int mtgpu_phy_configure(struct mtgpu_phy *mtgpu, union mtgpu_phy_configure_opts *opts)
{
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;
	struct mtgpu_phy_configure_opts_dp *phy_cfg = (struct mtgpu_phy_configure_opts_dp *)opts;

	dev_dbg(mtgpu->dev, "%s()\n", __func__);

	if (phy_cfg->set_lanes && mtgpu->ops->set_lane_count) {
		ctx->lane_count = phy_cfg->lanes;
		mtgpu->ops->set_lane_count(ctx, phy_cfg->lanes);
	}

	if (phy_cfg->set_rate && mtgpu->ops->set_link_rate) {
		ctx->bw_code = phy_cfg->link_rate;
		mtgpu->ops->set_link_rate(ctx, phy_cfg->link_rate);
	}

	if (phy_cfg->set_voltages && mtgpu->ops->set_vswing_preemph) {
		u8 vswing = phy_cfg->voltage[0];
		u8 preemph = phy_cfg->pre[0];

		if (vswing > 3 || preemph > 3) {
			dev_err(mtgpu->dev, "%s(): Invalid vswing or preemph\n", __func__);
			return -EINVAL;
		}

		mtgpu->ops->set_vswing_preemph(ctx, vswing, preemph);
	}

	if (mtgpu->ops->set_ssc)
		mtgpu->ops->set_ssc(ctx, phy_cfg->ssc);

	return 0;
}

int mtgpu_phy_init(struct mtgpu_phy *mtgpu)
{
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(mtgpu->dev, "%s()\n", __func__);

	if (mtgpu->ops->init)
		return mtgpu->ops->init(ctx);

	return 0;
}

int mtgpu_phy_exit(struct mtgpu_phy *mtgpu)
{
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(mtgpu->dev, "%s()\n", __func__);

	if (mtgpu->ops->exit)
		return mtgpu->ops->exit(ctx);

	return 0;
}

int mtgpu_phy_power_on(struct mtgpu_phy *mtgpu)
{
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(mtgpu->dev, "%s()\n", __func__);

	if (mtgpu->ops->power_on)
		return mtgpu->ops->power_on(ctx);

	return 0;
}

int mtgpu_phy_power_off(struct mtgpu_phy *mtgpu)
{
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(mtgpu->dev, "%s()\n", __func__);

	if (mtgpu->ops->power_off)
		return mtgpu->ops->power_off(ctx);

	return 0;
}

static int mtgpu_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtgpu_phy *mtgpu;
	struct resource *res;
	struct mtgpu_dp_phy_platform_data *phy_pdata = dev_get_platdata(dev);

	mtgpu = devm_kzalloc(dev, sizeof(*mtgpu), GFP_KERNEL);
	if (!mtgpu)
		return -ENOMEM;

	mtgpu->dev = dev;
	mtgpu->ctx.id = phy_pdata->id;
	mtgpu->ctx.phy_cfg_hdr = phy_pdata->phy_cfg_hdr;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy-regs");
	if (!res) {
		dev_err(dev, "Failed to get regs from %s\n", pdev->name);
		return -EIO;
	}

	mtgpu->ctx.regs = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(mtgpu->ctx.regs))
		return PTR_ERR(mtgpu->ctx.regs);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sram-regs");
	if (res) {
		mtgpu->ctx.sram_regs = devm_ioremap(dev, res->start, resource_size(res));
		if (IS_ERR(mtgpu->ctx.sram_regs))
			return PTR_ERR(mtgpu->ctx.sram_regs);
	}

	platform_set_drvdata(pdev, mtgpu);

	switch (phy_pdata->soc_gen) {
	case GPU_SOC_GEN1:
		mtgpu->ops = &mtgpu_phy_cdns;
		break;
	case GPU_SOC_GEN2:
		mtgpu->ops = &mtgpu_phy_snps;
		break;
	case GPU_SOC_GEN3:
		mtgpu->ops = &mtgpu_phy_snps;
		break;
	default:
		dev_err(dev, "%s() current SOC_GEN%d is not supported\n", __func__,
			phy_pdata->soc_gen);
		return -ENOTSUPP;
	}

	if (!mtgpu->ops) {
		dev_err(dev, "mtgpu phy ops is null\n");
		return -EINVAL;
	}

	dev_info(dev, "%s() mtgpu dp-phy driver probe successfully\n", __func__);

	return 0;
}

static int mtgpu_phy_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "remove mtgpu dp-phy driver\n");

	return 0;
}

static struct platform_device_id mtgpu_phy_device_id[] = {
	{ .name = MTGPU_DEVICE_NAME_DP_PHY, },
	{ },
};

struct platform_driver mtgpu_phy_driver = {
	.probe	= mtgpu_phy_probe,
	.remove	= mtgpu_phy_remove,
	.driver	= {
		.owner  = THIS_MODULE,
		.name	= "mtgpu-dp-phy-drv",
	},
	.id_table = mtgpu_phy_device_id,
};
