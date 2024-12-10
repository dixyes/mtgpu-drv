/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/component.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#endif
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_print.h>
#if defined(OS_DRM_DRM_PROBE_HELPER_H_EXIST)
#include <drm/drm_probe_helper.h>
#endif
#include <drm/drm_vblank.h>

#define drm_crtc_to_dummy_crtc(target) \
	container_of(target, struct dummy_crtc, crtc)

struct dummy_crtc {
	struct drm_crtc crtc;
	u32 vblank_periods;
	struct hrtimer vblank_hrtimer;
	spinlock_t dummy_lock;
};

#define ONE_SECOND_NS		1000000000
#define DUMMY_VREFRESH_MAX	300

static const u32 dummy_formats[] = {
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_YUV420,
	DRM_FORMAT_NV12,
};

static const u32 dummy_cursor_formats[] = {
	DRM_FORMAT_ARGB8888,
};

#if defined(OS_DRM_PLANE_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
static void dummy_plane_atomic_update(struct drm_plane *plane,
				      struct drm_atomic_state *state)
#else
static void dummy_plane_atomic_update(struct drm_plane *plane,
				      struct drm_plane_state *old_state)
#endif
{
	/* we do nothing here */
}

static void dummy_plane_destroy(struct drm_plane *plane)
{
	drm_plane_cleanup(plane);
	kfree(plane);
}

static const struct drm_plane_helper_funcs dummy_primary_helper_funcs = {
	.atomic_update		= dummy_plane_atomic_update,
};

static const struct drm_plane_funcs dummy_plane_funcs = {
	.update_plane		= drm_atomic_helper_update_plane,
	.disable_plane		= drm_atomic_helper_disable_plane,
	.destroy		= dummy_plane_destroy,
	.reset			= drm_atomic_helper_plane_reset,
	.atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_plane_destroy_state,
};

static struct drm_plane *dummy_plane_init(struct drm_device *drm,
				  enum drm_plane_type type, int crtc_index)
{
	const struct drm_plane_helper_funcs *funcs;
	struct drm_plane *plane;
	const u32 *formats;
	int ret, nformats;

	plane = kzalloc(sizeof(*plane), GFP_KERNEL);
	if (!plane)
		return ERR_PTR(-ENOMEM);

	if (type == DRM_PLANE_TYPE_CURSOR) {
		formats = dummy_cursor_formats;
		nformats = ARRAY_SIZE(dummy_cursor_formats);
		funcs = &dummy_primary_helper_funcs;
	} else {
		formats = dummy_formats;
		nformats = ARRAY_SIZE(dummy_formats);
		funcs = &dummy_primary_helper_funcs;
	}

	ret = drm_universal_plane_init(drm, plane, 1 << crtc_index,
				       &dummy_plane_funcs,
				       formats, nformats,
				       NULL, type, NULL);
	if (ret) {
		DRM_ERROR("failed to init plane\n");
		kfree(plane);
		return ERR_PTR(ret);
	}

	drm_plane_helper_add(plane, funcs);

	return plane;
}

static unsigned int dummy_vblank_periods_calculate(struct drm_crtc *crtc)
{
	u32 vblank_periods = 0;
	int vrefresh;

	vrefresh = drm_mode_vrefresh(&crtc->state->mode);
	if (vrefresh)
		vblank_periods = ONE_SECOND_NS / vrefresh;

	return vblank_periods;
}

static enum hrtimer_restart dummy_vblank_simulate(struct hrtimer *timer)
{
	struct dummy_crtc *dummy = container_of(timer, struct dummy_crtc,
						  vblank_hrtimer);
	struct drm_crtc *crtc = &dummy->crtc;
	u64 ret_overrun;
	bool ret;
	unsigned long flags;

	if (!spin_trylock_irqsave(&dummy->dummy_lock, flags)) {
		hrtimer_add_expires_ns(&dummy->vblank_hrtimer,
				       dummy->vblank_periods);
		DRM_WARN("add expires_ns for hrtimer.\n");
		return HRTIMER_RESTART;
	}

	ret = drm_crtc_handle_vblank(crtc);
	if (!ret) {
		DRM_WARN("dummy failure on handling vblank");
		spin_unlock_irqrestore(&dummy->dummy_lock, flags);
		return HRTIMER_NORESTART;
	}

	ret_overrun = hrtimer_forward_now(&dummy->vblank_hrtimer,
					  dummy->vblank_periods);
	if (ret_overrun != 1)
		DRM_WARN("hrtimer_forward return overrun %lld.\n", ret_overrun);

	spin_unlock_irqrestore(&dummy->dummy_lock, flags);

	return HRTIMER_RESTART;
}

