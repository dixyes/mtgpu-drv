/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTGPU_PHY_COMMON_H_
#define _MTGPU_PHY_COMMON_H_

#include "os-interface.h"

struct mtgpu_phy_ctx {
	void __iomem *regs;
	void __iomem *dp_regs;
	void __iomem *glb_regs;
	void __iomem *sram_regs;
	u8 lane_count;
	u8 bw_code;
	u8 vswing_level;
	u8 preemph_level;
	u8 id;
	struct dp_phy_cfg_hdr *phy_cfg_hdr;
};

struct mtgpu_phy {
	struct phy *phy;
	struct mtgpu_phy_ctx ctx;
	struct mtgpu_phy_ops *ops;
};

struct mtgpu_phy_ops {
	int (*init)(struct mtgpu_phy_ctx *ctx);
	int (*power_on)(struct mtgpu_phy_ctx *ctx);
	int (*power_off)(struct mtgpu_phy_ctx *ctx);
	void (*set_lane_count)(struct mtgpu_phy_ctx *ctx, u8 count);
	void (*set_link_rate)(struct mtgpu_phy_ctx *ctx, u8 bw_code);
	void (*set_vswing_preemph)(struct mtgpu_phy_ctx *ctx, u8 vswing, u8 preemph);
	void (*set_ssc)(struct mtgpu_phy_ctx *ctx, bool ssc_en);
};

static inline void phy_reg_write(struct mtgpu_phy_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->regs + offset);
}

static inline u32 phy_reg_read(struct mtgpu_phy_ctx *ctx, int offset)
{
	return os_readl(ctx->regs + offset);
}

static inline void phy_dptx_reg_write(struct mtgpu_phy_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->dp_regs + offset);
	/* dummy read to make post write take effect */
	os_readl(ctx->dp_regs + offset);
}

static inline u32 phy_dptx_reg_read(struct mtgpu_phy_ctx *ctx, int offset)
{
	return os_readl(ctx->dp_regs + offset);
}

static inline void phy_glb_reg_write(struct mtgpu_phy_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->glb_regs + offset);
	/* dummy read to make post write take effect */
	os_readl(ctx->glb_regs + offset);
}

static inline u32 phy_glb_reg_read(struct mtgpu_phy_ctx *ctx, int offset)
{
	return os_readl(ctx->glb_regs + offset);
}

static inline void phy_sram_reg_write(struct mtgpu_phy_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->sram_regs + offset);
	/* dummy read to make post write take effect */
	os_readl(ctx->sram_regs + offset);
}

static inline u32 phy_sram_reg_read(struct mtgpu_phy_ctx *ctx, int offset)
{
	return os_readl(ctx->sram_regs + offset);
}

extern struct mtgpu_phy_ops mtgpu_phy_xlnx;
extern struct mtgpu_phy_ops mtgpu_phy_cdns;
extern struct mtgpu_phy_ops mtgpu_phy_snps;

#endif /* _MTGPU_PHY_COMMON_H_ */
