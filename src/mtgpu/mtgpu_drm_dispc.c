/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/component.h>
#include <linux/platform_device.h>

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#endif
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_plane_helper.h>
#if defined(OS_DRM_DRM_DP_HELPER_H_EXIST)
#include <drm/drm_dp_helper.h>
#elif defined(OS_DRM_DP_DRM_DP_HELPER_H_EXIST)
#include <drm/dp/drm_dp_helper.h>
#elif defined(OS_DRM_DISPLAY_DRM_DP_HELPER_H_EXIST)
#include <drm/display/drm_dp_helper.h>
#endif
#if defined(OS_DRM_DRM_PROBE_HELPER_H_EXIST)
#include <drm/drm_probe_helper.h>
#endif
#include <drm/drm_vblank.h>
#include <drm/drm_color_mgmt.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_blend.h>
#include <video/videomode.h>

#include "mtgpu_drm_gem.h"
#include "mtgpu_dispc_common.h"
#include "mtgpu_drm_debugfs.h"
#include "mtgpu_drm_internal.h"
#include "../mtgpu/mtgpu_drv.h"
#include "os-interface-drm.h"

static inline struct mtgpu_dispc *to_mtgpu_dispc(struct drm_crtc *crtc)
{
	return os_get_drm_crtc_drvdata(crtc);
}

static inline bool mtgpu_is_crtc_active(struct drm_crtc *crtc)
{
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
	return crtc->state->active;
#else
	return crtc->enabled;
#endif
}

static void mtgpu_get_layer_config(struct drm_plane_state *state,
				   struct mtgpu_layer_config *config)
{
	struct drm_framebuffer *fb = state->fb;
	int i;

	config->src_x = state->src_x >> 16;
	config->src_y = state->src_y >> 16;
	config->src_w = state->src_w >> 16;
	config->src_h = state->src_h >> 16;
	config->dst_x = state->crtc_x;
	config->dst_y = state->crtc_y;
	config->dst_w = state->crtc_w;
	config->dst_h = state->crtc_h;
	config->alpha = state->alpha >> 8;
	config->rotation = state->rotation;
#if defined(OS_STRUCT_DRM_PLANE_STATE_HAS_PIXEL_BLEND_MODE)
	config->blend_mode = state->pixel_blend_mode;
#endif
	config->color_encoding = state->color_encoding;
	config->color_range = state->color_range;
	config->num_planes = fb->format->num_planes;
	config->format = fb->format->format;
	config->is_fbc = !!fb->modifier;

	for (i = 0; i < fb->format->num_planes; i++) {
		config->addr[i] = mtgpu_fb_get_dma_addr(fb, state, i);
		config->pitch[i] = fb->pitches[i];
	}
}

#if defined(OS_DRM_PLANE_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
static void mtgpu_plane_atomic_update(struct drm_plane *plane,
				      struct drm_atomic_state *atomic_state)
#else
static void mtgpu_plane_atomic_update(struct drm_plane *plane,
				      struct drm_plane_state *old_state)
#endif
{
	struct drm_plane_state *state = plane->state;
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(state->crtc);
	struct mtgpu_layer_config layer = {};
	struct drm_framebuffer *fb = state->fb;
	u8 index = state->normalized_zpos;

	mtgpu_get_layer_config(state, &layer);

	DRM_DEV_DEBUG(dispc->dev, "SRC_X = %d, SRC_Y = %d, SRC_W = %d, SRC_H = %d\n",
		      layer.src_x, layer.src_y, layer.src_w, layer.src_h);
	DRM_DEV_DEBUG(dispc->dev, "CRTC_X = %d, CRTC_Y = %d, CRTC_W = %d, CRTC_H = %d\n",
		      layer.dst_x, layer.dst_y, layer.dst_w, layer.dst_h);
	DRM_DEV_DEBUG(dispc->dev, "layer->addr[0] = 0x%llx, layer->pitch[0] = 0x%08x\n",
		      layer.addr[0], layer.pitch[0]);

	/* first enable fbc and then dma addr, otherwise dpc may underflow */
	if (fb->modifier && dispc->glb->fbc_enable)
		dispc->glb->fbc_enable(&dispc->ctx, &layer, index);
	else if (dispc->glb->fbc_disable)
		dispc->glb->fbc_disable(&dispc->ctx, index);

	if (dispc->core->cursor_config && plane->type == DRM_PLANE_TYPE_CURSOR)
		dispc->core->cursor_config(&dispc->ctx, &layer);
	else if (dispc->core->layer_config)
		dispc->core->layer_config(&dispc->ctx, &layer, index);
}

