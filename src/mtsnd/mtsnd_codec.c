// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <sound/tlv.h>
#include <sound/jack.h>
#include <sound/control.h>
#include <sound/hdmi-codec.h>

#include "mtsnd_codec.h"
#include "mtsnd_conf.h"
#include "mtsnd_debug.h"
#include "mtsnd_pcm_hw.h"
#include "eld.h"

#if !defined (OS_STRUCT_HDMI_CODEC_OPS_HAS_HOOK_PLUGGED_CB)
typedef void (*hdmi_codec_plugged_cb)(struct device *dev, bool plugged);

typedef int (*hook_plugged_callback)(struct device *dev, void *data,
				     hdmi_codec_plugged_cb fn, struct device *codec_dev);

struct mtsnd_audio_codec_ops {
	struct hdmi_codec_ops old_hdmi_audio_codec_ops;
	hook_plugged_callback hook_plugged_cb;
};
#endif

#if defined (OS_BUS_FIND_DEVICE_MATCH_USE_CONST_MODIFIER)
int pdev_match_name(struct device *dev, const void *data)
#else
int pdev_match_name(struct device *dev, void *data)
#endif
{
	struct platform_device *pdev = to_platform_device(dev);

	if (!strcmp(pdev->name, (char*)data))
		return 1;

	return 0;
}

static int get_codec_index(struct mtsnd_chip *chip, struct snd_kcontrol *kcontrol)
{
	int codec_index = -1;
	int i = 0;

	for (i = 0; i < get_codec_count(chip); i++) {
		if (strstr(kcontrol->id.name, get_jack_name(chip, i)) != NULL) {
			codec_index = i;
			break;
		}
	}

	return codec_index;
}

int mtsnd_mixer_switch_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct mtsnd_chip *chip = snd_kcontrol_chip(kcontrol);
	int index = get_codec_index(chip, kcontrol);

	if (index < 0) {
		dev_err(chip->card->dev, "get error index\n");
		return -EIO;
	}

	ucontrol->value.integer.value[0] = check_codec_state2(&chip->codec[index]);

	SND_DEBUG("codec: %s get: %ld, comm:%s, pid:%d\n", kcontrol->id.name,
		 ucontrol->value.integer.value[0], current->comm, current->pid);
	return 0;
}

int mtsnd_mixer_switch_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct mtsnd_chip *chip = snd_kcontrol_chip(kcontrol);
	int index = get_codec_index(chip, kcontrol);
	struct mtsnd_codec *codec;
	int changed = 0;

	SND_DEBUG("codec:%s set:%ld, comm:%s, pid:%d\n", kcontrol->id.name,
		 ucontrol->value.integer.value[0], current->comm, current->pid);

	if (index < 0)
		return -EIO;

	codec = &chip->codec[index];


	changed = codec_status_changed(codec, ucontrol->value.integer.value[0]);

	if (!check_codec_state1(codec))
		return changed;

	if (check_codec_state2(codec)) {
		/* if the IIS is not working, we can't enable the monitor audio
		 * but save the status
		 */
		if (!chip->pcm_running) {
			SND_DEBUG("IIS not running, mark enabled\n");
			return changed;
		}

		if (check_codec_state3(codec) && codec->hcd->ops->hw_params) {
			int ret = -1;

			ret = codec->hcd->ops->hw_params(codec->dev, codec->hcd->data,
							 chip->daifmt, chip->params);
			update_codec_state3(codec, ret);
		}

		/* codec enable */
		if (check_codec_start(chip, index) && codec->hcd->ops->audio_startup)
			codec->hcd->ops->audio_startup(codec->dev, codec->hcd->data);
	} else {
		/* codec disable */
		if (check_hw_status(chip, index) && codec->hcd->ops->audio_shutdown)
			codec->hcd->ops->audio_shutdown(codec->dev, codec->hcd->data);
	}

	return changed;
}

