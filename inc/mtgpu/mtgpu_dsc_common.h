
#ifndef __MTGPU_DSC_COMMON_H__
#define __MTGPU_DSC_COMMON_H__

#include "os-interface-drm.h"
#include "mtgpu_dp_common.h"
#include "mtgpu_display_debug.h"

#define MT_DSC_VERSION_01       0x01
#define MT_DSC_VERSION_02       0x02

#define DSC_NUM_SYNC_CTRLS	1
#define DSC_CDS_DW_V1		128
#define DSC_CDS_DW_V2		192
#define BITS_PER_BYTE		8

/* VESA Display Stream Compression DSC 1.2 constants */
#define MTGPU_DSC_NUM_BUF_RANGES			15
#define MTGPU_DSC_MUX_WORD_SIZE_8_10_BPC		48
#define MTGPU_DSC_MUX_WORD_SIZE_12_BPC			64
#define MTGPU_DSC_RC_PIXELS_PER_GROUP			3
#define MTGPU_DSC_SCALE_DECREMENT_INTERVAL_MAX		4095
#define MTGPU_DSC_RANGE_BPG_OFFSET_MASK			0x3f

/* MTGPU_DSC Rate Control Constants */
#define MTGPU_DSC_RC_MODEL_SIZE_CONST			8192
#define MTGPU_DSC_RC_EDGE_FACTOR_CONST			6
#define MTGPU_DSC_RC_TGT_OFFSET_HI_CONST		3
#define MTGPU_DSC_RC_TGT_OFFSET_LO_CONST		3

/* MTGPU_DSC PPS constants and macros */
#define MTGPU_DSC_PPS_VERSION_MAJOR_SHIFT		4
#define MTGPU_DSC_PPS_BPC_SHIFT				4
#define MTGPU_DSC_PPS_MSB_SHIFT				8
#define MTGPU_DSC_PPS_LSB_MASK				(0xFF << 0)
#define MTGPU_DSC_PPS_BPP_HIGH_MASK			(0x3 << 8)
#define MTGPU_DSC_PPS_VBR_EN_SHIFT			2
#define MTGPU_DSC_PPS_SIMPLE422_SHIFT			3
#define MTGPU_DSC_PPS_CONVERT_RGB_SHIFT			4
#define MTGPU_DSC_PPS_BLOCK_PRED_EN_SHIFT		5
#define MTGPU_DSC_PPS_INIT_XMIT_DELAY_HIGH_MASK		(0x3 << 8)
#define MTGPU_DSC_PPS_SCALE_DEC_INT_HIGH_MASK		(0xF << 8)
#define MTGPU_DSC_PPS_RC_TGT_OFFSET_HI_SHIFT		4
#define MTGPU_DSC_PPS_RC_RANGE_MINQP_SHIFT		11
#define MTGPU_DSC_PPS_RC_RANGE_MAXQP_SHIFT		6
#define MTGPU_DSC_PPS_NATIVE_420_SHIFT			1
#define MTGPU_DSC_1_2_MAX_LINEBUF_DEPTH_BITS		16
#define MTGPU_DSC_1_2_MAX_LINEBUF_DEPTH_VAL		0
#define MTGPU_DSC_1_1_MAX_LINEBUF_DEPTH_BITS		13

#define DSC_PPR_MP_1_SLICE  340000000
#define DSC_PPR_MP_2_SLICE  680000000
#define DSC_PPR_MP_4_SLICE  1360000000
#define DSC_PPR_MP_8_SLICE  3200000000
#define DSC_PPR_MP_12_SLICE 4800000000
#define DSC_PPR_MP_16_SLICE 6400000000
#define DSC_PPR_MP_20_SLICE 8000000000
#define DSC_PPR_MP_24_SLICE 9600000000

struct mtgpu_dsc_rc_range_parameters {
	u8 range_min_qp;
	u8 range_max_qp;
	u8 range_bpg_offset;
};

struct mtgpu_rc_parameters {
	u16 initial_xmit_delay;
	u8 first_line_bpg_offset;
	u16 initial_offset;
	u8 flatness_min_qp;
	u8 flatness_max_qp;
	u8 rc_quant_incr_limit0;
	u8 rc_quant_incr_limit1;
	struct mtgpu_dsc_rc_range_parameters rc_range_params[MTGPU_DSC_NUM_BUF_RANGES];
};