#if defined(OS_DRM_PLANE_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
static void mtgpu_plane_atomic_disable(struct drm_plane *plane,
				       struct drm_atomic_state *state)
{
	struct drm_plane_state *old_state = drm_atomic_get_old_plane_state(state, plane);
#else
static void mtgpu_plane_atomic_disable(struct drm_plane *plane,
				       struct drm_plane_state *old_state)
{
#endif
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(old_state->crtc);
	u8 index = old_state->normalized_zpos;

	DRM_DEV_INFO(dispc->dev, "%s()\n", __func__);

	if (dispc->core->cursor_config && plane->type == DRM_PLANE_TYPE_CURSOR)
		dispc->core->cursor_config(&dispc->ctx, NULL);
	else if (dispc->core->layer_disable)
		dispc->core->layer_disable(&dispc->ctx, index);

	if (dispc->glb->fbc_disable)
		dispc->glb->fbc_disable(&dispc->ctx, index);
}

static void mtgpu_plane_destroy(struct drm_plane *plane)
{
	DRM_INFO("%s()\n", __func__);

	drm_plane_cleanup(plane);
	kfree(plane);
}

static bool mtgpu_format_mod_supported(struct drm_plane *plane,
				       u32 format,
				       u64 modifier)
{
	struct drm_plane_state *state = plane->state;
	struct mtgpu_dispc *dispc;
	u8 index;

	/* before atomic commit, state or crtc is NULL */
	if (!state || !state->crtc || !plane->crtc)
		return true;

	dispc = to_mtgpu_dispc(state->crtc);
	index = state->normalized_zpos;
	if (dispc->core->fbc_validate)
		return dispc->core->fbc_validate(&dispc->ctx,
						 format,
						 modifier,
						 index);

	return true;
}

#if defined(OS_DRM_PLANE_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
static int mtgpu_plane_atomic_async_check(struct drm_plane *plane,
					  struct drm_atomic_state *atomic_state)
#else
static int mtgpu_plane_atomic_async_check(struct drm_plane *plane,
					  struct drm_plane_state *plane_state)
#endif
{
	if (plane->type == DRM_PLANE_TYPE_CURSOR)
		return 0;

	return -EINVAL;
}

#if defined(OS_DRM_PLANE_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
static void mtgpu_plane_atomic_async_update(struct drm_plane *plane,
					    struct drm_atomic_state *atomic_state)
{
	struct drm_plane_state *new_state =
		drm_atomic_get_new_plane_state(atomic_state, plane);
#else
static void mtgpu_plane_atomic_async_update(struct drm_plane *plane,
					    struct drm_plane_state *new_state)
{
#endif
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(new_state->crtc);
	struct mtgpu_layer_config config = {};

	mtgpu_get_layer_config(new_state, &config);

	swap(plane->state->fb, new_state->fb);

	plane->state->src_x = new_state->src_x;
	plane->state->src_y = new_state->src_y;
	plane->state->src_w = new_state->src_w;
	plane->state->src_h = new_state->src_h;
	plane->state->crtc_x = new_state->crtc_x;
	plane->state->crtc_y = new_state->crtc_y;
	plane->state->crtc_w = new_state->crtc_w;
	plane->state->crtc_h = new_state->crtc_h;

