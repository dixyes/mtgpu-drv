// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <sound/hdmi-codec.h>
#include <sound/core.h>
#include <sound/pcm.h>

#include "mtsnd_drv.h"
#include "mtsnd_irq.h"
#include "mtsnd_conf.h"
#include "mtsnd_debug.h"
#include "mtsnd_codec.h"
#include "mtsnd_pcm_hw.h"
#include "mtgpu_ipc.h"
#include "eld.h"

#define MT_HW_BUFBYTE_MAX	(48 << 10)
#define MT_HW_BUFBYTE_SIZE	(12 << 10)

#define MT_HW_PERIOD_MIN	2
#define MT_HW_PERIOD_MAX	4

//#define USING_INBOUND   //this is just for test

static const struct snd_pcm_hardware mtsnd_pcm_hw_gen1 = {
	.info =			SNDRV_PCM_INFO_INTERLEAVED,
	.formats =		SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S16_BE |
				SNDRV_PCM_FMTBIT_U16_LE | SNDRV_PCM_FMTBIT_U16_BE |
				SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_U18_3BE |
				SNDRV_PCM_FMTBIT_U18_3LE | SNDRV_PCM_FMTBIT_U18_3BE |
				SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S20_3BE |
				SNDRV_PCM_FMTBIT_U20_3LE | SNDRV_PCM_FMTBIT_U20_3BE |
				SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S24_3BE |
				SNDRV_PCM_FMTBIT_U24_3LE | SNDRV_PCM_FMTBIT_U24_3BE,
	.rates =		SNDRV_PCM_RATE_8000_192000,
	.rate_min =		8000,
	.rate_max =		192000,
	.channels_min =		2,
	.channels_max =		8,
	.buffer_bytes_max =	MT_HW_BUFBYTE_MAX,
	.period_bytes_min =	MT_HW_BUFBYTE_SIZE / MT_HW_PERIOD_MAX,
	.period_bytes_max =	MT_HW_BUFBYTE_MAX / MT_HW_PERIOD_MIN,
	.periods_min =		MT_HW_PERIOD_MIN,
	.periods_max =		MT_HW_PERIOD_MAX,
	.fifo_size =		0,
};

static const struct snd_pcm_hardware mtsnd_pcm_hw_gen2 = {
	.info =			SNDRV_PCM_INFO_INTERLEAVED,
	.formats =		SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S16_BE |
				SNDRV_PCM_FMTBIT_U16_LE | SNDRV_PCM_FMTBIT_U16_BE |
				SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_U18_3BE |
				SNDRV_PCM_FMTBIT_U18_3LE | SNDRV_PCM_FMTBIT_U18_3BE |
				SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S20_3BE |
				SNDRV_PCM_FMTBIT_U20_3LE | SNDRV_PCM_FMTBIT_U20_3BE |
				SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S24_3BE |
				SNDRV_PCM_FMTBIT_U24_3LE | SNDRV_PCM_FMTBIT_U24_3BE,
	.rates =		SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_64000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_192000,
	.rate_min =		32000,
	.rate_max =		192000,
	.channels_min =		2,
	.channels_max =		8,
	.buffer_bytes_max =	MT_HW_BUFBYTE_MAX,
	.period_bytes_min =	MT_HW_BUFBYTE_SIZE / MT_HW_PERIOD_MAX,
	.period_bytes_max =	MT_HW_BUFBYTE_MAX / MT_HW_PERIOD_MIN,
	.periods_min =		MT_HW_PERIOD_MIN,
	.periods_max =		MT_HW_PERIOD_MAX,
	.fifo_size =		0,
};

static int mtsnd_pcm_open(struct snd_pcm_substream *substream)
{
	int gen = -1;
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	dev_info(chip->card->dev, "PCM open, comm:%s, pid:%d\n", current->comm, current->pid);

	if (chip->open_pcm || chip->open_compr) {
		dev_err(chip->card->dev, "Already open pcm/compr\n");
		return -EBUSY;
	}

	/* pcm hw init */
	gen = mtsnd_hw_init(chip);
	switch (gen) {
	case CHIP_GEN1:
		runtime->hw = mtsnd_pcm_hw_gen1;
		break;
	case CHIP_GEN2:
		runtime->hw = mtsnd_pcm_hw_gen2;
		break;
	default:
		return -ENXIO;
	}

	chip->open_pcm = 1;
	chip->substream = substream;

	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 128);
	snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 128);
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_enable(chip);
#endif
	return 0;
}