static void deal_pnp_event(struct mtsnd_chip *chip, int codec_index, bool connected)
{
	struct mtsnd_codec *codec = &chip->codec[codec_index];

	SND_DEBUG("codec_index %d %s\n", codec_index, connected ? "connected" : "disconnected");

	update_codec_state4(codec, 1);
	/* shutdown the display audio while codec enters state4 */
	if (codec_safety_check_stop(chip, codec_index) && codec->hcd->ops->audio_shutdown)
		codec->hcd->ops->audio_shutdown(codec->dev, codec->hcd->data);

	if (connected) {
		/* check if the monitor support pcm audio */
		if (mtsnd_check_eld(chip, codec_index)) {
			dev_info(chip->card->dev, "monitor plug in, but don't support audio\n");
			return;
		}
		chip->jack_timer[codec_index]->codec_index = codec_index;
		chip->jack_timer[codec_index]->jack_status = SND_JACK_AVOUT;
		mod_timer(&chip->jack_timer[codec_index]->timer, jiffies + msecs_to_jiffies(2000));
	} else {
		chip->jack_timer[codec_index]->codec_index = codec_index;
		chip->jack_timer[codec_index]->jack_status = 0;
		mod_timer(&chip->jack_timer[codec_index]->timer, jiffies + msecs_to_jiffies(2000));
	}
}

static void report_pnp_event(struct timer_list *t)
{
	struct mtsnd_timer * jack_timer = from_timer(jack_timer, t, timer);
	struct mtsnd_chip *chip = jack_timer->chip;
	int codec_index = jack_timer->codec_index;
	int jack_status = jack_timer->jack_status;
	struct mtsnd_codec *codec = &chip->codec[codec_index];
	SND_DEBUG("codec_index %d jack_status %d\n", codec_index, jack_status);

	if (jack_status == SND_JACK_AVOUT) {
		SND_DEBUG("%s plug in\n", codec->eld->monitor_name);
		snd_jack_report(chip->jack[codec_index], SND_JACK_AVOUT);

		/* enable the display audio when codec exits state4 */
		if (check_codec_state2(codec)) {
			/* set the audio config for display */
			if (codec->hcd->ops->hw_params) {
				int ret = -1;

				ret = codec->hcd->ops->hw_params(codec->dev, codec->hcd->data,
								chip->daifmt, chip->params);
				update_codec_state3(codec, ret);
			}
			/* start the display audio */
			if (codec_safety_check_start(chip, codec_index) && codec->hcd->ops->audio_startup)
				codec->hcd->ops->audio_startup(codec->dev, codec->hcd->data);
		} else {
			update_codec_state3(codec, 1);
		}
	} else {
		SND_DEBUG("%s plug out\n", codec->eld->monitor_name);
		snd_jack_report(chip->jack[codec_index], 0);
		memset(codec->eld, 0, sizeof(*codec->eld));
		update_codec_state3(codec, 1);
	}

	update_codec_state4(codec, 0);
}

void cancel_all_pnp_event(struct mtsnd_chip *chip) {
	int i;
	for (i = 0; i < get_codec_count(chip); i++)
		del_timer_sync(&chip->jack_timer[i]->timer);
}

static struct mtsnd_chip *to_chip(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);

	return (struct mtsnd_chip *)card->private_data;
}

void mtsnd_codec_hot_plug_dp0(struct device *dev, bool connected)
{
	struct mtsnd_chip *chip = to_chip(dev);
	int codec_index = 0;

	SND_DEBUG("dp0 hot plug detect, val: %d\n", connected);
	deal_pnp_event(chip, codec_index, connected);
}

void mtsnd_codec_hot_plug_dp1(struct device *dev, bool connected)
{
	struct mtsnd_chip *chip = to_chip(dev);
	int codec_index = 1;

	SND_DEBUG("dp1 hot plug detect, val: %d\n", connected);
	deal_pnp_event(chip, codec_index, connected);
}

void mtsnd_codec_hot_plug_dp2(struct device *dev, bool connected)
{
	struct mtsnd_chip *chip = to_chip(dev);
	int codec_index = 2;

	SND_DEBUG("dp2 hot plug detect, val: %d\n", connected);
	deal_pnp_event(chip, codec_index, connected);
}

void mtsnd_codec_hot_plug_dp3(struct device *dev, bool connected)
{
	struct mtsnd_chip *chip = to_chip(dev);
	int codec_index = 3;

	SND_DEBUG("dp3 hot plug detect, val: %d\n", connected);
	deal_pnp_event(chip, codec_index, connected);
}

void mtsnd_codec_hot_plug_hdmi(struct device *dev, bool connected)
{
	struct mtsnd_chip *chip = to_chip(dev);
	int codec_index = 2;

	SND_DEBUG("hdmi hot plug detect, val: %d\n", connected);
	deal_pnp_event(chip, codec_index, connected);
}

struct device *find_device_by_name(struct mtsnd_chip *chip, char *name)
{
	struct device *dev = NULL;
	int i = 0;

