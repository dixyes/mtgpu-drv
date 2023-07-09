/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>

#include "phy-dp.h"
#include "mtgpu_drv.h"
#include "mtgpu_phy_common.h"

static int mtgpu_phy_configure(struct phy *phy, union phy_configure_opts *opts)
{
	struct mtgpu_phy *mtgpu = phy_get_drvdata(phy);
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;
	struct phy_configure_opts_dp *phy_cfg = (struct phy_configure_opts_dp *)opts;

	dev_dbg(&phy->dev, "%s()\n", __func__);

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
			dev_err(&phy->dev, "%s(): Invalid vswing or preemph\n", __func__);
			return -EINVAL;
		}

		mtgpu->ops->set_vswing_preemph(ctx, vswing, preemph);
	}

	if (mtgpu->ops->set_ssc)
		mtgpu->ops->set_ssc(ctx, phy_cfg->ssc);

	return 0;
}

static int mtgpu_phy_init(struct phy *phy)
{
	struct mtgpu_phy *mtgpu = phy_get_drvdata(phy);
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(&phy->dev, "%s()\n", __func__);

	if (mtgpu->ops->init)
		return mtgpu->ops->init(ctx);

	return 0;
}

static int mtgpu_phy_power_on(struct phy *phy)
{
	struct mtgpu_phy *mtgpu = phy_get_drvdata(phy);
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(&phy->dev, "%s()\n", __func__);

	if (mtgpu->ops->power_on)
		return mtgpu->ops->power_on(ctx);

	return 0;
}

static int mtgpu_phy_power_off(struct phy *phy)
{
	struct mtgpu_phy *mtgpu = phy_get_drvdata(phy);
	struct mtgpu_phy_ctx *ctx = &mtgpu->ctx;

	dev_dbg(&phy->dev, "%s()\n", __func__);

	if (mtgpu->ops->power_off)
		return mtgpu->ops->power_off(ctx);

	return 0;
}

#if defined(OS_STRUCT_PHY_OPS_HAS_CONFIGURE)
static const struct phy_ops mtgpu_generic_phy_ops = {
	.init		= mtgpu_phy_init,
	.power_on	= mtgpu_phy_power_on,
	.power_off	= mtgpu_phy_power_off,
	.configure	= mtgpu_phy_configure,
};
#else
struct phy_ops_ext {
	struct phy_ops ops;
	int (*configure)(struct phy *phy, union phy_configure_opts *opts);
};

static const struct phy_ops_ext mtgpu_generic_phy_ops = {
	.ops = {
		.init		= mtgpu_phy_init,
		.power_on	= mtgpu_phy_power_on,
		.power_off	= mtgpu_phy_power_off,
	},
	.configure	= mtgpu_phy_configure,
};

int phy_configure(struct phy *phy, union phy_configure_opts *opts)
{
	int ret;
	struct phy_ops_ext *ops_ext;

	if (!phy)
		return -EINVAL;

	ops_ext = container_of(phy->ops, struct phy_ops_ext, ops);
	if (!ops_ext->configure)
		return -EOPNOTSUPP;

	mutex_lock(&phy->mutex);
	ret = ops_ext->configure(phy, opts);
	mutex_unlock(&phy->mutex);

	return ret;
}
#endif /* OS_STRUCT_PHY_OPS_HAS_CONFIGURE */

static int mtgpu_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtgpu_phy *mtgpu;
	struct resource *res;
	int ret;
	struct mtgpu_dp_phy_platform_data *phy_pdata = dev_get_platdata(dev);

	mtgpu = devm_kzalloc(dev, sizeof(*mtgpu), GFP_KERNEL);
	if (!mtgpu)
		return -ENOMEM;

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

	mtgpu->phy = devm_phy_create(dev, NULL, (const struct phy_ops *)&mtgpu_generic_phy_ops);
	if (IS_ERR(mtgpu->phy)) {
		dev_err(dev, "Failed to create mtgpu dp-phy device\n");
		return PTR_ERR(mtgpu->phy);
	}

	platform_set_drvdata(pdev, mtgpu);
	phy_set_drvdata(mtgpu->phy, mtgpu);

	ret = phy_create_lookup(mtgpu->phy, "dp-phy", dev_name(dev->parent));
	if (ret)
		dev_warn(dev, "Failed to create dp-phy lookup, ret = %d\n", ret);

	switch (phy_pdata->soc_gen) {
	case GPU_SOC_GEN1:
		mtgpu->ops = &mtgpu_phy_cdns;
		break;
	case GPU_SOC_GEN2:
		mtgpu->ops = &mtgpu_phy_snps;
		break;
	case GPU_SOC_GEN3:
#if defined(CONFIG_QUYUAN2_HAPS)
		mtgpu->ops = &mtgpu_phy_xlnx;
#else
		mtgpu->ops = &mtgpu_phy_snps;
#endif
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
	struct mtgpu_phy *mtgpu = platform_get_drvdata(pdev);

	phy_remove_lookup(mtgpu->phy, "dp-phy", dev_name(pdev->dev.parent));

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
