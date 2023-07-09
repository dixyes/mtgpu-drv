/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTGPU_DISPC_COMMON_H_
#define _MTGPU_DISPC_COMMON_H_

#include "mtgpu_display_debug.h"

struct wait_queue_head;
typedef struct wait_queue_head wait_queue_head_t;

#define DISPC_INT_BIT_VSYNC	BIT(0)
#define DISPC_INT_BIT_UNDERRUN	BIT(1)

/* Supported FBC modes */
#define DISPC_FBC_MODE_LINEAR     (0x00)
#define DISPC_FBC_MODE_8X8_V10    (0x01)
#define DISPC_FBC_MODE_16X4_V10   (0x02)
#define DISPC_FBC_MODE_MAX        (0x03)

/* Supported FBC fmt */
#define DISPC_FBC_FMT_NONE       (0x00)
#define DISPC_FBC_FMT_ARGB8888   (0x0C)

enum mtgpu_layer_type {
	MTGPU_LAYER_TYPE_OVERLAY, /* DRM_PLANE_TYPE_OVERLAY */
	MTGPU_LAYER_TYPE_PRIMARY, /* DRM_PLANE_TYPE_PRIMARY */
	MTGPU_LAYER_TYPE_CURSOR, /* DRM_PLANE_TYPE_CURSOR */
};

struct mtgpu_dispc_debugfs {
	struct dentry *dentry;
	u64 underrun_cnt;
};

struct mtgpu_layer_capability {
	u8 type;
	u8 fmts_cnt;
	const u32 *fmts_ptr;
	const u64 *modifiers;
	u32 supported_encodings;
	u32 supported_ranges;
};

struct mtgpu_dispc_capability {
	u8 layer_count;
	const struct mtgpu_layer_capability *layer_caps;
	u32 gamma_size;
};

struct mtgpu_layer_config {
	u16 src_x;
	u16 src_y;
	u16 src_w;
	u16 src_h;
	s16 dst_x;
	s16 dst_y;
	u16 dst_w;
	u16 dst_h;
	u32 format;
	u32 addr[4];
	u32 pitch[4];
	u8 num_planes;
	u8 alpha;
	u8 blend_mode;
	u8 rotation;
	u8 color_encoding;
	u8 color_range;
	u8 is_fbc;
	u32 height;
	u32 header_size_r;
	u32 header_size_y;
	u32 header_size_uv;
};

struct mtgpu_dispc_ctx {
	void __iomem *regs;
	void __iomem *glb_regs;
	u64 cursor_mem_base;
	u32 cursor_mem_size;
	u8 cursor_buffer_index;
	u64 pcie_mem_base;
	int irq;
	int id;
	struct videomode *vm;
	void *private;
	bool update_done;
	bool disable_done;
	wait_queue_head_t *waitq;
};

struct mtgpu_dispc_ops {
	void (*init)(struct mtgpu_dispc_ctx *ctx);
	void (*deinit)(struct mtgpu_dispc_ctx *ctx);
	int (*ctx_init)(struct mtgpu_dispc_ctx *ctx);
	void (*ctx_deinit)(struct mtgpu_dispc_ctx *ctx);
	void (*start)(struct mtgpu_dispc_ctx *ctx);
	void (*stop)(struct mtgpu_dispc_ctx *ctx);
	u32 (*isr)(struct mtgpu_dispc_ctx *ctx);
	void (*config_begin)(struct mtgpu_dispc_ctx *ctx);
	void (*layer_config)(struct mtgpu_dispc_ctx *ctx,
			     struct mtgpu_layer_config *layer, u8 index);
	void (*layer_disable)(struct mtgpu_dispc_ctx *ctx, u8 index);
	void (*config_end)(struct mtgpu_dispc_ctx *ctx);
	void (*vsync_on)(struct mtgpu_dispc_ctx *ctx);
	void (*vsync_off)(struct mtgpu_dispc_ctx *ctx);
	void (*capability)(struct mtgpu_dispc_ctx *ctx,
			   struct mtgpu_dispc_capability *cap);
	u32 (*mode_valid)(struct mtgpu_dispc_ctx *ctx,
			   struct videomode *vm);
	void (*gamma_set)(struct mtgpu_dispc_ctx *ctx, u16 *r, u16 *g, u16 *b, u32 size);
	void (*cursor_config)(struct mtgpu_dispc_ctx *ctx,
			      struct mtgpu_layer_config *layer);
	bool (*fbc_validate)(struct mtgpu_dispc_ctx *ctx,
			     u32 format, u64 modifier, u32 index);
};

