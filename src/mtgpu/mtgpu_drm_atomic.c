/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <drm/drm_print.h>
#include <drm/drm_atomic.h>
#include <drm/drm_vblank.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_writeback.h>

#include "mtgpu_drm_atomic.h"

static bool mtgpu_crtc_needs_disable(struct drm_crtc_state *old_state,
				     struct drm_crtc_state *new_state)
{
#if defined(OS_FUNC_DRM_ATOMIC_CRTC_EFFECTIVELY_ACTIVE_EXIST)
	/*
	 * No new_state means the crtc is off, so the only criteria is whether
	 * it's currently active or in self refresh mode.
	 */
	if (!new_state)
		return drm_atomic_crtc_effectively_active(old_state);

	/*
	 * We need to run through the crtc_funcs->disable() function if the crtc
	 * is currently on, if it's transitioning to self refresh mode, or if
	 * it's in self refresh mode and needs to be fully disabled.
	 */
	return old_state->active ||
	       (old_state->self_refresh_active && !new_state->enable) ||
	       new_state->self_refresh_active;
#else
	return old_state->active;
#endif
}

static void mtgpu_disable_outputs(struct drm_device *dev, struct drm_atomic_state *old_state)
{
	struct drm_connector *connector;
	struct drm_connector_state *old_conn_state, *new_conn_state;
	struct drm_crtc *crtc;
	struct drm_crtc_state *old_crtc_state, *new_crtc_state;
	int i;

	for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i) {
		const struct drm_crtc_helper_funcs *funcs;
		int ret;

		/* Shut down everything that needs a full modeset. */
		if (!drm_atomic_crtc_needs_modeset(new_crtc_state))
			continue;

		if (!mtgpu_crtc_needs_disable(old_crtc_state, new_crtc_state))
			continue;

		funcs = crtc->helper_private;

		DRM_DEBUG_ATOMIC("disabling [CRTC:%d:%s]\n",
				 crtc->base.id, crtc->name);

		/* Right function depends upon target state. */
		if (new_crtc_state->enable && funcs->prepare)
			funcs->prepare(crtc);
		else if (funcs->atomic_disable)
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
			funcs->atomic_disable(crtc, old_state);
#else
			funcs->atomic_disable(crtc, old_crtc_state);
#endif
		else if (funcs->disable)
			funcs->disable(crtc);
		else if (funcs->dpms)
			funcs->dpms(crtc, DRM_MODE_DPMS_OFF);

#if defined(OS_FUNC_DRM_DEV_HAS_VBLANK_EXIST)
		if (!drm_dev_has_vblank(dev))
#else
		if (!(dev->irq_enabled && dev->num_crtcs))
#endif
			continue;

		ret = drm_crtc_vblank_get(crtc);
		WARN_ONCE(ret != -EINVAL, "driver forgot to call drm_crtc_vblank_off()\n");
		if (ret == 0)
			drm_crtc_vblank_put(crtc);
	}

	for_each_oldnew_connector_in_state(old_state, connector,
					   old_conn_state, new_conn_state, i) {
		const struct drm_encoder_helper_funcs *funcs;
		struct drm_encoder *encoder;

		/* Shut down everything that's in the changeset and currently
		 * still on. So need to check the old, saved state.
		 */
		if (!old_conn_state->crtc)
			continue;

		old_crtc_state = drm_atomic_get_old_crtc_state(old_state, old_conn_state->crtc);

		if (new_conn_state->crtc)
			new_crtc_state = drm_atomic_get_new_crtc_state(old_state,
								       new_conn_state->crtc);
		else
			new_crtc_state = NULL;

		if (!mtgpu_crtc_needs_disable(old_crtc_state, new_crtc_state) ||
		    !drm_atomic_crtc_needs_modeset(old_conn_state->crtc->state))
			continue;

		encoder = old_conn_state->best_encoder;

		/* We shouldn't get this far if we didn't previously have
		 * an encoder.. but WARN_ON() rather than explode.
		 */
		if (WARN_ON(!encoder))
			continue;

		funcs = encoder->helper_private;

		DRM_DEBUG_ATOMIC("disabling [ENCODER:%d:%s]\n",
				 encoder->base.id, encoder->name);

		/* Right function depends upon target state. */
		if (funcs) {
#if defined(OS_STRUCT_DRM_ENCODER_HELPER_FUNCS_HAS_ATOMIC_DISABLE)
			if (funcs->atomic_disable)
				funcs->atomic_disable(encoder, old_state);
			else if (new_conn_state->crtc && funcs->prepare)
#else
			if (new_conn_state->crtc && funcs->prepare)
#endif
				funcs->prepare(encoder);
			else if (funcs->disable)
				funcs->disable(encoder);
			else if (funcs->dpms)
				funcs->dpms(encoder, DRM_MODE_DPMS_OFF);
		}
	}
}

