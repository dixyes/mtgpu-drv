/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/component.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#endif
#include <drm/drm_atomic_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_print.h>
#if defined(OS_DRM_DRM_PROBE_HELPER_H_EXIST)
#include <drm/drm_probe_helper.h>
#endif
#if defined(OS_DRM_DISPLAY_DRM_SCDC_H_EXIST)
#include <drm/display/drm_scdc.h>
#else
#include <drm/drm_scdc_helper.h>
#endif
#include <video/videomode.h>
#include <sound/hdmi-codec.h>

#include "mtgpu_hdmi_common.h"
#include "mtgpu_drv.h"
#include "mtgpu_board_cfg.h"

struct mtgpu_hdmi {
	struct drm_connector connector;
	struct drm_encoder encoder;
	struct mtgpu_hdmi_ctx ctx;
	struct mtgpu_hdmi_ops *core;
	struct mtgpu_hdmi_glb_ops *glb;
	struct device *dev;
	struct i2c_adapter *ddc;
	struct mutex i2c_lock;
	struct delayed_work hpd_work;
	u8 port_type;
	/* audio codec */
	struct platform_device *hdmi_audio;
	hdmi_codec_plugged_cb codec_plugged_cb;

	struct device *codec_dev;
	struct mutex codec_lock;
	struct edid *edid;
	bool connected;
	bool enabled;
	bool audio_enabled;
};

static const u8 hdmi_port_type[] = {
	DRM_MODE_CONNECTOR_Unknown,	/* PORT_DISABLED */
	DRM_MODE_CONNECTOR_HDMIA,	/* PORT_TYPE_HDMIA */
	DRM_MODE_CONNECTOR_VGA,		/* PORT_TYPE_HDMIA2VGA */
	DRM_MODE_CONNECTOR_LVDS,	/* PORT_TYPE_HDMIA2LVDS */
	DRM_MODE_CONNECTOR_DisplayPort,	/* PORT_TYPE_HDMIA2DP */
};