static int mtsnd_pcm_close(struct snd_pcm_substream *substream)
{
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);

	dev_info(chip->card->dev, "PCM close\n");

	chip->substream = NULL;
	chip->open_pcm = 0;
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_disable(chip);
#endif
	return 0;
}

static int mtsnd_pcm_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params)
{
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err;

	dev_info(chip->card->dev, "PCM params, %u bytes\n", params_buffer_bytes(params));

#ifndef USING_INBOUND
	err = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
#else
	/* this is just for internal debug */
	err = snd_pcm_lib_malloc_pages_inbound(substream, params_buffer_bytes(params));
#endif
	if (err < 0) {
		dev_err(chip->card->dev, "Error snd_pcm_lib_malloc_pages\n");
		return err;
	}

	/* set the hw buffer */
	chip->pcm_device_addr = mtsnd_pcm_ata_buffer(chip, runtime->dma_addr);

	return 0;
}

static int mtsnd_pcm_hw_free(struct snd_pcm_substream *substream)
{
#ifndef USING_INBOUND
	return snd_pcm_lib_free_pages(substream);
#else
	return 0;
#endif
}

static int mtsnd_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct pcm_info pcm;
	int i, err;

	dev_info(chip->card->dev,
		 "PCM prepare, %d%s ch %d rate %d, period_bytes:%ld, HW_bytes:%ld\n",
		 runtime->sample_bits,
		 snd_pcm_format_signed(runtime->format) ? "S" : "U", runtime->channels,
		 runtime->rate, snd_pcm_lib_period_bytes(substream), runtime->dma_bytes);

	pcm.rate = runtime->rate;
	pcm.bit_depth = runtime->sample_bits;
	pcm.channels = runtime->channels;
	pcm.period_bytes = snd_pcm_lib_period_bytes(substream);
	pcm.dma_bytes = runtime->dma_bytes;
	pcm.big_endian = snd_pcm_format_big_endian(runtime->format);

	err = mtsnd_clock_set(chip, &pcm);
	if (err < 0) {
		dev_err(chip->card->dev, "prepare step fail to set the clock\n");
		return err;
	}

	chip->daifmt->fmt = HDMI_I2S;
	chip->params->sample_rate = runtime->rate;
	chip->params->channels = runtime->channels;
	chip->params->sample_width = runtime->sample_bits;

	/*
	 * In some cases, pulseaudio will prepare the IIS once, but on/off the monitor more than
	 * once because of monitor PNP. And we need to make sure that put interface has the ability
	 * to program the config for monitors. So we can set pcm_running to 1 here. Pulseaudio may
	 * set put 1 and then trigger start. If we dont't set pcm_running here, put interface can't
	 * config for monitors successfully.
	 */
	chip->pcm_running = 1;

	for (i = 0; i < get_codec_count(chip); i++) {
		struct mtsnd_codec *codec = &chip->codec[i];

		/* set the audio config for monitors */
		if (check_codec_state1(codec) && codec->hcd->ops->hw_params) {
			int ret = -1;

			ret = codec->hcd->ops->hw_params(codec->dev, codec->hcd->data,
							 chip->daifmt, chip->params);
			update_codec_state3(codec, ret);
		}
	}

	return 0;
}

static int mtsnd_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);
	int i, err = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		mtsnd_snd_start(chip);
		/* enable the monitor audio */
		for (i = 0; i < get_codec_count(chip); i++) {
			struct mtsnd_codec *codec = &chip->codec[i];

			if (codec_safety_check_start(chip, i) && codec->hcd->ops->audio_startup)
				codec->hcd->ops->audio_startup(codec->dev, codec->hcd->data);
		}
		dev_info(chip->card->dev, "trigger start\n");
		break;

#if defined (OS_ENUM_SNDRV_PCM_TRIGGER_SUSPEND_EXIST)
	case SNDRV_PCM_TRIGGER_SUSPEND:
#endif
	case SNDRV_PCM_TRIGGER_STOP:
		chip->pcm_running = 0;
		/* disable the monitor audio output because of IIS closing */
		for (i = 0; i < get_codec_count(chip); i++) {
			struct mtsnd_codec *codec = &chip->codec[i];

			if (codec_safety_check_stop(chip, i) &&
			    codec->hcd->ops->audio_shutdown)
				codec->hcd->ops->audio_shutdown(codec->dev, codec->hcd->data);
		}
		mtsnd_snd_stop(chip);
		dev_info(chip->card->dev, "trigger stop/suspend %d\n", cmd);
		break;

	default:
		dev_info(chip->card->dev, "trigger %d\n", cmd);
		err = -EINVAL;
		break;
	}

	return err;
}