	if (dispc->core->cursor_config) {
		if (plane->type == DRM_PLANE_TYPE_CURSOR)
			dispc->core->cursor_config(&dispc->ctx, &config);
	}
}

static const struct drm_plane_helper_funcs mtgpu_primary_helper_funcs = {
	.atomic_update		= mtgpu_plane_atomic_update,
	.atomic_disable		= mtgpu_plane_atomic_disable,
	.atomic_async_check     = mtgpu_plane_atomic_async_check,
	.atomic_async_update    = mtgpu_plane_atomic_async_update,

};

static const struct drm_plane_funcs mtgpu_plane_funcs = {
	.update_plane		= drm_atomic_helper_update_plane,
	.disable_plane		= drm_atomic_helper_disable_plane,
	.destroy		= mtgpu_plane_destroy,
	.reset			= drm_atomic_helper_plane_reset,
	.atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_plane_destroy_state,
	.format_mod_supported   = mtgpu_format_mod_supported,
};

static void mtgpu_crtc_mode_set_nofb(struct drm_crtc *crtc)
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	drm_display_mode_to_videomode(&crtc->state->adjusted_mode, dispc->ctx.vm);
}

static enum drm_mode_status
mtgpu_crtc_mode_valid(struct drm_crtc *crtc,
		      const struct drm_display_mode *mode)
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);
	DRM_DEV_DEBUG(dispc->dev, DRM_MODE_FMT "\n", DRM_MODE_ARG(mode));

	/* check if the mode can be supported by DC. */
	if (dispc->core->mode_valid) {
		struct videomode vm;

		drm_display_mode_to_videomode(mode, &vm);
		return dispc->core->mode_valid(&dispc->ctx, &vm);
	}

	return MODE_OK;
}

static bool mtgpu_crtc_mode_fixup(struct drm_crtc *crtc,
				  const struct drm_display_mode *mode,
				  struct drm_display_mode *adjusted_mode)
{
	return true;
}

static void mtgpu_crtc_atomic_enable(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				     struct drm_atomic_state *old_state)
#else
				     struct drm_crtc_state *old_state)
#endif
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);
	u16 *red = crtc->gamma_store;
	u16 *green = red + crtc->gamma_size;
	u16 *blue = green + crtc->gamma_size;

	DRM_DEV_INFO(dispc->dev, DRM_MODE_FMT "\n",
		     DRM_MODE_ARG(&crtc->state->adjusted_mode));

	if (dispc->glb->enable)
		dispc->glb->enable(&dispc->ctx);

	if (dispc->core->init)
		dispc->core->init(&dispc->ctx);

	if (dispc->core->gamma_set)
		dispc->core->gamma_set(&dispc->ctx, red, green, blue, crtc->gamma_size);

	dispc->debugfs.underrun_cnt = 0;

	drm_crtc_vblank_on(crtc);
}

static void mtgpu_crtc_atomic_disable(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				      struct drm_atomic_state *old_state)
#else
				      struct drm_crtc_state *old_state)
#endif
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);
	unsigned long flags;

	DRM_DEV_DEBUG(dispc->dev, "%s()\n", __func__);

	drm_crtc_vblank_off(crtc);

	if (dispc->core->deinit)
		dispc->core->deinit(&dispc->ctx);

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	if (crtc->state->event) {
		drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
	}
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

	if (dispc->glb->disable)
		dispc->glb->disable(&dispc->ctx);
}

static void mtgpu_crtc_atomic_begin(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				    struct drm_atomic_state *old_state)
#else
				    struct drm_crtc_state *old_state)
#endif
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	DRM_DEV_DEBUG(dispc->dev, "%s()\n", __func__);

	if (dispc->core->config_begin)
		dispc->core->config_begin(&dispc->ctx);
}

static void mtgpu_crtc_atomic_flush(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				    struct drm_atomic_state *old_state)
#else
				    struct drm_crtc_state *old_state)
#endif
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);
	unsigned long flags;

	DRM_DEV_DEBUG(dispc->dev, "%s()\n", __func__);

	if (dispc->core->config_end)
		dispc->core->config_end(&dispc->ctx);

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	if (crtc->state->event) {

		if (drm_crtc_vblank_get(crtc) != 0)
			drm_crtc_send_vblank_event(crtc, crtc->state->event);
		else
			drm_crtc_arm_vblank_event(crtc, crtc->state->event);

		crtc->state->event = NULL;
	}
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}

static const struct drm_crtc_helper_funcs mtgpu_crtc_helper_funcs = {
	.mode_set_nofb	= mtgpu_crtc_mode_set_nofb,
	.mode_valid	= mtgpu_crtc_mode_valid,
	.mode_fixup	= mtgpu_crtc_mode_fixup,
	.atomic_begin	= mtgpu_crtc_atomic_begin,
	.atomic_flush	= mtgpu_crtc_atomic_flush,
	.atomic_enable	= mtgpu_crtc_atomic_enable,
	.atomic_disable	= mtgpu_crtc_atomic_disable,
};