struct mtgpu_dsc_config {
	u8 line_buf_depth;
	u8 bits_per_component;
	bool convert_rgb;
	u8 slice_count;
	u16 slice_width;
	u16 slice_height;
	bool simple_422;
	u16 pic_width;
	u16 pic_height;
	u8 rc_tgt_offset_high;
	u8 rc_tgt_offset_low;
	u16 bits_per_pixel;
	u8 rc_edge_factor;
	u8 rc_quant_incr_limit1;
	u8 rc_quant_incr_limit0;
	u16 initial_xmit_delay;
	u16  initial_dec_delay;
	bool block_pred_enable;
	u8 first_line_bpg_offset;
	u16 initial_offset;
	u16 rc_buf_thresh[MTGPU_DSC_NUM_BUF_RANGES - 1];
	struct mtgpu_dsc_rc_range_parameters rc_range_params[MTGPU_DSC_NUM_BUF_RANGES];
	u16 rc_model_size;
	u8 flatness_min_qp;
	u8 flatness_max_qp;
	u8 initial_scale_value;
	u16 scale_decrement_interval;
	u16 scale_increment_interval;
	u16 nfl_bpg_offset;
	u16 slice_bpg_offset;
	u16 final_offset;
	bool vbr_enable;
	u8 mux_word_size;
	u16 slice_chunk_size;
	u16 rc_bits;
	u8 dsc_version_minor;
	u8 dsc_version_major;
	bool native_422;
	bool native_420;
	u8 second_line_bpg_offset;
	u16 nsl_bpg_offset;
	u16 second_line_offset_adj;
};

struct mtgpu_dsc_picture_parameter_set {
	u8 dsc_version;
	u8 pps_identifier;
	u8 pps_reserved;
	u8 pps_3;
	u8 pps_4;
	u8 bits_per_pixel_low;
	u16 pic_height;
	u16 pic_width;
	u16 slice_height;
	u16 slice_width;
	u16 chunk_size;
	u8 initial_xmit_delay_high;
	u8 initial_xmit_delay_low;
	u16 initial_dec_delay;
	u8 pps20_reserved;
	u8 initial_scale_value;
	u16 scale_increment_interval;
	u8 scale_decrement_interval_high;
	u8 scale_decrement_interval_low;
	u8 pps26_reserved;
	u8 first_line_bpg_offset;
	u16 nfl_bpg_offset;
	u16 slice_bpg_offset;
	u16 initial_offset;
	u16 final_offset;
	u8 flatness_min_qp;
	u8 flatness_max_qp;
	u16 rc_model_size;
	u8 rc_edge_factor;
	u8 rc_quant_incr_limit0;
	u8 rc_quant_incr_limit1;
	u8 rc_tgt_offset;
	u8 rc_buf_thresh[MTGPU_DSC_NUM_BUF_RANGES - 1];
	u16 rc_range_parameters[MTGPU_DSC_NUM_BUF_RANGES];
	u8 native_422_420;
	u8 second_line_bpg_offset;
	u16 nsl_bpg_offset;
	u16 second_line_offset_adj;
	u32 pps_long_94_reserved;
	u32 pps_long_98_reserved;
	u32 pps_long_102_reserved;
	u32 pps_long_106_reserved;
	u32 pps_long_110_reserved;
	u32 pps_long_114_reserved;
	u32 pps_long_118_reserved;
	u32 pps_long_122_reserved;
	u16 pps_short_126_reserved;
} __packed;

struct slice_select_t {
	u32 slice_num;
	u32 nslc;
};

struct mtgpu_dsc_param {
	bool dsc_en;
	u32 compressed_bpp;
	u32 slice_width;
	u32 slice_height;
	u32 pps_packed[32];
};

struct mtgpu_dsc_ctx {
	void __iomem *regs;
	void __iomem *glb_regs;
	void __iomem *crg_regs;
	u32 id;
	u8  version;
	struct videomode *vm;
	struct mtgpu_dsc_param dsc_param;
};

struct mtgpu_dsc {
	struct mtgpu_dsc_ctx ctx;
	struct mtgpu_dsc_ops *core;
	struct mtgpu_dsc_glb_ops *glb;
	struct device *dev;
};