struct mtgpu_dispc_glb_ops {
	void (*init)(struct mtgpu_dispc_ctx *ctx);
	void (*enable)(struct mtgpu_dispc_ctx *ctx);
	void (*disable)(struct mtgpu_dispc_ctx *ctx);
	void (*fbc_enable)(struct mtgpu_dispc_ctx *ctx,
			   struct mtgpu_layer_config *layer, u32 index);
	void (*fbc_disable)(struct mtgpu_dispc_ctx *ctx, u32 index);
};

struct mtgpu_dispc_chip {
	struct mtgpu_dispc_ops *core;
	struct mtgpu_dispc_glb_ops *glb;
};

static inline
void dispc_reg_write(struct mtgpu_dispc_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->regs + offset);
	/* dummy read to make post write take effect */
	os_readl(ctx->regs + offset);
	DISPC_DEBUG("offset = 0x%04x value = 0x%08x\n", offset, val);
}

static inline
u32 dispc_reg_read(struct mtgpu_dispc_ctx *ctx, int offset)
{
	u32 val = os_readl(ctx->regs + offset);

	DISPC_DEBUG("offset = 0x%04x value = 0x%08x\n", offset, val);
	return val;
}

static inline
void dispc_reg_set(struct mtgpu_dispc_ctx *ctx, int offset, u32 bits)
{
	u32 reg_val = dispc_reg_read(ctx, offset);

	dispc_reg_write(ctx, offset, reg_val | bits);
}

static inline
void dispc_reg_clr(struct mtgpu_dispc_ctx *ctx, int offset, u32 bits)
{
	u32 reg_val = dispc_reg_read(ctx, offset);

	dispc_reg_write(ctx, offset, reg_val & ~bits);
}

/*
 * In order to work aroud a hardware problem, we need to make sure all reg_write as
 * atomic operation, and add read after write to avoid write-write operations.
 */
static inline
void dispc_reg_write2(struct mtgpu_dispc_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->regs + offset);
	os_readl(ctx->regs + offset);
}

static inline
u32 dispc_reg_read2(struct mtgpu_dispc_ctx *ctx, int offset)
{
	return os_readl(ctx->regs + offset);
}

static inline
void dispc_glb_reg_write(struct mtgpu_dispc_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->glb_regs + offset);
	/* dummy read to make post write take effect */
	os_readl(ctx->glb_regs + offset);
	DISPC_DEBUG("offset = 0x%04x value = 0x%08x\n", offset, val);
}

static inline
u32 dispc_glb_reg_read(struct mtgpu_dispc_ctx *ctx, int offset)
{
	u32 val = os_readl(ctx->glb_regs + offset);

	DISPC_DEBUG("offset = 0x%04x value = 0x%08x\n", offset, val);
	return val;
}

static inline
void dispc_glb_reg_set(struct mtgpu_dispc_ctx *ctx, int offset, u32 bits)
{
	u32 val = dispc_glb_reg_read(ctx, offset);

	dispc_glb_reg_write(ctx, offset, val | bits);
}

static inline
void dispc_glb_reg_clr(struct mtgpu_dispc_ctx *ctx, int offset, u32 bits)
{
	u32 val = dispc_glb_reg_read(ctx, offset);

	dispc_glb_reg_write(ctx, offset, val & ~bits);
}

extern struct mtgpu_dispc_chip mtgpu_dispc_sudi;
extern struct mtgpu_dispc_chip mtgpu_dispc_qy1;
extern struct mtgpu_dispc_chip mtgpu_dispc_qy2;

#endif /* _MTGPU_DISPC_COMMON_H_ */