static struct drm_display_mode hdmi_supported_mode[] = {
	/* 1 - 640x480@60Hz */
	{ DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 25175, 640, 656,
		   752, 800, 0, 480, 490, 492, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 2 - 720x480@60Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 27000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 3 - 1024x768@60Hz */
	{ DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 65000, 1024, 1048,
		   1184, 1344, 0, 768, 771, 777, 806, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 4 - 1024x768@70Hz */
	{ DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 75000, 1024, 1048,
		   1184, 1328, 0, 768, 771, 777, 806, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 5 - 1024x768@75Hz */
	{ DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 78750, 1024, 1040,
		   1136, 1312, 0, 768, 769, 772, 800, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 6 - 1024x768@85Hz */
	{ DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 94500, 1024, 1072,
		   1168, 1376, 0, 768, 769, 772, 808, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 7 - 1024x768@120Hz */
	{ DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 115500, 1024, 1072,
		   1104, 1184, 0, 768, 771, 775, 813, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 8 - 1280x720@60Hz 4:3 */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1390,
		   1430, 1650, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 9 - 1680x1050@60Hz */
	{ DRM_MODE("1680x1050", DRM_MODE_TYPE_DRIVER, 146250, 1680, 1784,
		   1960, 2240, 0, 1050, 1053, 1059, 1089, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 10 - 1920x1080@60Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008,
		   2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 11 - 2560x1440@60Hz */
	{ DRM_MODE("2560x1440", DRM_MODE_TYPE_DRIVER, 241500, 2560, 2608,
		   2640, 2720, 0, 1440, 1443, 1448, 1481, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 12 - 2560x1440@60Hz HKC Monitor use 241700Khz for 2k@60 */
	{ DRM_MODE("2560x1440", DRM_MODE_TYPE_DRIVER, 241700, 2560, 2608,
		   2640, 2720, 0, 1440, 1443, 1448, 1481, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 13 - 3440x1440@50Hz */
	{ DRM_MODE("3440x1440", DRM_MODE_TYPE_DRIVER, 266580, 3440, 3488,
		   3520, 3600, 0, 1440, 1443, 1448, 1481, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 14 - 3440x1440@60Hz */
	{ DRM_MODE("3440x1440", DRM_MODE_TYPE_DRIVER, 319890, 3440, 3488,
		   3520, 3600, 0, 1440, 1443, 1448, 1481, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 15 - 3440x1440@100Hz */
	{ DRM_MODE("3440x1440", DRM_MODE_TYPE_DRIVER, 533120, 3440, 3528,
		   3592, 3600, 0, 1440, 1444, 1449, 1481, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC), },
	/* 16 - 3840x2160@30Hz */
	{ DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4016,
		   4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
	/* 17 - 3840x2160@60Hz */
	{ DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 4016,
		   4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC), },
};

static inline struct mtgpu_hdmi *encoder_to_mtgpu_hdmi(struct drm_encoder *encoder)
{
	return container_of(encoder, struct mtgpu_hdmi, encoder);
}

static inline
struct mtgpu_hdmi *connector_to_mtgpu_hdmi(struct drm_connector *connector)
{
	return container_of(connector, struct mtgpu_hdmi, connector);
}

static enum drm_connector_status
mtgpu_hdmi_connector_detect(struct drm_connector *connector, bool force)
{
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);
	bool connected;
	u16 retries = 100;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	if (!hdmi->core->is_plugin)
		return connector_status_disconnected;

	connected = hdmi->core->is_plugin(&hdmi->ctx);

	if (connected && !hdmi->edid) {
		do {
			hdmi->edid = drm_get_edid(connector, hdmi->ddc);
			usleep_range(2000, 3000);
			retries--;
		} while (!hdmi->edid && retries);

		if (!hdmi->edid) {
			DRM_DEV_ERROR(hdmi->dev, "failed to get edid\n");
			drm_connector_update_edid_property(connector, NULL);
		} else {
			drm_connector_update_edid_property(connector, hdmi->edid);
		}
	} else if (!connected && hdmi->edid) {
		kfree(hdmi->edid);
		hdmi->edid = NULL;
	}

	hdmi->connected = connected;

	return connected ? connector_status_connected :
		connector_status_disconnected;
}

static int mtgpu_hdmi_connector_get_modes(struct drm_connector *connector)
{
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);
	int ret;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	if (!hdmi->edid) {
		DRM_DEV_ERROR(hdmi->dev, "no edid\n");

		return -ENODEV;
	}

	ret = drm_add_edid_modes(connector, hdmi->edid);

	return ret;
}

static int mtgpu_hdmi_mode_compare(struct drm_display_mode *a,
				   struct drm_display_mode *b)
{
	int diff;

	diff = b->hdisplay - a->hdisplay;
	if (diff)
		return diff;

	diff = b->vdisplay - a->vdisplay;
	if (diff)
		return diff;

	diff = drm_mode_vrefresh(b) - drm_mode_vrefresh(a);
	if (diff)
		return diff;

	diff = (b->clock - a->clock) / 50;

	return diff;
}

static bool mtgpu_hdmi_mode_supported_with_audio(struct mtgpu_hdmi *hdmi,
						 struct drm_display_mode *mode)
{
	int idx;
	int sad_count;
	struct cea_sad *sads = NULL;

	/* check if audio is supported */
	sad_count =  drm_edid_to_sad(hdmi->edid, &sads);
	kfree(sads);
	if (sad_count <= 0)
		return true;

	for (idx = 0; idx < ARRAY_SIZE(hdmi_supported_mode); ++idx) {
		/* compare the timing and clock */
		if (!mtgpu_hdmi_mode_compare(mode, &hdmi_supported_mode[idx]))
			return true;
	}
	return false;
}

static struct drm_encoder *
mtgpu_hdmi_connector_best_encoder(struct drm_connector *connector)
{
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	return &hdmi->encoder;
}

static int mtgpu_hdmi_mode_supported_by_product(struct mtgpu_hdmi *hdmi,
						struct drm_display_mode *mode)
{
	if (mode->clock / 100 > hdmi->ctx.max_pclk_100khz)
		return MODE_CLOCK_HIGH;

	if (mode->hdisplay > hdmi->ctx.max_hres)
		return MODE_H_ILLEGAL;

	if (mode->vdisplay > hdmi->ctx.max_vres)
		return MODE_V_ILLEGAL;

	return MODE_OK;
}

static void mtgpu_hdmi_set_preferred_mode(struct drm_connector *connector,
					  struct drm_display_mode *filtered_mode)
{
	struct drm_display_mode *mode;
	struct drm_display_mode tmp_best_mode = {0};
	struct drm_display_mode *preferred_mode = NULL;
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);

	if (list_empty(&connector->modes))
		return;

	/* Find the best mode and set its type preferred. */
	list_for_each_entry(mode, &connector->modes, head) {
		if (!mtgpu_hdmi_mode_supported_with_audio(hdmi, mode) ||
		    (mtgpu_hdmi_mode_supported_by_product(hdmi, mode) != MODE_OK) ||
		    mode->status != MODE_OK ||
		    drm_mode_equal(mode, filtered_mode))
			continue;

		if (mtgpu_hdmi_mode_compare(&tmp_best_mode, mode) > 0) {
			memcpy(&tmp_best_mode, mode, sizeof(struct drm_display_mode));
			preferred_mode = mode;
		}
	}

	if (preferred_mode)
		preferred_mode->type |= DRM_MODE_TYPE_PREFERRED;
}

static int mtgpu_hdmi_connector_mode_valid(struct drm_connector *connector,
					   struct drm_display_mode *mode)
{
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);
	int mode_status;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	/* check if the mode can be supported by product. */
	mode_status = mtgpu_hdmi_mode_supported_by_product(hdmi, mode);
	if (mode_status != MODE_OK)
		goto preferred_mode_check;

	/*
	 * HDMI IP does not support well for reduced blank timing,
	 * if audio packet is transimited with this timing,
	 * hdmi receiver would flicker, so we only support the modes
	 * listed in the hdmi_supported_mode.
	 */
	if (!mtgpu_hdmi_mode_supported_with_audio(hdmi, mode)) {
		mode_status = MODE_BAD;
		goto preferred_mode_check;
	}

	return mode_status;

preferred_mode_check:
	if (mode->type & DRM_MODE_TYPE_PREFERRED) {
		/*
		 * When the preferred mode is unsupported,
		 * we should set the remaining best mode preferred.
		 */
		mtgpu_hdmi_set_preferred_mode(connector, mode);
	}

	return mode_status;
}

static const struct drm_connector_funcs mtgpu_hdmi_connector_funcs = {
	.detect			= mtgpu_hdmi_connector_detect,
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= drm_connector_cleanup,
	.atomic_duplicate_state	= drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
	.reset			= drm_atomic_helper_connector_reset,
};

static const struct drm_connector_helper_funcs
mtgpu_hdmi_connector_helper_funcs = {
	.get_modes	= mtgpu_hdmi_connector_get_modes,
	.best_encoder	= mtgpu_hdmi_connector_best_encoder,
	.mode_valid	= mtgpu_hdmi_connector_mode_valid,
};

static int mtgpu_hdmi_send_audio_infoframe(struct mtgpu_hdmi *hdmi)
{
	struct hdmi_audio_infoframe frame;
	u8 buffer[HDMI_INFOFRAME_HEADER_SIZE + HDMI_AUDIO_INFOFRAME_SIZE];
	ssize_t err;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	err = hdmi_audio_infoframe_init(&frame);
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to setup audio infoframe: %zd\n", err);
		return err;
	}

	frame.coding_type = HDMI_AUDIO_CODING_TYPE_STREAM;
	frame.sample_frequency = HDMI_AUDIO_SAMPLE_FREQUENCY_STREAM;
	frame.sample_size = HDMI_AUDIO_SAMPLE_SIZE_STREAM;
	frame.channels = 2;

	err = hdmi_audio_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to pack audio infoframe: %zd\n", err);
		return err;
	}

	if (hdmi->core->audio_infoframe)
		hdmi->core->audio_infoframe(&hdmi->ctx, buffer, sizeof(buffer));

	return 0;
}

static int mtgpu_hdmi_send_avi_infoframe(struct mtgpu_hdmi *hdmi)
{
	struct hdmi_avi_infoframe frame;
	u8 buffer[HDMI_INFOFRAME_HEADER_SIZE + HDMI_AVI_INFOFRAME_SIZE];
	struct drm_display_mode mode = {};
	ssize_t err;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	drm_display_mode_from_videomode(hdmi->ctx.vm, &mode);

#if !defined(OS_DRM_HDMI_AVI_INFOFRAME_FROM_DISPLAY_MODE_HAS_BOOL_ARG)
	err = drm_hdmi_avi_infoframe_from_display_mode(&frame,
						       &hdmi->connector, &mode);
#else
	err = drm_hdmi_avi_infoframe_from_display_mode(&frame, &mode, false);
#endif
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to get AVI infoframe from mode: %zd\n", err);
		return err;
	}

	err = hdmi_avi_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to pack AVI infoframe: %zd\n", err);
		return err;
	}

	if (hdmi->core->avi_infoframe)
		hdmi->core->avi_infoframe(&hdmi->ctx, buffer, sizeof(buffer));

	return 0;
}

static int mtgpu_hdmi_send_spd_infoframe(struct mtgpu_hdmi *hdmi)
{
	struct hdmi_spd_infoframe frame;
	u8 buffer[HDMI_INFOFRAME_HEADER_SIZE + HDMI_SPD_INFOFRAME_SIZE];
	ssize_t err;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	err = hdmi_spd_infoframe_init(&frame, "MooreThreads", "On-Chip HDMI");
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to initialize SPD infoframe: %zd\n", err);
		return err;
	}

	err = hdmi_spd_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to pack SDP infoframe: %zd\n", err);
		return err;
	}

	if (hdmi->core->spd_infoframe)
		hdmi->core->spd_infoframe(&hdmi->ctx, buffer, sizeof(buffer));

	return 0;
}

static int mtgpu_hdmi_send_vendor_infoframe(struct mtgpu_hdmi *hdmi)
{
	struct drm_display_mode mode = {};
	struct hdmi_vendor_infoframe frame;
	u8 buffer[10];
	ssize_t err;
	bool has_hdmi_infoframe = hdmi->connector.display_info.has_hdmi_infoframe;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	if (!has_hdmi_infoframe)
		return 0;

	drm_display_mode_from_videomode(hdmi->ctx.vm, &mode);

	err = drm_hdmi_vendor_infoframe_from_display_mode(&frame,
							  &hdmi->connector, &mode);
	if (err) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to get vendor infoframe from mode: %zd\n", err);
		return err;
	}

	err = hdmi_vendor_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to pack vendor infoframe: %zd\n", err);
		return err;
	}

	if (hdmi->core->vendor_infoframe)
		hdmi->core->vendor_infoframe(&hdmi->ctx, buffer, sizeof(buffer));

	return 0;
}

static void mtgpu_hdmi_scramble_config(struct mtgpu_hdmi *hdmi)
{
	int ret;
	u8 version;
	u8 offset = SCDC_SINK_VERSION;
	struct drm_display_info *disp_info = &hdmi->connector.display_info;
	struct i2c_msg sink_ver_msgs[] = {
		{
			.addr	= DDC_SCDCS_ADDR,
			.flags	= 0,
			.len	= 1,
			.buf	= &offset,
		}, {
			.addr	= DDC_SCDCS_ADDR,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= &version,
		}
	};
	u8 src_ver[2] = {SCDC_SOURCE_VERSION, 1};
	struct i2c_msg src_ver_msg = {
		.addr	= DDC_SCDCS_ADDR,
		.flags	= 0,
		.len	= 2,
		.buf	= &src_ver[0],
	};
	u8 config[2] = {SCDC_TMDS_CONFIG, 0};
	struct i2c_msg config_msg = {
		.addr	= DDC_SCDCS_ADDR,
		.flags	= 0,
		.len	= 2,
		.buf	= &config[0],
	};

	if (!disp_info->hdmi.scdc.supported) {
		hdmi->core->scramble_config(&hdmi->ctx, false);

		return;
	}

	ret = i2c_transfer(hdmi->ddc, &sink_ver_msgs[0], 2);
	if (ret < 0)
		DRM_DEV_INFO(hdmi->dev, "failed to read scdc sink version\n");

	ret = i2c_transfer(hdmi->ddc, &src_ver_msg, 1);
	if (ret < 0)
		DRM_DEV_INFO(hdmi->dev, "failed to write scdc source version\n");

	if (hdmi->ctx.vm->pixelclock > 340000000) {
		config[1] = SCDC_SCRAMBLING_ENABLE | SCDC_TMDS_BIT_CLOCK_RATIO_BY_40;

		ret = i2c_transfer(hdmi->ddc, &config_msg, 1);
		if (ret < 0)
			DRM_DEV_INFO(hdmi->dev, "failed to write scdc tmds config\n");

		hdmi->core->scramble_config(&hdmi->ctx, true);
	} else {
		config[1] = 0;

		ret = i2c_transfer(hdmi->ddc, &config_msg, 1);
		if (ret < 0)
			DRM_DEV_INFO(hdmi->dev, "failed to write scdc tmds config\n");

		hdmi->core->scramble_config(&hdmi->ctx, false);
	}
}

static void mtgpu_hdmi_encoder_enable(struct drm_encoder *encoder)
{
	struct mtgpu_hdmi *hdmi = encoder_to_mtgpu_hdmi(encoder);

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	if (!hdmi->connected) {
		DRM_DEV_INFO(hdmi->dev, "Not connected, skip the enable\n");
		goto hdmi_enabled;
	}

	if (hdmi->core->config)
		hdmi->core->config(&hdmi->ctx);

	mtgpu_hdmi_send_audio_infoframe(hdmi);
	mtgpu_hdmi_send_avi_infoframe(hdmi);
	mtgpu_hdmi_send_spd_infoframe(hdmi);
	mtgpu_hdmi_send_vendor_infoframe(hdmi);

	/* TODO: Send other packets here, for example: GCP, ACP etc. */

	mtgpu_hdmi_scramble_config(hdmi);

	if (hdmi->core->enable)
		hdmi->core->enable(&hdmi->ctx);

	if (hdmi->codec_plugged_cb && hdmi->codec_dev)
		hdmi->codec_plugged_cb(hdmi->codec_dev, true);

hdmi_enabled:
	hdmi->enabled = true;

}

static void mtgpu_hdmi_encoder_disable(struct drm_encoder *encoder)
{
	struct mtgpu_hdmi *hdmi = encoder_to_mtgpu_hdmi(encoder);

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	if (hdmi->codec_plugged_cb && hdmi->codec_dev)
		hdmi->codec_plugged_cb(hdmi->codec_dev, false);

	if (hdmi->core->disable)
		hdmi->core->disable(&hdmi->ctx);

	hdmi->enabled = false;
}

static void
mtgpu_hdmi_encoder_atomic_mode_set(struct drm_encoder *encoder,
				   struct drm_crtc_state *crtc_state,
				   struct drm_connector_state *connector_state)
{
	struct mtgpu_hdmi *hdmi = encoder_to_mtgpu_hdmi(encoder);
	struct drm_display_mode *adjusted_mode = &crtc_state->adjusted_mode;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	drm_display_mode_to_videomode(adjusted_mode, hdmi->ctx.vm);
}

static const struct drm_encoder_helper_funcs mtgpu_hdmi_encoder_helper_funcs = {
	.enable			= mtgpu_hdmi_encoder_enable,
	.disable		= mtgpu_hdmi_encoder_disable,
	.atomic_mode_set	= mtgpu_hdmi_encoder_atomic_mode_set,
};

static const struct drm_encoder_funcs mtgpu_hdmi_encoder_funcs = {
	.destroy = drm_encoder_cleanup,
};

static int mtgpu_hdmi_drm_init(struct mtgpu_hdmi *hdmi, struct drm_device *drm)
{
	struct drm_encoder *encoder = &hdmi->encoder;
	struct drm_connector *connector = &hdmi->connector;
	int ret;

	/*
	* WARNING:
	* The possible_crtcs mask is in the logical order of dispc platform device
	* registration, not the dispc hardware ID order. So if the dispc platform
	* device count changed, this possible_crtcs mask should also be changed.
	*/
	encoder->possible_crtcs = BIT(2);
	ret = drm_encoder_init(drm, encoder, &mtgpu_hdmi_encoder_funcs,
			       DRM_MODE_ENCODER_TMDS, NULL);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "failed to create hdmi encoder\n");
		return ret;
	}
	drm_encoder_helper_add(encoder, &mtgpu_hdmi_encoder_helper_funcs);

	connector->polled = DRM_CONNECTOR_POLL_HPD;

	if (hdmi->port_type >= ARRAY_SIZE(hdmi_port_type)) {
		DRM_DEV_INFO(hdmi->dev,
			     "WARNING: invalid hdmi->port_type 0x%x, use HDMIA type as default\n",
			     hdmi->port_type);
		hdmi->port_type = PORT_TYPE_HDMIA;
	}

	ret = drm_connector_init(drm, connector,
				 &mtgpu_hdmi_connector_funcs,
				 hdmi_port_type[hdmi->port_type]);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "failed to create hdmi connector\n");
		return ret;
	}