static int mtgpu_crtc_enable_vblank(struct drm_crtc *crtc)
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	DRM_DEV_DEBUG(dispc->dev, "%s()\n", __func__);

	if (dispc->core->vsync_on)
		dispc->core->vsync_on(&dispc->ctx);

	return 0;
}

static void mtgpu_crtc_disable_vblank(struct drm_crtc *crtc)
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	DRM_DEV_DEBUG(dispc->dev, "%s()\n", __func__);

	if (dispc->core->vsync_off)
		dispc->core->vsync_off(&dispc->ctx);
}

static void mtgpu_crtc_destroy(struct drm_crtc *crtc)
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	DRM_DEV_INFO(dispc->dev, "%s()\n", __func__);

	drm_crtc_cleanup(crtc);
}

static int mtgpu_crtc_gamma_set(struct drm_crtc *crtc, u16 *red, u16 *green,
				 u16 *blue, uint32_t size,
				 struct drm_modeset_acquire_ctx *ctx)
{
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	if (dispc->core->gamma_set)
		dispc->core->gamma_set(&dispc->ctx, red, green, blue, size);

	return 0;
}

#if defined(OS_FUNC_DRM_GEM_OBJECT_PUT_UNLOCKED_EXIST)
#define drm_gem_object_put(obj) drm_gem_object_put_unlocked(obj)
#endif

int mtgpu_legacy_cursor_set(struct drm_crtc *crtc, struct drm_file *file,
				   u32 handle, u32 width, u32 height)
{
	struct mtgpu_gem_object *mt_obj;
	struct drm_gem_object *cursor_bo = NULL;
	struct mtgpu_layer_config config = {0};
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	if (!mtgpu_is_crtc_active(crtc))
		return 0;

	/* handle 0 means cursor hide */
	if (!handle && dispc->core->cursor_config) {
		dispc->cursor_info.dev_addr = 0;
		dispc->cursor_info.width = 0;
		dispc->cursor_info.height = 0;
		dispc->core->cursor_config(&dispc->ctx, NULL);
		return 0;
	}

	cursor_bo = drm_gem_object_lookup(file, handle);
	if (!cursor_bo)
		return -ENOENT;

	mt_obj = os_get_drm_gem_object_drvdata(cursor_bo);
	if (!mt_obj->dev_addr) {
		drm_gem_object_put(cursor_bo);
		return -EINVAL;
	}

	if (cursor_bo->size < width * height * 4) {
		DRM_DEV_ERROR(dispc->dev, " cursor buffer is too small!\n");
		drm_gem_object_put(cursor_bo);
		return -ENOMEM;
	}

	if (dispc->cursor_info.bo)
		drm_gem_object_put(dispc->cursor_info.bo);

	dispc->cursor_info.bo = cursor_bo;
	config.addr[0] = mt_obj->dev_addr;
	config.dst_x   = dispc->cursor_info.x;
	config.dst_y   = dispc->cursor_info.y;
	config.dst_w   = width;
	config.dst_h   = height;
	config.src_w   = width;
	config.src_h   = height;
	config.pitch[0] = width * 4;
	config.alpha   = 255;
	config.blend_mode = 0;
	config.format = DRM_FORMAT_ARGB8888;

	dispc->cursor_info.dev_addr = mt_obj->dev_addr;
	dispc->cursor_info.width = width;
	dispc->cursor_info.height = height;

	if (dispc->core->cursor_config)
		dispc->core->cursor_config(&dispc->ctx, &config);

	return 0;
}