static int dummy_enable_vblank(struct drm_crtc *crtc)
{
	struct dummy_crtc *dummy = drm_crtc_to_dummy_crtc(crtc);

	DRM_DEBUG("Enable software vsync timer\n");

	hrtimer_init(&dummy->vblank_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dummy->vblank_hrtimer.function = &dummy_vblank_simulate;
	dummy->vblank_periods = dummy_vblank_periods_calculate(crtc);

	if (dummy->vblank_periods)
		hrtimer_start(&dummy->vblank_hrtimer, dummy->vblank_periods, HRTIMER_MODE_REL);

	return 0;
}

static void dummy_disable_vblank(struct drm_crtc *crtc)
{
	struct dummy_crtc *dummy = drm_crtc_to_dummy_crtc(crtc);
	unsigned long flags;

	DRM_DEBUG("Disable software vsync timer\n");

	if (!spin_trylock_irqsave(&dummy->dummy_lock, flags))
		return;

	hrtimer_cancel(&dummy->vblank_hrtimer);

	spin_unlock_irqrestore(&dummy->dummy_lock, flags);
}

static void dummy_crtc_destroy(struct drm_crtc *crtc)
{
	struct dummy_crtc *dummy = drm_crtc_to_dummy_crtc(crtc);

	drm_crtc_cleanup(crtc);
	kfree(dummy);
}

static const struct drm_crtc_funcs dummy_crtc_funcs = {
	.set_config             = drm_atomic_helper_set_config,
	.destroy                = dummy_crtc_destroy,
	.page_flip              = drm_atomic_helper_page_flip,
	.reset                  = drm_atomic_helper_crtc_reset,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state   = drm_atomic_helper_crtc_destroy_state,
	.enable_vblank		= dummy_enable_vblank,
	.disable_vblank		= dummy_disable_vblank,
};

static enum drm_mode_status dummy_crtc_mode_valid(struct drm_crtc *crtc,
						  const struct drm_display_mode *mode)
{
	int vrefresh = drm_mode_vrefresh(mode);

	if (vrefresh <= 0 || vrefresh > DUMMY_VREFRESH_MAX) {
		DRM_ERROR("invalid vrefresh %d, the range of vrefresh should be (0,%d]\n",
			  vrefresh, DUMMY_VREFRESH_MAX);
		return MODE_BAD_VSCAN;
	}

	return MODE_OK;
}

static void dummy_crtc_atomic_enable(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				    struct drm_atomic_state *old_state)
#else
				    struct drm_crtc_state *old_state)
#endif
{
	drm_crtc_vblank_on(crtc);
}

static void dummy_crtc_atomic_disable(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				     struct drm_atomic_state *old_state)
#else
				     struct drm_crtc_state *old_state)
#endif
{
	unsigned long flags;

	drm_crtc_vblank_off(crtc);

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	if (crtc->state->event) {
		drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
	}
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}

static void dummy_crtc_atomic_flush(struct drm_crtc *crtc,
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				   struct drm_atomic_state *old_state)
#else
				   struct drm_crtc_state *old_state)
#endif
{
	unsigned long flags;

	if (crtc->state->event) {
		spin_lock_irqsave(&crtc->dev->event_lock, flags);

		if (drm_crtc_vblank_get(crtc) != 0)
			drm_crtc_send_vblank_event(crtc, crtc->state->event);
		else
			drm_crtc_arm_vblank_event(crtc, crtc->state->event);

		spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

		crtc->state->event = NULL;
	}
}

static const struct drm_crtc_helper_funcs dummy_crtc_helper_funcs = {
	.mode_valid	= dummy_crtc_mode_valid,
	.atomic_flush	= dummy_crtc_atomic_flush,
	.atomic_enable	= dummy_crtc_atomic_enable,
	.atomic_disable	= dummy_crtc_atomic_disable,
};

static int dummy_crtc_component_bind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm = data;
	struct dummy_crtc *dummy;
	struct drm_crtc *crtc;
	struct drm_plane *primary, *cursor;
	int ret;

	dummy = kzalloc(sizeof(*dummy), GFP_KERNEL);
	if (!dummy)
		return -ENOMEM;
	crtc = &dummy->crtc;

#if defined(OS_STRUCT_DRM_MODE_CONFIG_HAS_ALLOW_FB_MODIFIERS)
	drm->mode_config.allow_fb_modifiers = false;
#endif

	primary = dummy_plane_init(drm, DRM_PLANE_TYPE_PRIMARY, 0);
	if (IS_ERR(primary)) {
		ret = PTR_ERR(primary);
		goto err_plane;
	}

	cursor = dummy_plane_init(drm, DRM_PLANE_TYPE_CURSOR, 0);
	if (IS_ERR(cursor)) {
		ret = PTR_ERR(cursor);
		goto err_cursor;
	}

	ret = drm_crtc_init_with_planes(drm, crtc, primary, cursor,
					&dummy_crtc_funcs, NULL);
	if (ret) {
		DRM_ERROR("Failed to init CRTC\n");
		goto err_crtc;
	}

	drm_crtc_helper_add(crtc, &dummy_crtc_helper_funcs);

	spin_lock_init(&dummy->dummy_lock);

	DRM_INFO("dummy crtc driver loaded successfully\n");

	return 0;

err_crtc:
	drm_plane_cleanup(cursor);
err_cursor:
	drm_plane_cleanup(primary);
err_plane:
	kfree(dummy);

	return ret;
}

static void dummy_crtc_component_unbind(struct device *dev,
					struct device *master, void *data)
{
	DRM_INFO("unload dummy crtc driver\n");
}

static const struct component_ops dummy_crtc_component_ops = {
	.bind   = dummy_crtc_component_bind,
	.unbind = dummy_crtc_component_unbind,
};

static int dummy_crtc_probe(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &dummy_crtc_component_ops);
}

static int dummy_crtc_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &dummy_crtc_component_ops);
	return 0;
}

static struct platform_device_id dummy_crtc_device_id[] = {
	{ .name = "dummy-crtc", },
	{ },
};

struct platform_driver dummy_crtc_driver = {
	.probe    = dummy_crtc_probe,
	.remove   = dummy_crtc_remove,
	.driver   = {
		.owner  = THIS_MODULE,
		.name   = "dummy-crtc-drv",
	},
	.id_table = dummy_crtc_device_id,
};