	drm_connector_helper_add(connector, &mtgpu_hdmi_connector_helper_funcs);
	drm_connector_register(connector);
	drm_connector_attach_encoder(connector, encoder);

	return 0;
}

static void mtgpu_hdmi_hpd_work(struct work_struct *work)
{
	enum drm_connector_status old_status;
	struct mtgpu_hdmi *hdmi = container_of(work, struct mtgpu_hdmi, hpd_work.work);

	old_status = hdmi->connected ? connector_status_connected : connector_status_disconnected;

	hdmi->connector.status = drm_helper_probe_detect(&hdmi->connector, NULL, false);

	if (hdmi->enabled && hdmi->connector.status == connector_status_connected)
		mtgpu_hdmi_encoder_enable(&hdmi->encoder);

	if (old_status != hdmi->connector.status)
		drm_kms_helper_hotplug_event(hdmi->connector.dev);
}

static void mtgpu_hdmi_irq_handler(void *data)
{
	struct mtgpu_hdmi *hdmi = (struct mtgpu_hdmi *)data;
	u32 status = 0;

	if (hdmi->core->isr)
		status = hdmi->core->isr(&hdmi->ctx);

	if (status & MTGPU_HDMI_IRQ_TYPE_HPD_OUT)
		schedule_delayed_work(&hdmi->hpd_work,
				      msecs_to_jiffies(DELAY_ON_HDMI_PLUGOUT_DETECTION_MS));
	else if (status & MTGPU_HDMI_IRQ_TYPE_HPD_IN)
		schedule_delayed_work(&hdmi->hpd_work, 0);
}