static void mtgpu_crtc_set_mode(struct drm_device *dev, struct drm_atomic_state *old_state)
{
	struct drm_crtc *crtc;
	struct drm_crtc_state *new_crtc_state;
	struct drm_connector *connector;
	struct drm_connector_state *new_conn_state;
	int i;

	for_each_new_connector_in_state(old_state, connector, new_conn_state, i) {
		const struct drm_encoder_helper_funcs *funcs;
		struct drm_encoder *encoder;
		struct drm_display_mode *mode, *adjusted_mode;

		if (!new_conn_state->best_encoder)
			continue;

		encoder = new_conn_state->best_encoder;
		funcs = encoder->helper_private;
		new_crtc_state = new_conn_state->crtc->state;
		mode = &new_crtc_state->mode;
		adjusted_mode = &new_crtc_state->adjusted_mode;

		if (!new_crtc_state->mode_changed)
			continue;

		DRM_DEBUG_ATOMIC("modeset on [ENCODER:%d:%s]\n",
				 encoder->base.id, encoder->name);

		/*
		 * Each encoder has at most one connector (since we always steal
		 * it away), so we won't call mode_set hooks twice.
		 */
		if (funcs && funcs->atomic_mode_set) {
			funcs->atomic_mode_set(encoder, new_crtc_state,
					       new_conn_state);
		} else if (funcs && funcs->mode_set) {
			funcs->mode_set(encoder, mode, adjusted_mode);
		}
	}

	for_each_new_crtc_in_state(old_state, crtc, new_crtc_state, i) {
		const struct drm_crtc_helper_funcs *funcs;

		if (!new_crtc_state->mode_changed)
			continue;

		funcs = crtc->helper_private;

		if (new_crtc_state->enable && funcs->mode_set_nofb) {
			DRM_DEBUG_ATOMIC("modeset on [CRTC:%d:%s]\n",
					 crtc->base.id, crtc->name);

			funcs->mode_set_nofb(crtc);
		}
	}
}

static void mtgpu_atomic_helper_commit_writebacks(struct drm_device *dev,
						  struct drm_atomic_state *old_state)
{
	struct drm_connector *connector;
	struct drm_connector_state *new_conn_state;
	int i;

	for_each_new_connector_in_state(old_state, connector, new_conn_state, i) {
		const struct drm_connector_helper_funcs *funcs;

		funcs = connector->helper_private;
		if (!funcs->atomic_commit)
			continue;

		if (new_conn_state->writeback_job && new_conn_state->writeback_job->fb) {
			WARN_ON(connector->connector_type != DRM_MODE_CONNECTOR_WRITEBACK);
#if defined(OS_DRM_CONNECTOR_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
			funcs->atomic_commit(connector, old_state);
#else
			funcs->atomic_commit(connector, new_conn_state);
#endif
		}
	}
}

void mtgpu_atomic_helper_encoder_detect(struct drm_device *dev,
					struct drm_atomic_state *old_state)
{
	struct drm_atomic_state *original_state;
	struct drm_connector *connector;
	struct drm_connector_state *new_conn_state;
	int i;

	for_each_new_connector_in_state(old_state, connector, new_conn_state, i) {
		const struct drm_encoder_helper_funcs *funcs;
		struct drm_encoder *encoder;

		if (!new_conn_state || !new_conn_state->best_encoder)
				continue;

		if (new_conn_state->best_encoder->encoder_type != DRM_MODE_ENCODER_DPMST || !connector)
			continue;

		encoder = new_conn_state->best_encoder;
		funcs = encoder->helper_private;

		/*
		 * Each encoder has at most one connector (since we always steal
		 * it away), so we won't call enable hooks twice.
		 */
		if (!connector->state)
			continue;

		original_state = connector->state->state;
		connector->state->state = old_state;
		if (funcs && funcs->detect)
			funcs->detect(encoder, connector);
		connector->state->state = original_state;
	}

}