int mtgpu_legacy_cursor_move(struct drm_crtc *crtc, int x, int y)
{
	struct mtgpu_layer_config config = {0};
	struct mtgpu_dispc *dispc = to_mtgpu_dispc(crtc);

	if (!mtgpu_is_crtc_active(crtc))
		return 0;

	config.addr[0] = dispc->cursor_info.dev_addr;
	config.dst_x   = x;
	config.dst_y   = y;
	config.dst_w   = dispc->cursor_info.width;
	config.dst_h   = dispc->cursor_info.height;
	config.src_w   = dispc->cursor_info.width;
	config.src_h   = dispc->cursor_info.height;
	config.pitch[0] = dispc->cursor_info.width * 4;
	config.alpha   = 255;
	config.blend_mode = 0;
	config.format = DRM_FORMAT_ARGB8888;

	dispc->cursor_info.x = x;
	dispc->cursor_info.y = y;

	if ((!config.addr[0] || !config.dst_w || !config.dst_h) &&
	    dispc->core->cursor_config) {
		dispc->core->cursor_config(&dispc->ctx, NULL);
		return 0;
	}

	if (dispc->core->cursor_config)
		dispc->core->cursor_config(&dispc->ctx, &config);

	return 0;
}

static const struct drm_crtc_funcs mtgpu_crtc_funcs = {
	.set_config             = drm_atomic_helper_set_config,
	.destroy                = mtgpu_crtc_destroy,
	.page_flip              = drm_atomic_helper_page_flip,
	.reset                  = drm_atomic_helper_crtc_reset,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state   = drm_atomic_helper_crtc_destroy_state,
	.enable_vblank		= mtgpu_crtc_enable_vblank,
	.disable_vblank		= mtgpu_crtc_disable_vblank,
	.gamma_set		= mtgpu_crtc_gamma_set,
	// .cursor_set		= mtgpu_legacy_cursor_set,
	// .cursor_move		= mtgpu_legacy_cursor_move,
};

static void mtgpu_plane_create_properties(struct drm_plane *plane,
					  const struct mtgpu_layer_capability *layer_cap,
					  int index)
{
#if defined(OS_FUNC_DRM_PLANE_CREATE_BLEND_MODE_PROPERTY_EXIST)
	u32 supported_modes = BIT(DRM_MODE_BLEND_PIXEL_NONE) |
			      BIT(DRM_MODE_BLEND_PREMULTI) |
			      BIT(DRM_MODE_BLEND_COVERAGE);
	drm_plane_create_blend_mode_property(plane, supported_modes);
#endif

	drm_plane_create_rotation_property(plane,
					   DRM_MODE_ROTATE_0,
					   DRM_MODE_ROTATE_MASK |
					   DRM_MODE_REFLECT_MASK);

	drm_plane_create_alpha_property(plane);

	drm_plane_create_zpos_immutable_property(plane, index);

	if (layer_cap->supported_encodings && layer_cap->supported_ranges)
		drm_plane_create_color_properties(plane,
						  layer_cap->supported_encodings,
						  layer_cap->supported_ranges,
						  DRM_COLOR_YCBCR_BT709,
						  DRM_COLOR_YCBCR_LIMITED_RANGE);
}

static void mtgpu_dispc_isr(void *data)
{
	struct mtgpu_dispc *dispc = data;
	u32 int_sts = 0;

	if (dispc->core->isr)
		int_sts = dispc->core->isr(&dispc->ctx);

	if (int_sts & DISPC_INT_BIT_VSYNC)
		drm_crtc_handle_vblank(dispc->crtc);

	if (int_sts & DISPC_INT_BIT_UNDERRUN)
		dispc->debugfs.underrun_cnt++;
}

static int mtgpu_dispc_component_bind(struct device *dev,
				      struct device *master, void *data)
{
	struct drm_device *drm = data;
	struct drm_plane *primary = NULL, *cursor = NULL;
	struct mtgpu_dispc *dispc;
	struct mtgpu_dispc_capability dispc_cap = {};
	const struct mtgpu_layer_capability *layer_caps;
	struct platform_device *pdev = to_platform_device(dev);
	struct resource *res;
	struct mtgpu_dispc_platform_data *pdata = dev_get_platdata(dev);
	int i, ret;
	struct mtgpu_dispc_chip *chip;

	dispc = kzalloc(sizeof(*dispc), GFP_KERNEL);
	if (!dispc)
		return -ENOMEM;

	dispc->ctx.vm = kzalloc(sizeof(*dispc->ctx.vm), GFP_KERNEL);
	if (!dispc->ctx.vm) {
		DRM_DEV_ERROR(dev, "failed to create vm\n");
		ret = -ENOMEM;
		goto err_free_dispc;
	}