/*
 * HDMI audio codec callbacks
 */

static int mtgpu_hdmi_audio_hw_params(struct device *dev, void *data,
				      struct hdmi_codec_daifmt *daifmt,
				      struct hdmi_codec_params *params)
{
	struct mtgpu_hdmi *hdmi = data;

	if (!params) {
		DRM_DEV_ERROR(hdmi->dev, "%s: audio params is NULL.\n", __func__);
		return -EINVAL;
	}

	if (!hdmi->enabled) {
		DRM_DEV_INFO(hdmi->dev, "%s: hdmi stream has not enabled.\n", __func__);
		return -EPERM;
	}

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	DRM_DEV_DEBUG(hdmi->dev, "%s: %u Hz, %d bit, %d channels\n",
		      __func__, params->sample_rate,
		      params->sample_width, params->cea.channels);

	if (hdmi->core->audio_set_param)
		return hdmi->core->audio_set_param(&hdmi->ctx, params);
	else
		return -ENOTSUPP;
}

static int mtgpu_hdmi_audio_startup(struct device *dev, void *data)
{
	struct mtgpu_hdmi *hdmi = data;

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	if (hdmi->core->audio_enable)
		hdmi->core->audio_enable(&hdmi->ctx);

	hdmi->audio_enabled = true;

	return 0;
}

