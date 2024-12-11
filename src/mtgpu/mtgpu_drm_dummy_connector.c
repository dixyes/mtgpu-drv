/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/component.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#endif
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_print.h>
#if defined(OS_DRM_DRM_PROBE_HELPER_H_EXIST)
#include <drm/drm_probe_helper.h>
#endif
#include <drm/drm_edid.h>

#include "mtgpu_drv.h"
#include "mtgpu_dummy_common.h"

static struct dummy_display_mode {
	int hdisplay;
	int vdisplay;
	int vrefresh;
} extra_modes[] = {
#if (RGX_NUM_OS_SUPPORTED > 1)
	{  800,  600, 30 },
	{  800,  600, 60 },
	{ 1024,  768, 30 },
	{ 1024,  768, 60 },
	{ 1152,  864, 30 },
	{ 1152,  864, 60 },
	{ 1280,  720, 30 },
	{ 1280,  720, 60 },
	{ 1280,  768, 30 },
	{ 1280,  768, 60 },
	{ 1280,  800, 30 },
	{ 1280,  800, 60 },
	{ 1280,  960, 30 },
	{ 1280,  960, 60 },
	{ 1280, 1024, 30 },
	{ 1280, 1024, 60 },
	{ 1360,  768, 30 },
	{ 1360,  768, 60 },
	{ 1600,  900, 30 },
	{ 1600,  900, 60 },
	{ 1600, 1024, 30 },
	{ 1600, 1024, 60 },
	{ 1600, 1200, 30 },
	{ 1600, 1200, 60 },
	{ 1680, 1050, 30 },
	{ 1680, 1050, 60 },
	{ 1920, 1080, 30 },
	{ 1920, 1080, 60 },
	{ 1920, 1200, 30 },
	{ 1920, 1200, 60 },
	{ 2560, 1440, 30 },
	{ 2560, 1440, 60 },
	{ 2560, 1600, 30 },
	{ 2560, 1600, 60 },
	{ 3840, 2160, 30 },
	{ 3840, 2160, 60 },
	{ 4096, 2160, 30 },
	{ 4096, 2160, 60 },
#else
	{ 1920, 1080, 30 },
	{ 1920, 1080, 120 },
	{ 1920, 1080, 144 },
	{ 2560, 1440, 30 },
	{ 2560, 1440, 60 },
	{ 2560, 1440, 120 },
	{ 2560, 1440, 144 },
	{ 3440, 1440, 30 },
	{ 3440, 1440, 60 },
	{ 3440, 1440, 120 },
	{ 3440, 1440, 144 },
	{ 3840, 2160, 30 },
	{ 3840, 2160, 60 },
	{ 3840, 2160, 120 },
	{ 3840, 2160, 144 },
#endif
};

static struct drm_display_mode custom_modes[] = {
	{ DRM_MODE("1366x768", DRM_MODE_TYPE_DRIVER, 40028, 1366, 1400,
		   1534, 1702, 0, 768, 771, 781, 784, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC) },
	{ DRM_MODE("1366x768", DRM_MODE_TYPE_DRIVER, 85250, 1366, 1400,
		   1574, 1782, 0, 768, 771, 781, 798, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC) },
};

static int dummy_connector_add_extra_modes(struct drm_connector *connector,
					   uint32_t max_hres,
					   uint32_t max_vres)
{
	struct drm_device *dev = connector->dev;
	struct drm_display_mode *mode;
	int i, count = 0;

	for (i = 0; i < ARRAY_SIZE(extra_modes); i++) {
		if ((max_hres >= extra_modes[i].hdisplay || max_vres >= extra_modes[i].vdisplay) &&
		    ((max_hres * max_vres) >= (extra_modes[i].hdisplay * extra_modes[i].vdisplay))) {
			mode = drm_cvt_mode(dev, extra_modes[i].hdisplay, extra_modes[i].vdisplay,
					    extra_modes[i].vrefresh, false, false, false);
			drm_mode_probed_add(connector, mode);

			count++;
		}
	}

	for (i = 0; mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_GUEST &&
	     i < ARRAY_SIZE(custom_modes); i++) {
		if ((max_hres >= custom_modes[i].hdisplay || max_vres >= custom_modes[i].vdisplay) &&
		    ((max_hres * max_vres) >= (custom_modes[i].hdisplay * custom_modes[i].vdisplay))) {
			mode = drm_mode_duplicate(dev, &custom_modes[i]);
			drm_mode_probed_add(connector, mode);
			count++;
		}
	}

	return count;
}

static void dummy_connector_set_preferred_mode(struct drm_connector *connector,
					       int hdisplay_pref, int vdisplay_pref,
					       int vrefresh_pref)
{
	struct drm_display_mode *mode;

	list_for_each_entry(mode, &connector->probed_modes, head) {
		if (mode->hdisplay != hdisplay_pref)
			continue;

		if (mode->vdisplay != vdisplay_pref)
			continue;

		if (drm_mode_vrefresh(mode) == vrefresh_pref) {
			mode->type |= DRM_MODE_TYPE_PREFERRED;
			break;
		}
	}
}