	dispc->ctx.waitq = kzalloc(sizeof(*dispc->ctx.waitq), GFP_KERNEL);
	if (!dispc->ctx.waitq) {
		DRM_DEV_ERROR(dev, "failed to create waitq\n");
		ret = -ENOMEM;
		goto err_free_dispc;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dispc-regs");
	if (!res) {
		DRM_DEV_ERROR(dev, "failed to get dispc-regs\n");
		ret = -EIO;
		goto err_free_dispc;
	}

	dispc->ctx.regs = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(dispc->ctx.regs)) {
		ret = PTR_ERR(dispc->ctx.regs);
		goto err_free_dispc;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "glb-regs");
	if (!res) {
		DRM_DEV_ERROR(dev, "failed to get display glb-regs\n");
		ret = -EIO;
		goto err_free_dispc;
	}

	dispc->ctx.glb_regs = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(dispc->ctx.glb_regs)) {
		ret = PTR_ERR(dispc->ctx.glb_regs);
		goto err_free_dispc;
	}

	dispc->ctx.cursor_mem_base = pdata->cursor_mem_base;
	dispc->ctx.cursor_mem_size = pdata->cursor_mem_size;
	dispc->ctx.pcie_mem_base = pdata->pcie_mem_base;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		DRM_DEV_ERROR(dev, "failed to get display irq number\n");
		ret = -EIO;
		goto err_free_dispc;
	}

	ret = mtgpu_set_interrupt_handler(dev->parent->parent, res->start, mtgpu_dispc_isr, dispc);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to register dispc irq handler\n");
		ret = -EINVAL;
		goto err_free_dispc;
	}

	dispc->ctx.irq = res->start;
	dispc->ctx.id = pdata->id;
	dispc->dev = dev;
	dev_set_drvdata(dev, dispc);
	init_waitqueue_head(dispc->ctx.waitq);

	switch (pdata->soc_gen) {
	case GPU_SOC_GEN1:
		chip = &mtgpu_dispc_sudi;
		break;
	case GPU_SOC_GEN2:
		chip = &mtgpu_dispc_qy1;
		break;
	case GPU_SOC_GEN3:
		chip = &mtgpu_dispc_qy2;
		break;
	default:
		DRM_DEV_ERROR(dev, "current SOC_GEN%d is not supported\n", pdata->soc_gen);
		ret = -ENOTSUPP;
		goto err_free_dispc;
	}

	dispc->core = chip->core;
	if (!dispc->core) {
		DRM_DEV_ERROR(dev, "dispc ops is null\n");
		ret = -EINVAL;
		goto err_free_dispc;
	}

	if (dispc->core->ctx_init) {
		ret = dispc->core->ctx_init(&dispc->ctx);
		if (ret)
			goto err_free_dispc;
	}

	/* disable de configs like interrupts as firmware may config them,
	 * which may lead OOPS in irq handler as drm has not finished
	 * initial.
	 */
	if (dispc->core->deinit)
		dispc->core->deinit(&dispc->ctx);

	ret = mtgpu_enable_interrupt(dev->parent->parent, res->start);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to enable dispc irq number: %d\n", (int)res->start);
		ret = -EINVAL;
		goto err_free_dispc;
	}

	dispc->glb = chip->glb;
	if (!dispc->glb) {
		DRM_ERROR("dispc glb ops is null\n");
		ret = -EINVAL;
		goto err_ctx_deinit;
	}

	if (dispc->glb->init)
		dispc->glb->init(&dispc->ctx);

	if (!dispc->core->capability) {
		DRM_DEV_ERROR(dev, "dispc capability() callback is null\n");
		ret = -EINVAL;
		goto err_ctx_deinit;
	}

	dispc->core->capability(&dispc->ctx, &dispc_cap);
	if (!dispc_cap.layer_count) {
		DRM_DEV_ERROR(dev, "dispc layer count is 0\n");
		ret = -EINVAL;
		goto err_ctx_deinit;
	}

	layer_caps = dispc_cap.layer_caps;

	for (i = 0; i < dispc_cap.layer_count; i++) {
		struct drm_plane *plane;

	/* skip register cursor plane as for some old kernel versions,
	 * async update cursor maybe check fail,so we need add legacy
	 * cursor interfaces instead of update plane.
	 */
#if (KERNEL_VERSION(5, 1, 9) >= LINUX_VERSION_CODE)
		if (layer_caps[i].type == DRM_PLANE_TYPE_CURSOR)
			continue;
#endif

		plane = kzalloc(sizeof(*plane), GFP_KERNEL);
		if (!plane) {
			ret = -ENOMEM;
			goto err_ctx_deinit;
		}

		ret = drm_universal_plane_init(drm, plane, 0xff,
					       &mtgpu_plane_funcs,
					       layer_caps[i].fmts_ptr,
					       layer_caps[i].fmts_cnt,
					       layer_caps[i].modifiers,
					       layer_caps[i].type, NULL);
		if (ret) {
			DRM_DEV_ERROR(dev, "failed to create plane\n");
			kfree(plane);
			goto err_ctx_deinit;
		}

		drm_plane_helper_add(plane, &mtgpu_primary_helper_funcs);

		mtgpu_plane_create_properties(plane, &layer_caps[i], i);

		if (i == 0)
			primary = plane;
		else if (layer_caps[i].type == DRM_PLANE_TYPE_CURSOR)
			cursor = plane;
	}

	dispc->crtc = os_create_drm_crtc();
	if (!dispc->crtc)
		goto err_ctx_deinit;

	os_set_drm_crtc_drvdata(dispc->crtc, dispc);
	ret = drm_crtc_init_with_planes(drm, dispc->crtc, primary, cursor,
					&mtgpu_crtc_funcs, NULL);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to init CRTC\n");
		goto err_crtc_free;
	}

	drm_mode_crtc_set_gamma_size(dispc->crtc, dispc_cap.gamma_size);

	drm_crtc_helper_add(dispc->crtc, &mtgpu_crtc_helper_funcs);

	mtgpu_dispc_debugfs_create_files(dispc);

	DRM_DEV_INFO(dev, "mtgpu display controller driver loaded successfully\n");

	return 0;