static void mtgpu_hdmi_audio_shutdown(struct device *dev, void *data)
{
	struct mtgpu_hdmi *hdmi = data;

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	if (hdmi->core->audio_disable)
		hdmi->core->audio_disable(&hdmi->ctx);

	hdmi->audio_enabled = false;
}

#if defined(OS_STRUCT_HDMI_CODEC_OPS_HAS_MUTE_STREAM)
static int mtgpu_hdmi_audio_mute(struct device *dev, void *data, bool enable, int direction)
#else
static int mtgpu_hdmi_audio_mute(struct device *dev, void *data, bool enable)
#endif
{
	struct mtgpu_hdmi *hdmi = data;

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	if (enable && hdmi->core->audio_mute)
		hdmi->core->audio_mute(&hdmi->ctx);
	else if (!enable && hdmi->core->audio_unmute)
		hdmi->core->audio_unmute(&hdmi->ctx);

	return 0;
}

static int mtgpu_hdmi_audio_get_eld(struct device *dev, void *data, uint8_t *buf, size_t len)
{
	struct mtgpu_hdmi *hdmi = data;

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	memcpy(buf, hdmi->connector.eld, min(sizeof(hdmi->connector.eld), len));

	return 0;
}

static int mtgpu_hdmi_audio_hook_plugged_cb(struct device *dev, void *data,
					    hdmi_codec_plugged_cb fn,
					    struct device *codec_dev)
{
	struct mtgpu_hdmi *hdmi = data;