void mtgpu_atomic_helper_commit_modeset_enables(struct drm_device *dev,
						struct drm_atomic_state *old_state)
{
	struct drm_crtc *crtc;
	struct drm_crtc_state *old_crtc_state;
	struct drm_crtc_state *new_crtc_state;
	struct drm_connector *connector;
	struct drm_connector_state *new_conn_state;
	int i;

	for_each_new_connector_in_state(old_state, connector, new_conn_state, i) {
		const struct drm_encoder_helper_funcs *funcs;
		struct drm_encoder *encoder;

		if (!new_conn_state->best_encoder)
			continue;

		if (!new_conn_state->crtc->state->active ||
		    !drm_atomic_crtc_needs_modeset(new_conn_state->crtc->state))
			continue;

		encoder = new_conn_state->best_encoder;
		funcs = encoder->helper_private;

		DRM_DEBUG_ATOMIC("enabling [ENCODER:%d:%s]\n",
				 encoder->base.id, encoder->name);

		if (funcs) {
#if defined(OS_STRUCT_DRM_ENCODER_HELPER_FUNCS_HAS_ATOMIC_ENABLE)
			if (funcs->atomic_enable)
				funcs->atomic_enable(encoder, old_state);
			else if (funcs->enable)
#else
			if (funcs->enable)
#endif
				funcs->enable(encoder);
			else if (funcs->commit)
				funcs->commit(encoder);
		}
	}

	for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i) {
		const struct drm_crtc_helper_funcs *funcs;

		/* Need to filter out CRTCs where only planes change. */
		if (!drm_atomic_crtc_needs_modeset(new_crtc_state))
			continue;

		if (!new_crtc_state->active)
			continue;

		funcs = crtc->helper_private;

		if (new_crtc_state->enable) {
			DRM_DEBUG_ATOMIC("enabling [CRTC:%d:%s]\n",
					 crtc->base.id, crtc->name);
			if (funcs->atomic_enable)
#if defined(OS_DRM_CRTC_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
				funcs->atomic_enable(crtc, old_state);
#else
				funcs->atomic_enable(crtc, old_crtc_state);
#endif
			else if (funcs->commit)
				funcs->commit(crtc);
		}
	}

	mtgpu_atomic_helper_commit_writebacks(dev, old_state);
}

void mtgpu_atomic_helper_commit_modeset_disables(struct drm_device *dev,
						 struct drm_atomic_state *old_state)
{
	mtgpu_disable_outputs(dev, old_state);

	drm_atomic_helper_update_legacy_modeset_state(dev, old_state);

	mtgpu_crtc_set_mode(dev, old_state);
}

int mtgpu_atomic_helper_async_check(struct drm_device *dev,
				    struct drm_atomic_state *state)
{
	struct drm_crtc *crtc;
	struct drm_crtc_state *crtc_state;
	struct drm_plane *plane = NULL;
	struct drm_plane_state *old_plane_state = NULL;
	struct drm_plane_state *new_plane_state = NULL;
	const struct drm_plane_helper_funcs *funcs;
	int i, n_planes = 0;

	for_each_new_crtc_in_state(state, crtc, crtc_state, i) {
		if (drm_atomic_crtc_needs_modeset(crtc_state))
			return -EINVAL;
	}

	for_each_oldnew_plane_in_state(state, plane, old_plane_state, new_plane_state, i)
		n_planes++;

	/* FIXME: we support only single plane updates for now */
	if (n_planes != 1)
		return -EINVAL;

	if (!new_plane_state->crtc ||
	    old_plane_state->crtc != new_plane_state->crtc)
		return -EINVAL;

	funcs = plane->helper_private;
	if (!funcs->atomic_async_update)
		return -EINVAL;

	if (new_plane_state->fence)
		return -EINVAL;

	/*
	 * Don't do an async update if there is an outstanding commit modifying
	 * the plane.  This prevents our async update's changes from getting
	 * overridden by a previous synchronous update's state.
	 */
	if (old_plane_state->commit &&
	    !try_wait_for_completion(&old_plane_state->commit->hw_done))
		return -EBUSY;
#if defined(OS_DRM_PLANE_HELPER_FUNCS_USE_DRM_ATOMIC_STATE)
	return funcs->atomic_async_check(plane, state);
#else
	return funcs->atomic_async_check(plane, new_plane_state);
#endif
}