static snd_pcm_uframes_t mtsnd_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned int bytes = mtsnd_snd_pointer(chip);

	if (bytes >= runtime->dma_bytes)
		bytes = 0;

	return bytes_to_frames(runtime, bytes);
}

static int mtsnd_pcm_ack(struct snd_pcm_substream *substream)
{
	struct mtsnd_chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned int appl_ofs = runtime->control->appl_ptr % runtime->buffer_size;
	unsigned int bytes = frames_to_bytes(runtime, appl_ofs);

	mtsnd_snd_ack(chip, runtime->control->appl_ptr, runtime->buffer_size,
		      runtime->dma_bytes, bytes);

	return 0;
}

static const struct snd_pcm_ops mtsnd_pcm_ops = {
	.ioctl = snd_pcm_lib_ioctl,
	.open = mtsnd_pcm_open,
	.close = mtsnd_pcm_close,
	.hw_params = mtsnd_pcm_hw_params,
	.hw_free = mtsnd_pcm_hw_free,
	.prepare = mtsnd_pcm_prepare,
	.trigger = mtsnd_pcm_trigger,
	.pointer = mtsnd_pcm_pointer,
	.ack = mtsnd_pcm_ack,
#ifdef USING_INBOUND
	.copy_user = mtsnd_copy_user,
#endif
};

int mtsnd_create_pcm(struct mtsnd_chip *chip)
{
	struct snd_pcm *pcm = NULL;
	int dev = 0, err = 0;

	err = snd_pcm_new(chip->card, "MooreThreads", dev, 1, 0, &pcm);
	if (err < 0) {
		dev_err(chip->card->dev, "Error snd_pcm_new\n");
		return err;
	}

	chip->params = kzalloc(sizeof(*chip->params), GFP_KERNEL);
	if (!chip->params)
		return -ENOMEM;

	chip->daifmt = kzalloc(sizeof(*chip->daifmt), GFP_KERNEL);
	if (!chip->daifmt)
		return -ENOMEM;

	strcpy(pcm->name, "Display");
	pcm->private_data = chip;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &mtsnd_pcm_ops);
	/* buffer pre-allocation */
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV,
						&chip->pci->dev,
						MT_HW_BUFBYTE_SIZE, MT_HW_BUFBYTE_MAX);

	return 0;
}

#define I2S_INT_STAT_EMPTY 1
#define I2S_INT_STAT_COUNT 2

void mtsnd_handle_pcm(struct mtsnd_chip *chip)
{
	unsigned int handle = 0;

	handle = mtsnd_snd_irq_handle(chip);

	if (!chip->pcm_running)
		return;

	if (handle & I2S_INT_STAT_EMPTY) {
		snd_pcm_stop_xrun(chip->substream);
		pr_warn_ratelimited("mtsnd pcm, xrun\n");
	} else if (handle & I2S_INT_STAT_COUNT)
		snd_pcm_period_elapsed(chip->substream);
}

void mtsnd_free_pcm(struct mtsnd_chip *chip)
{
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_disable(chip);
#endif
	kfree(chip->params);
	kfree(chip->daifmt);
}

void mtsnd_reset_pcm(struct mtsnd_chip *chip)
{
}

#ifdef CONFIG_PM_SLEEP
void mtsnd_suspend_pcm(struct mtsnd_chip *chip)
{
	dev_info(chip->card->dev, "PCM suspend\n");

	cancel_all_pnp_event(chip);

	if (!chip->substream || !chip->substream->runtime) {
		SND_DEBUG("no substream or not running, just skip\n");
		return;
	}
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_disable(chip);
#endif
	if (chip->substream->runtime->status->suspended_state == SNDRV_PCM_STATE_RUNNING)
		mtsnd_pcm_trigger(chip->substream, SNDRV_PCM_TRIGGER_STOP);

	mtsnd_do_pcm_suspend(chip);
}

void mtsnd_resume_pcm(struct mtsnd_chip *chip)
{
	dev_info(chip->card->dev, "PCM resume\n");

	/* The card will lose power during S3, so clear the SW status */
	chip->iis_clock = 0;
	if (!chip->substream || !chip->substream->runtime) {
		SND_DEBUG("no substream or not running, just skip\n");
		return;
	}

	mtsnd_do_pcm_resume(chip, chip->substream->runtime->dma_addr);
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_enable(chip);
#endif
}
#endif