	DRM_DEV_INFO(hdmi->dev, "%s()\n", __func__);

	mutex_lock(&hdmi->codec_lock);
	hdmi->codec_plugged_cb = fn;
	hdmi->codec_dev = codec_dev;
	mutex_unlock(&hdmi->codec_lock);

	if (hdmi->codec_plugged_cb && hdmi->codec_dev)
		hdmi->codec_plugged_cb(hdmi->codec_dev, hdmi->enabled);

	return 0;
}

#if defined(OS_STRUCT_HDMI_CODEC_OPS_HAS_HOOK_PLUGGED_CB)
static const struct hdmi_codec_ops mtgpu_hdmi_audio_codec_ops = {
	.hw_params = mtgpu_hdmi_audio_hw_params,
	.audio_startup = mtgpu_hdmi_audio_startup,
	.audio_shutdown = mtgpu_hdmi_audio_shutdown,
#if defined(OS_STRUCT_HDMI_CODEC_OPS_HAS_MUTE_STREAM)
	.mute_stream = mtgpu_hdmi_audio_mute,
#else
	.digital_mute = mtgpu_hdmi_audio_mute,
#endif
	.get_eld = mtgpu_hdmi_audio_get_eld,
	.hook_plugged_cb = mtgpu_hdmi_audio_hook_plugged_cb,
};
#else
typedef int (*hook_plugged_callback)(struct device *dev, void *data,
					    hdmi_codec_plugged_cb fn,
					    struct device *codec_dev);

struct mtgpu_codec_ops {
	struct hdmi_codec_ops old_hdmi_audio_codec_ops;
	hook_plugged_callback hook_plugged_cb;
};

static const struct mtgpu_codec_ops mtgpu_hdmi_audio_codec_ops = {
	.old_hdmi_audio_codec_ops = {
		.hw_params = mtgpu_hdmi_audio_hw_params,
		.audio_startup = mtgpu_hdmi_audio_startup,
		.audio_shutdown = mtgpu_hdmi_audio_shutdown,
		.digital_mute = mtgpu_hdmi_audio_mute,
		.get_eld = mtgpu_hdmi_audio_get_eld,
	},
	.hook_plugged_cb = mtgpu_hdmi_audio_hook_plugged_cb,
};
#endif

static int mtgpu_hdmi_audio_register(struct mtgpu_hdmi *hdmi)
{
	struct hdmi_codec_pdata codec_data = {
		.ops = (struct hdmi_codec_ops *)&mtgpu_hdmi_audio_codec_ops,
		.max_i2s_channels = 2,
		.i2s = 1,
		.data = hdmi,
	};

	hdmi->hdmi_audio = platform_device_register_data(hdmi->dev, "mtgpu-hdmi-audio-codec",
							 PLATFORM_DEVID_AUTO, &codec_data,
							 sizeof(codec_data));
	if (IS_ERR(hdmi->hdmi_audio))
		return PTR_ERR(hdmi->hdmi_audio);

	mutex_init(&hdmi->codec_lock);

	return 0;
}

static void mtgpu_hdmi_audio_unregister(struct mtgpu_hdmi *hdmi)
{
	if (hdmi->hdmi_audio) {
		platform_device_unregister(hdmi->hdmi_audio);
		hdmi->hdmi_audio = NULL;
	}
}

/*
 * DDC I2C Adapter
 */

static int mtgpu_hdmi_i2c_xfer(struct i2c_adapter *ddc, struct i2c_msg *msgs, int num)
{
	struct mtgpu_hdmi *hdmi = i2c_get_adapdata(ddc);
	int i, j, ret = 0;

	mutex_lock(&hdmi->i2c_lock);

	for (i = 0; i < num; i++) {
		DRM_DEV_DEBUG(hdmi->dev,
			      "xfer: num: %d/%d, addr: 0x%x, len: %d, flags: %#x\n",
			      i + 1, num, msgs[i].addr, msgs[i].len, msgs[i].flags);

		if (msgs[i].flags & I2C_M_RD)
			ret = hdmi->core->i2c_read(&hdmi->ctx, msgs[i].addr,
						   msgs[i].buf, msgs[i].len);
		else
			ret = hdmi->core->i2c_write(&hdmi->ctx, msgs[i].addr,
						    msgs[i].buf, msgs[i].len);

		if (ret < 0)
			break;

		for (j = 0; j < msgs[i].len; j++)
			DRM_DEV_DEBUG(hdmi->dev, "0x%x\n", msgs[i].buf[j]);
	}

	if (!ret)
		ret = num;

	mutex_unlock(&hdmi->i2c_lock);

	return ret;
}

static u32 mtgpu_hdmi_i2c_func(struct i2c_adapter *ddc)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm mtgpu_hdmi_algorithm = {
	.master_xfer	= mtgpu_hdmi_i2c_xfer,
	.functionality	= mtgpu_hdmi_i2c_func,
};

