/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_HDMI_COMMON_H__
#define __MTGPU_HDMI_COMMON_H__

#include "mtgpu_display_debug.h"
#include "os-interface-drm.h"

#define MTGPU_HDMI_IRQ_TYPE_HPD_IN	BIT(0)
#define MTGPU_HDMI_IRQ_TYPE_HPD_OUT	BIT(1)

#define DELAY_ON_HDMI_PLUGOUT_DETECTION_MS (1250)

#define DDC_SCDCS_ADDR		0X54

typedef void (*hdmi_codec_plugged_cb)(struct device *dev, bool plugged);

struct wait_queue_head;
typedef struct wait_queue_head wait_queue_head_t;

struct hdmi_codec_params;

struct mtgpu_hdmi_debugfs {
	struct dentry *dentry;
};

struct mtgpu_hdmi_ctx {
	void __iomem *regs;
	void __iomem *phy_regs;
	int irq;
	struct videomode *vm;
	u16 max_hres;
	u16 max_vres;
	u16 max_pclk_100khz;
	int pclk;
	u8 bpp;
	u32 update_done;
	wait_queue_head_t *waitq;
	void *private;
};

struct mtgpu_hdmi_ops {
	u32 (*isr)(struct mtgpu_hdmi_ctx *ctx);
	void (*ctx_init)(struct mtgpu_hdmi_ctx *ctx);
	void (*ctx_exit)(struct mtgpu_hdmi_ctx *ctx);
	void (*hw_init)(struct mtgpu_hdmi_ctx *ctx);
	void (*hw_deinit)(struct mtgpu_hdmi_ctx *ctx);
	void (*hpd_enable)(struct mtgpu_hdmi_ctx *ctx);
	void (*config)(struct mtgpu_hdmi_ctx *ctx);
	void (*enable)(struct mtgpu_hdmi_ctx *ctx);
	void (*disable)(struct mtgpu_hdmi_ctx *ctx);
	void (*avi_infoframe)(struct mtgpu_hdmi_ctx *ctx, u8 *buf, u8 len);
	void (*spd_infoframe)(struct mtgpu_hdmi_ctx *ctx, u8 *buf, u8 len);
	void (*audio_infoframe)(struct mtgpu_hdmi_ctx *ctx, u8 *buf, u8 len);
	void (*vendor_infoframe)(struct mtgpu_hdmi_ctx *ctx, u8 *buf, u8 len);
	bool (*is_plugin)(struct mtgpu_hdmi_ctx *ctx);
	int (*i2c_read)(struct mtgpu_hdmi_ctx *ctx, u8 addr, u8 *buf, u8 len);
	int (*i2c_write)(struct mtgpu_hdmi_ctx *ctx, u8 addr, u8 *buf, u8 len);
	void (*audio_enable)(struct mtgpu_hdmi_ctx *ctx);
	void (*audio_disable)(struct mtgpu_hdmi_ctx *ctx);
	void (*audio_mute)(struct mtgpu_hdmi_ctx *ctx);
	void (*audio_unmute)(struct mtgpu_hdmi_ctx *ctx);
	void (*scramble_config)(struct mtgpu_hdmi_ctx *ctx, bool enable);
	int (*audio_set_param)(struct mtgpu_hdmi_ctx *ctx, struct hdmi_codec_params *params);
};

struct mtgpu_hdmi_glb_ops {
	void (*init)(struct mtgpu_hdmi_ctx *ctx);
};

struct mtgpu_hdmi_chip {
	struct mtgpu_hdmi_ops *core;
	struct mtgpu_hdmi_glb_ops *glb;
};

struct mtgpu_hdmi {
	struct drm_encoder *encoder;
	struct drm_connector *connector;
	struct mtgpu_hdmi_ctx ctx;
	struct mtgpu_hdmi_ops *core;
	struct mtgpu_hdmi_glb_ops *glb;
	struct device *dev;
	struct i2c_adapter *ddc;
	struct mutex *i2c_lock;
	struct delayed_work *hpd_work;
	u8 port_type;
	/* audio codec */
	struct platform_device *hdmi_audio;
	hdmi_codec_plugged_cb codec_plugged_cb;

	struct device *codec_dev;
	struct mutex *codec_lock;
	struct edid *edid;
	bool connected;
	bool enabled;
	bool audio_enabled;

	struct drm_property *prop_fixed_edid;
	bool fixed_edid;
	u8 fixed_inited;
	/* hdmi debugfs */
	struct mtgpu_hdmi_debugfs debugfs;
};

static inline void hdmi_reg_write(struct mtgpu_hdmi_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->regs + offset);
	HDMI_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
}

static inline u32 hdmi_reg_read(struct mtgpu_hdmi_ctx *ctx, int offset)
{
	u32 val = os_readl(ctx->regs + offset);

	HDMI_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
	return val;
}

static inline
void hdmi_reg_set(struct mtgpu_hdmi_ctx *ctx, int offset, u32 bits)
{
	u32 reg_val = hdmi_reg_read(ctx, offset);

	hdmi_reg_write(ctx, offset, reg_val | bits);
}

static inline
void hdmi_reg_clr(struct mtgpu_hdmi_ctx *ctx, int offset, u32 bits)
{
	u32 reg_val = hdmi_reg_read(ctx, offset);

	hdmi_reg_write(ctx, offset, reg_val & ~bits);
}

extern struct mtgpu_hdmi_chip mtgpu_hdmi_chip_sudi;

static inline
struct mtgpu_hdmi *encoder_to_mtgpu_hdmi(struct drm_encoder *encoder)
{
	return os_get_drm_encoder_drvdata(encoder);
}

static inline
struct mtgpu_hdmi *connector_to_mtgpu_hdmi(struct drm_connector *connector)
{
	return os_get_drm_connector_drvdata(connector);
}

static inline
struct mtgpu_hdmi *dwork_to_mtgpu_hdmi(struct work_struct *work)
{
	return os_get_dwork_drvdata(work);
}

int  mtgpu_hdmi_kernel_struct_create(struct mtgpu_hdmi *hdmi);
void mtgpu_hdmi_kernel_struct_destroy(struct mtgpu_hdmi *hdmi);
int mtgpu_hdmi_update_fixed_edid_flag(struct mtgpu_hdmi *hdmi, bool enable);

#endif /* __MTGPU_HDMI_COMMON_H__ */