err_crtc_free:
	os_destroy_drm_crtc(dispc->crtc);

err_ctx_deinit:
	if (dispc->core->ctx_deinit)
		dispc->core->ctx_deinit(&dispc->ctx);
err_free_dispc:
	kfree(dispc->ctx.waitq);
	kfree(dispc->ctx.vm);
	kfree(dispc);

	return ret;
}

static void mtgpu_dispc_component_unbind(struct device *dev,
					 struct device *master, void *data)
{
	int ret;
	struct mtgpu_dispc *dispc = dev_get_drvdata(dev);

	ret = mtgpu_disable_interrupt(dev->parent->parent, dispc->ctx.irq);
	if (ret)
		DRM_DEV_ERROR(dev, "failed to disable dispc irq %d\n", dispc->ctx.irq);

	ret = mtgpu_set_interrupt_handler(dev->parent->parent, dispc->ctx.irq, NULL, NULL);
	if (ret)
		DRM_DEV_ERROR(dev, "failed to deregister dispc irq %d handler\n", dispc->ctx.irq);

	os_destroy_drm_crtc(dispc->crtc);

	if (dispc->core->ctx_deinit)
		dispc->core->ctx_deinit(&dispc->ctx);

	mtgpu_dispc_debugfs_remove_files(dispc);

	kfree(dispc->ctx.waitq);
	kfree(dispc->ctx.vm);
	kfree(dispc);

	DRM_DEV_INFO(dev, "unload mtgpu display controller driver\n");
}

static const struct component_ops mtgpu_dispc_component_ops = {
	.bind   = mtgpu_dispc_component_bind,
	.unbind = mtgpu_dispc_component_unbind,
};

static int mtgpu_dispc_probe(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &mtgpu_dispc_component_ops);
}

static int mtgpu_dispc_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &mtgpu_dispc_component_ops);
	return 0;
}

static struct platform_device_id mtgpu_dispc_device_id[] = {
	{ .name = MTGPU_DEVICE_NAME_DISPC, },
	{ },
};

struct platform_driver mtgpu_dispc_driver = {
	.probe    = mtgpu_dispc_probe,
	.remove   = mtgpu_dispc_remove,
	.driver   = {
		.owner  = THIS_MODULE,
		.name   = "mtgpu-dispc-drv",
	},
	.id_table = mtgpu_dispc_device_id,
};