static int mtgpu_hdmi_ddc_register(struct mtgpu_hdmi *hdmi)
{
	struct i2c_adapter *ddc;
	int ret;

	if (!hdmi->core->i2c_read || !hdmi->core->i2c_write) {
		DRM_DEV_ERROR(hdmi->dev, "hdmi ops i2c_read or i2c_write is null\n");
		return -ENODEV;
	}

	ddc = devm_kzalloc(hdmi->dev, sizeof(*ddc), GFP_KERNEL);
	if (!ddc)
		return -ENOMEM;

	ddc->owner = THIS_MODULE;
	ddc->class = I2C_CLASS_DDC;
	ddc->dev.parent = hdmi->dev;
	ddc->algo = &mtgpu_hdmi_algorithm;
	snprintf(ddc->name, sizeof(ddc->name), "mtgpu hdmi i2c");

	ret = i2c_add_adapter(ddc);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "failed to add I2C adapter: %d\n", ret);
		return ret;
	}

	i2c_set_adapdata(ddc, hdmi);
	hdmi->ddc = ddc;
#if defined(OS_STRUCT_DRM_CONNECTOR_HAS_DDC)
	hdmi->connector.ddc = ddc;
#endif

	mutex_init(&hdmi->i2c_lock);

	return 0;
}

static int mtgpu_hdmi_component_bind(struct device *dev,
				     struct device *master, void *data)
{
	struct mtgpu_hdmi_platform_data *pdata = dev_get_platdata(dev);
	struct platform_device *pdev = to_platform_device(dev);
	struct drm_device *drm = data;
	struct mtgpu_hdmi *hdmi;
	struct resource *res;
	int ret;
	struct mtgpu_hdmi_chip *chip;

	hdmi = kzalloc(sizeof(*hdmi), GFP_KERNEL);
	if (!hdmi)
		return -ENOMEM;

	hdmi->ctx.vm = kzalloc(sizeof(*hdmi->ctx.vm), GFP_KERNEL);
	if (!hdmi->ctx.vm) {
		DRM_DEV_ERROR(dev, "failed to create vm\n");
		ret = -ENOMEM;
		goto err_free_hdmi;
	}

	hdmi->ctx.waitq = kzalloc(sizeof(*hdmi->ctx.waitq), GFP_KERNEL);
	if (!hdmi->ctx.waitq) {
		DRM_DEV_ERROR(dev, "failed to create waitq\n");
		ret = -ENOMEM;
		goto err_free_hdmi;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hdmi-regs");
	if (!res) {
		DRM_DEV_ERROR(dev, "failed to get hdmi-regs\n");
		ret = -EIO;
		goto err_free_hdmi;
	}

	hdmi->ctx.regs = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(hdmi->ctx.regs)) {
		ret = PTR_ERR(hdmi->ctx.regs);
		goto err_free_hdmi;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		DRM_DEV_ERROR(dev, "failed to get HDMI irq number\n");
		ret = -EIO;
		goto err_free_hdmi;
	}

	ret = mtgpu_set_interrupt_handler(dev->parent->parent, res->start,
					  mtgpu_hdmi_irq_handler, hdmi);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to register HDMI irq handler\n");
		ret = -EINVAL;
		goto err_free_hdmi;
	}

	hdmi->ctx.irq = res->start;
	hdmi->ctx.max_hres = pdata->max_hres;
	hdmi->ctx.max_vres = pdata->max_vres;
	hdmi->ctx.max_pclk_100khz = pdata->max_pclk_100khz;
	hdmi->dev = dev;
	hdmi->port_type = pdata->port_type;
	dev_set_drvdata(dev, hdmi);
	INIT_DELAYED_WORK(&hdmi->hpd_work, mtgpu_hdmi_hpd_work);
	init_waitqueue_head(hdmi->ctx.waitq);

	chip = &mtgpu_hdmi_chip_sudi,

	hdmi->core = chip->core;
	if (!hdmi->core) {
		DRM_DEV_ERROR(dev, "hdmi core ops is null\n");
		ret = -EINVAL;
		goto err_free_hdmi;
	}

	hdmi->glb = chip->glb;
	if (!hdmi->glb) {
		DRM_DEV_ERROR(dev, "hdmi glb ops is null\n");
		ret = -EINVAL;
		goto err_free_hdmi;
	}

	/* disable first in case it's enabled by bios */
	if (hdmi->core->disable)
		hdmi->core->disable(&hdmi->ctx);

	if (hdmi->glb->init)
		hdmi->glb->init(&hdmi->ctx);

	if (hdmi->core->hw_init)
		hdmi->core->hw_init(&hdmi->ctx);

	if (hdmi->core->ctx_init)
		hdmi->core->ctx_init(&hdmi->ctx);

	ret = mtgpu_hdmi_ddc_register(hdmi);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to register HDMI ddc adapter: %d\n", ret);
		goto err_free_hdmi;
	}

	ret = mtgpu_hdmi_audio_register(hdmi);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to register HDMI audio device: %d\n", ret);
		goto err_free_hdmi;
	}

	mtgpu_hdmi_drm_init(hdmi, drm);

	if (hdmi->core->hpd_enable)
		hdmi->core->hpd_enable(&hdmi->ctx);

	ret = mtgpu_enable_interrupt(dev->parent->parent, res->start);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to enable HDMI irq\n");
		ret = -EINVAL;
		goto err_free_hdmi;
	}

	DRM_DEV_INFO(dev, "mtgpu HDMI driver loaded successfully\n");

	return 0;