struct mtgpu_dsc_ops {
	int (*init)(struct mtgpu_dsc_ctx *ctx);
	void (*enable)(struct mtgpu_dsc_ctx *ctx);
	void (*disable)(struct mtgpu_dsc_ctx *ctx);
	void (*deinit)(struct mtgpu_dsc_ctx *ctx);
	u32 (*isr)(struct mtgpu_dsc_ctx *ctx);
	void (*valid_check)(struct mtgpu_dsc_ctx *ctx);
	void (*custom_set)(struct mtgpu_dsc_ctx *ctx);
	void (*pps_set)(struct mtgpu_dsc_ctx *ctx);
};

struct mtgpu_dsc_glb_ops {
	void (*enable)(struct mtgpu_dsc_ctx *ctx, bool dsc_en);
};

struct mtgpu_dsc_chip {
	struct mtgpu_dsc_ops *core;
	struct mtgpu_dsc_glb_ops *glb;
};

static inline
void dsc_reg_write(struct mtgpu_dsc_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->regs + offset);
	os_readl(ctx->regs + offset);
	DSC_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
}

static inline
u32 dsc_reg_read(struct mtgpu_dsc_ctx *ctx, int offset)
{
	u32 val = os_readl(ctx->regs + offset);

	DSC_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
	return val;
}

static inline
void dsc_reg_set(struct mtgpu_dsc_ctx *ctx, int offset, u32 bits)
{
	u32 reg_val = dsc_reg_read(ctx, offset);

	dsc_reg_write(ctx, offset, reg_val | bits);
}

static inline
void dsc_reg_clr(struct mtgpu_dsc_ctx *ctx, int offset, u32 bits)
{
	u32 reg_val = dsc_reg_read(ctx, offset);

	dsc_reg_write(ctx, offset, reg_val & ~bits);
}

static inline
void dsc_glb_reg_write(struct mtgpu_dsc_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->glb_regs + offset);
	os_readl(ctx->glb_regs + offset);
	GLB_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
}

static inline
u32 dsc_glb_reg_read(struct mtgpu_dsc_ctx *ctx, int offset)
{
	u32 val = os_readl(ctx->glb_regs + offset);

	GLB_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
	return val;
}

static inline
void dsc_crg_reg_write(struct mtgpu_dsc_ctx *ctx, int offset, u32 val)
{
	os_writel(val, ctx->crg_regs + offset);
	os_readl(ctx->crg_regs + offset);

	DSC_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
}

static inline
u32 dsc_crg_reg_read(struct mtgpu_dsc_ctx *ctx, int offset)
{
	u32 val = os_readl(ctx->crg_regs + offset);

	DSC_DBG_REG("offset = 0x%04x value = 0x%08x\n", offset, val);
	return val;
}

void mtgpu_dsc_start(struct mtgpu_dsc *dsc);
void mtgpu_dsc_stop(struct mtgpu_dsc *dsc);
u32 mtgpu_dsc_nslc_get(int index);
int mtgpu_dsc_slice_index_find(u32 slice_num);
int mtgpu_dsc_pps_base_param_set(struct mtgpu_dsc_config *dsc_cfg, struct mtgpu_dp *dp,
				 u16 rgb_compressed_bpp);
void mtgpu_get_dsc_config(struct drm_encoder *encoder,
			  struct drm_crtc *crtc,
			  struct mtgpu_dsc_config *mtgpu_dsc_cfg);
int mtgpu_dsc_compute_rc_parameters(struct mtgpu_dsc_config *mtgpu_dsc_cfg);
void mtgpu_dsc_pps_payload_pack(struct mtgpu_dsc_picture_parameter_set *mtgpu_pps_payload,
				const struct mtgpu_dsc_config *mtgpu_dsc_cfg);
const struct mtgpu_rc_parameters *mtgpu_dsc_get_rc_params(u16 rgb_compressed_bpp,
							  u8 bits_per_component);
void mtgpu_dsc_param_save(struct mtgpu_dp *dp, struct mtgpu_dsc *dsc,
			  struct mtgpu_dsc_config *dsc_cfg, u32 *pps_packed);
void mtgpu_dsc_param_clear(struct drm_encoder *encoder,
			   struct drm_crtc *crtc);
void mtgpu_dsc_error_handle(struct mtgpu_dsc *dsc);

extern struct mtgpu_dsc_chip mtgpu_dsc_qy1;
extern struct mtgpu_dsc_chip mtgpu_dsc_qy2;

#endif