	for (;; i++) {
		dev = bus_find_device(&platform_bus_type, dev, name, pdev_match_name);
		if (i == chip->idx)
			return dev;
	}

	return NULL;
}

static struct snd_kcontrol_new mtsnd_kctls[4];

int mtsnd_create_codec(struct mtsnd_chip *chip)
{
#if !defined (OS_STRUCT_HDMI_CODEC_OPS_HAS_HOOK_PLUGGED_CB)
	const struct mtsnd_audio_codec_ops *ops;
#else
	const struct hdmi_codec_ops *ops;
#endif
	struct hdmi_codec_pdata *hcd;
	struct platform_device *pdev;
	struct device *dev;
	int i, count;

	count = get_codec_count(chip);

	for (i = 0; i < count; i++) {
		struct snd_jack *jack = NULL;
		int ret = -1;

		ret = snd_jack_new(chip->card, get_jack_name(chip, i),
				       SND_JACK_AVOUT, &jack, true, true);
		if (ret < 0) {
			dev_err(chip->card->dev, "Error snd_jack_new\n");
			continue;
		}
		chip->jack[i] = jack;

		chip->jack_timer[i] = kzalloc(sizeof(struct mtsnd_timer), GFP_KERNEL);
		chip->jack_timer[i]->chip = chip;
		timer_setup(&chip->jack_timer[i]->timer, report_pnp_event, 0);
	}

	for (i = 0; i < count; i++) {
		struct snd_kcontrol *kctl = NULL;
		int ret = -1;

		mtsnd_kctls[i].name = get_kctrl_name(chip, i);
		mtsnd_kctls[i].iface = SNDRV_CTL_ELEM_IFACE_MIXER;
		mtsnd_kctls[i].access = SNDRV_CTL_ELEM_ACCESS_READWRITE;
		mtsnd_kctls[i].info = snd_ctl_boolean_mono_info;
		mtsnd_kctls[i].get = mtsnd_mixer_switch_get;
		mtsnd_kctls[i].put = mtsnd_mixer_switch_put;

		kctl = snd_ctl_new1(&mtsnd_kctls[i], chip);
		ret = snd_ctl_add(chip->card, kctl);
		if (ret < 0) {
			dev_err(chip->card->dev, "Error snd_ctl_add\n");
			continue;
		}
		chip->kctl[i] = kctl;
	}

	for (i = 0; i < count; i++) {
		dev = find_device_by_name(chip, (char *)get_codec_name(chip, i));
		if (dev) {
			hcd = (struct hdmi_codec_pdata *)dev_get_platdata(dev);
			if (hcd) {
#if !defined (OS_STRUCT_HDMI_CODEC_OPS_HAS_HOOK_PLUGGED_CB)
				ops = (struct mtsnd_audio_codec_ops *)hcd->ops;
#else
				ops = (struct hdmi_codec_ops *)hcd->ops;
#endif
				if (ops) {
					if (ops->hook_plugged_cb) {
						chip->codec[i].eld = kzalloc(sizeof(*chip->codec[i].eld), GFP_KERNEL);
						if (!chip->codec[i].eld)
							return -ENOMEM;

						chip->codec[i].dev = dev;
						chip->codec[i].hcd = hcd;
						codec_status_init(&chip->codec[i]);
						pdev = to_platform_device(dev);
						platform_set_drvdata(pdev, chip);
						ops->hook_plugged_cb(chip->codec[i].dev,
								     chip->codec[i].hcd->data,
								     get_codec_cb(chip, i),
								     chip->card->dev);
					}
				}
			}
		}
	}

	return 0;
}

void mtsnd_free_codec(struct mtsnd_chip *chip)
{
	int i;

	for (i = 0; i < get_codec_count(chip); i++) {
		struct mtsnd_codec *codec = &chip->codec[i];

		if (check_codec_state1(codec)) {
#if !defined (OS_STRUCT_HDMI_CODEC_OPS_HAS_HOOK_PLUGGED_CB)
			const struct mtsnd_audio_codec_ops *ops = (struct mtsnd_audio_codec_ops *)codec->hcd->ops;
#else
			const struct hdmi_codec_ops *ops = codec->hcd->ops;
#endif
			if (ops && ops->hook_plugged_cb)
				ops->hook_plugged_cb(codec->dev, codec->hcd->data, NULL, NULL);

			kfree(codec->eld);
		}
		del_timer_sync(&chip->jack_timer[i]->timer);
		kfree(chip->jack_timer[i]);
	}
}