err_free_hdmi:
	kfree(hdmi->ctx.waitq);
	kfree(hdmi->ctx.vm);
	kfree(hdmi);

	return ret;
}

static void mtgpu_hdmi_component_unbind(struct device *dev,
					struct device *master, void *data)
{
	int ret = 0;
	struct mtgpu_hdmi *hdmi = dev_get_drvdata(dev);

	if (hdmi->core->hw_deinit)
		hdmi->core->hw_deinit(&hdmi->ctx);

	ret = mtgpu_disable_interrupt(dev->parent->parent, hdmi->ctx.irq);
	if (ret)
		DRM_DEV_ERROR(dev, "failed to disable HDMI irq\n");

	ret = mtgpu_set_interrupt_handler(dev->parent->parent, hdmi->ctx.irq, NULL, NULL);
	if (ret)
		DRM_DEV_ERROR(dev, "failed to deregister HDMI irq handler\n");

	mtgpu_hdmi_audio_unregister(hdmi);

	if (hdmi->ddc)
		i2c_del_adapter(hdmi->ddc);

	if (hdmi->core->ctx_exit)
		hdmi->core->ctx_exit(&hdmi->ctx);

	kfree(hdmi->ctx.waitq);
	kfree(hdmi->ctx.vm);
	kfree(hdmi);

	DRM_DEV_INFO(dev, "unload mtgpu HDMI driver\n");
}

static const struct component_ops mtgpu_hdmi_component_ops = {
	.bind   = mtgpu_hdmi_component_bind,
	.unbind = mtgpu_hdmi_component_unbind,
};

static int mtgpu_hdmi_probe(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &mtgpu_hdmi_component_ops);
}

static int mtgpu_hdmi_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &mtgpu_hdmi_component_ops);
	return 0;
}

static struct platform_device_id mtgpu_hdmi_device_id[] = {
	{ .name = MTGPU_DEVICE_NAME_HDMI, },
	{ },
};

static int mtgpu_hdmi_resume(struct device *dev)
{
	struct mtgpu_hdmi *hdmi = dev_get_drvdata(dev);

	DRM_DEV_INFO(hdmi->dev, "mtgpu hdmi device resume early enter\n");

	/* disable first in case it's enabled by bios */
	if (hdmi->core->disable)
		hdmi->core->disable(&hdmi->ctx);

	if (hdmi->core->hw_init)
		hdmi->core->hw_init(&hdmi->ctx);

	/*
	* The audio service (eg. pluseaudio) will not response to the
	* codec_plugged_cb() in S3/S4 suspend and resume stage since
	* it was freezeed. Restore the audio status if it was enabled
	* before encoder suspend.
	*/
	if (hdmi->core->audio_enable && hdmi->audio_enabled)
		hdmi->core->audio_enable(&hdmi->ctx);

	if (hdmi->core->is_plugin)
		hdmi->connected = hdmi->core->is_plugin(&hdmi->ctx);

	DRM_DEV_INFO(hdmi->dev, "mtgpu hdmi device resume early exit\n");

	return 0;
}

static void mtgpu_hdmi_complete(struct device *dev)
{
	struct mtgpu_hdmi *hdmi = dev_get_drvdata(dev);

	DRM_DEV_INFO(hdmi->dev, "mtgpu hdmi complete enter\n");

	if (hdmi->core->hpd_enable)
		hdmi->core->hpd_enable(&hdmi->ctx);

	DRM_DEV_INFO(hdmi->dev, "mtgpu hdmi complete exit\n");
}

const struct dev_pm_ops mtgpu_hdmi_pm_ops = {
	/* this will be called before mtgpu_drm_resume */
	.resume_early	= mtgpu_hdmi_resume,
	.restore_early	= mtgpu_hdmi_resume,
	/* this will be called after mtgpu_drm_resume */
	.complete = mtgpu_hdmi_complete,
};

static void mtgpu_hdmi_shutdown(struct platform_device *pdev)
{
	struct mtgpu_hdmi *hdmi = dev_get_drvdata(&pdev->dev);

	DRM_DEV_DEBUG(hdmi->dev, "%s()\n", __func__);

	if (hdmi->core->hw_deinit)
		hdmi->core->hw_deinit(&hdmi->ctx);

	if (hdmi->core->disable)
		hdmi->core->disable(&hdmi->ctx);
}

struct platform_driver mtgpu_hdmi_driver = {
	.probe    = mtgpu_hdmi_probe,
	.remove   = mtgpu_hdmi_remove,
	.shutdown = mtgpu_hdmi_shutdown,
	.driver   = {
		.owner  = THIS_MODULE,
		.name   = "mtgpu-hdmi-drv",
		.pm	= &mtgpu_hdmi_pm_ops,
	},
	.id_table = mtgpu_hdmi_device_id,
};