static int dummmy_connector_get_max_resolution(struct drm_connector *connector,
						  uint32_t *max_hres,
						  uint32_t *max_vres)
{
	struct mtgpu_dummy *dummy = connector_to_mtgpu_dummy(connector);

	*max_hres = dummy->ctx.max_hres;
	*max_vres = dummy->ctx.max_vres;
	return 0;
}

static int dummy_connector_get_modes(struct drm_connector *connector)
{
	int count = 0, max_hres, max_vres;

	dummmy_connector_get_max_resolution(connector, &max_hres, &max_vres);

	if (mtgpu_get_driver_mode() != MTGPU_DRIVER_MODE_GUEST)
		count = drm_add_modes_noedid(connector, max_hres, max_vres);

	count += dummy_connector_add_extra_modes(connector, max_hres, max_vres);

	/* set dmt mode 1920x1080@60 preferred */
	dummy_connector_set_preferred_mode(connector, 1920, 1080, 60);
	return count;
}

static void dummy_connector_destroy(struct drm_connector *connector)
{
	drm_connector_cleanup(connector);
	os_destroy_drm_connector(connector);
}

static void dummy_encoder_destroy(struct drm_encoder *encoder)
{
	drm_encoder_cleanup(encoder);
	os_destroy_drm_encoder(encoder);
}

static enum drm_connector_status
mtgpu_dummy_connector_detect(struct drm_connector *connector, bool force)
{
	if (mtgpu_display_is_none())
		return connector_status_disconnected;

	return connector_status_connected;
}

static struct drm_connector_helper_funcs dummy_connector_helper_funcs = {
	.get_modes = dummy_connector_get_modes,
};

static const struct drm_connector_funcs dummy_connector_funcs = {
	.detect = mtgpu_dummy_connector_detect,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = dummy_connector_destroy,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static const struct drm_encoder_funcs dummy_encoder_funcs = {
	.destroy = dummy_encoder_destroy,
};

static int dummy_connector_component_bind(struct device *dev,
					  struct device *master, void *data)
{
	struct drm_device *drm = data;
	struct drm_connector *connector;
	struct drm_encoder *encoder;
	struct mtgpu_dummy_platform_data *pdata = dev_get_platdata(dev);
	struct mtgpu_dummy *dummy;
	int err;

	dummy = kzalloc(sizeof(*dummy), GFP_KERNEL);
	if (!dummy)
		return -ENOMEM;

	dummy->dev = dev;

	encoder = os_create_drm_encoder();
	if (!encoder) {
		err = -ENOMEM;
		goto err_alloc_encoder;
	}

	err = drm_encoder_init(drm, encoder, &dummy_encoder_funcs,
			       DRM_MODE_ENCODER_VIRTUAL, NULL);
	if (err) {
		DRM_ERROR("failed to init encoder\n");
		goto err_encoder_init;
	}
	encoder->possible_crtcs = 0x1;

	connector = os_create_drm_connector();
	if (!connector) {
		err = -ENOMEM;
		goto err_alloc_connector;
	}

	drm_connector_init(drm, connector, &dummy_connector_funcs,
			   DRM_MODE_CONNECTOR_VIRTUAL);
	drm_connector_helper_add(connector, &dummy_connector_helper_funcs);

	err = drm_connector_attach_encoder(connector, encoder);
	if (err) {
		DRM_ERROR("Failed to attach connector to encoder\n");
		goto err_attach;
	}

	DRM_INFO("dummy connector driver loaded successfully\n");
	dummy->ctx.id = pdata->id;
	dummy->ctx.max_hres = pdata->max_hres;
	dummy->ctx.max_vres = pdata->max_vres;
	dummy->connector = connector;
	dummy->encoder = encoder;
	os_set_drm_encoder_drvdata(dummy->encoder, dummy);
	os_set_drm_connector_drvdata(dummy->connector, dummy);
	dev_set_drvdata(dev, dummy);

	return 0;

err_attach:
	drm_connector_cleanup(connector);
	os_destroy_drm_connector(connector);
err_alloc_connector:
	drm_encoder_cleanup(encoder);
err_encoder_init:
	os_destroy_drm_encoder(encoder);
err_alloc_encoder:
	kfree(dummy);

	return err;
}

static void dummy_connector_component_unbind(struct device *dev,
					     struct device *master, void *data)
{
	struct mtgpu_dummy *dummy = dev_get_drvdata(dev);
	DRM_INFO("unload dummy connector driver\n");
	
	if (dummy)
		kfree(dummy);	
}

static const struct component_ops dummy_connector_component_ops = {
	.bind   = dummy_connector_component_bind,
	.unbind = dummy_connector_component_unbind,
};

static int dummy_connector_probe(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &dummy_connector_component_ops);
}

static int dummy_connector_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &dummy_connector_component_ops);
	return 0;
}

static struct platform_device_id dummy_device_id[] = {
	{ .name = "dummy-connector",},
	{ },
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void _dummy_connector_remove(struct platform_device *pdev)
{
	dummy_connector_remove(pdev);
}
#endif // KERNEL_VERSION


struct platform_driver dummy_connector_driver = {
	.probe    = dummy_connector_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
	.remove   = _dummy_connector_remove,
#else
	.remove   = dummy_connector_remove,
#endif // KERNEL_VERSION
	.driver   = {
		.owner  = THIS_MODULE,
		.name   = "dummy-connector-drv",
	},
	.id_table = dummy_device_id,
};

