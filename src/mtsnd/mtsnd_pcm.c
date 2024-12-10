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
#include "eld.h"

#define MT_HW_BUFBYTE_MAX	(48 << 10)
#define MT_HW_BUFBYTE_SIZE	(12 << 10)

#define MT_HW_PERIOD_MIN	2
#define MT_HW_PERIOD_MAX	4

#define snd_pcm_substream_get_chip(substream)	(((struct mtsnd_pcm*)substream->private_data)->private_data)
#define snd_pcm_substream_get_index(substream)	(((struct mtsnd_pcm*)substream->private_data)->index)

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
	.channels_max =		2,
	.buffer_bytes_max =	MT_HW_BUFBYTE_MAX,
	.period_bytes_min =	MT_HW_BUFBYTE_SIZE / MT_HW_PERIOD_MAX,
	.period_bytes_max =	MT_HW_BUFBYTE_MAX / MT_HW_PERIOD_MIN,
	.periods_min =		MT_HW_PERIOD_MIN,
	.periods_max =		MT_HW_PERIOD_MAX,
	.fifo_size =		0,
};

static const struct snd_pcm_hardware mtsnd_pcm_hw_gen2_to_gen4 = {
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
	.channels_max =		2,
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
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	dev_info(chip->card->dev, "PCM%d open, comm:%s, pid:%d\n", pcm_idx, current->comm, current->pid);

	if (chip->pcm[pcm_idx].open_pcm || chip->open_compr) {
		dev_err(chip->card->dev, "Already open pcm/compr\n");
		return -EBUSY;
	}

	/* pcm hw init */
	gen = mtsnd_hw_init(chip, pcm_idx);
	switch (gen) {
	case CHIP_GEN1:
		runtime->hw = mtsnd_pcm_hw_gen1;
		break;
	case CHIP_GEN2:
	case CHIP_GEN3:
		runtime->hw = mtsnd_pcm_hw_gen2_to_gen4;
		break;
	default:
		return -ENXIO;
	}

	chip->pcm[pcm_idx].open_pcm = 1;
	chip->pcm[pcm_idx].substream = substream;

	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 128);
	snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 128);
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_enable(chip, pcm_idx);
#endif
	return 0;
}

static int mtsnd_pcm_close(struct snd_pcm_substream *substream)
{
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);

	dev_info(chip->card->dev, "PCM%d close\n", pcm_idx);

	chip->pcm[pcm_idx].substream = NULL;
	chip->pcm[pcm_idx].open_pcm = 0;
#ifdef INTERRUPT_DEBUG
	mtsnd_pcm_irq_disable(chip, pcm_idx);
#endif
	return 0;
}

static int mtsnd_pcm_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params)
{
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err;

	dev_info(chip->card->dev, "PCM%d params, %u bytes\n", pcm_idx, params_buffer_bytes(params));

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
	mtsnd_pcm_ata_buffer(chip, pcm_idx, runtime->dma_addr, params_buffer_bytes(params));

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
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct pcm_info pcm;
	int i, err;

	dev_info(chip->card->dev,
		 "PCM%d prepare, %d%s ch %d rate %d, period_bytes:%ld, HW_bytes:%ld\n",
		 pcm_idx, runtime->sample_bits,
		 snd_pcm_format_signed(runtime->format) ? "S" : "U", runtime->channels,
		 runtime->rate, snd_pcm_lib_period_bytes(substream), runtime->dma_bytes);

	pcm.rate = runtime->rate;
	pcm.bit_depth = runtime->sample_bits;
	pcm.channels = runtime->channels;
	pcm.period_bytes = snd_pcm_lib_period_bytes(substream);
	pcm.dma_bytes = runtime->dma_bytes;
	pcm.big_endian = snd_pcm_format_big_endian(runtime->format);

	err = mtsnd_clock_set(chip, &pcm, pcm_idx);
	if (err < 0) {
		dev_err(chip->card->dev, "prepare step fail to set the clock\n");
		return err;
	}

	/*
	 * In some cases, pulseaudio will prepare the IIS once, but on/off the monitor more than
	 * once because of monitor PNP. And we need to make sure that put interface has the ability
	 * to program the config for monitors. So we can set pcm_running to 1 here. Pulseaudio may
	 * set put 1 and then trigger start. If we dont't set pcm_running here, put interface can't
	 * config for monitors successfully.
	 */
	chip->pcm[pcm_idx].pcm_running = 1;

	for (i = 0; i < get_codec_count(chip); i++) {
		struct mtsnd_codec *codec = chip->pcm[pcm_idx].codec[i];
		if (codec) {
			codec->daifmt->fmt = HDMI_I2S;
			codec->params->sample_rate = runtime->rate;
			codec->params->channels = runtime->channels;
			codec->params->sample_width = runtime->sample_bits;

			/* set the audio config for monitors */
			if (check_codec_state1(codec) && codec->hcd->ops->hw_params) {
				int ret = -1;

				ret = codec->hcd->ops->hw_params(codec->dev, codec->hcd->data,
								codec->daifmt, codec->params);
				update_codec_state3(codec, ret);
				if (ret)
					SND_DEBUG("PCM%d prepare codec%d hw_params failed %d\n", pcm_idx, i, ret);
			} else
				SND_DEBUG("PCM%d prepare codec%d hw_params not called\n", pcm_idx, i);
		}
	}

	return 0;
}

static int mtsnd_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);
	int i;
	int err = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		mtsnd_snd_start(chip, pcm_idx);
		/* enable the monitor audio */
		for (i = 0; i < get_codec_count(chip); i++) {
			int ret = -1;
			struct mtsnd_codec *codec = chip->pcm[pcm_idx].codec[i];

			if (codec && codec_safety_check_start(chip, i) && codec->hcd->ops->audio_startup) {
				ret = codec->hcd->ops->audio_startup(codec->dev, codec->hcd->data);
				if (ret)
					SND_DEBUG("PCM%d trigger start codec%d audio_startup failed %d\n", pcm_idx, i, ret);
			}
			else
				SND_DEBUG("PCM%d trigger start codec%d audio_startup not called\n", pcm_idx, i);
		}
		dev_info(chip->card->dev, "PCM%d trigger start\n", pcm_idx);
		break;

#if defined (OS_ENUM_SNDRV_PCM_TRIGGER_SUSPEND_EXIST)
	case SNDRV_PCM_TRIGGER_SUSPEND:
#endif
	case SNDRV_PCM_TRIGGER_STOP:
		chip->pcm[pcm_idx].pcm_running = 0;
		/* disable the monitor audio output because of IIS closing */
		for (i = 0; i < get_codec_count(chip); i++) {
			struct mtsnd_codec *codec = chip->pcm[pcm_idx].codec[i];

			if (codec && codec_safety_check_stop(chip, i) &&
			    codec->hcd->ops->audio_shutdown)
				codec->hcd->ops->audio_shutdown(codec->dev, codec->hcd->data);
			else
				SND_DEBUG("PCM%d trigger stop codec%d audio_shutdown not called\n", pcm_idx, i);
		}
		mtsnd_snd_stop(chip, pcm_idx);
		dev_info(chip->card->dev, "PCM%d trigger stop/suspend %d\n", pcm_idx, cmd);
		break;

	default:
		dev_info(chip->card->dev, "PCM%d trigger %d\n", pcm_idx, cmd);
		err = -EINVAL;
		break;
	}

	return err;
}

static snd_pcm_uframes_t mtsnd_pcm_pointer(struct snd_pcm_substream *substream)
{
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	u32 bytes = mtsnd_snd_pointer(chip, pcm_idx);

	if (bytes >= runtime->dma_bytes)
		bytes = 0;

	return bytes_to_frames(runtime, bytes);
}

static int mtsnd_pcm_ack(struct snd_pcm_substream *substream)
{
	u32 pcm_idx = snd_pcm_substream_get_index(substream);
	struct mtsnd_chip *chip = snd_pcm_substream_get_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	u32 appl_ofs = runtime->control->appl_ptr % runtime->buffer_size;
	u32 bytes = frames_to_bytes(runtime, appl_ofs);

	mtsnd_snd_ack(chip, runtime->control->appl_ptr, runtime->buffer_size,
		      runtime->dma_bytes, bytes, pcm_idx);

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
	int i = 0, err = 0;

	for (i = 0; i < get_pcm_count(chip); i++) {
		err = snd_pcm_new(chip->card, "MooreThreads", i, 1, 0, &pcm);
		if (err < 0) {
			dev_err(chip->card->dev, "Error snd_pcm_new\n");
			return err;
		}

		strcpy(pcm->name, "Display");
		pcm->private_data = &chip->pcm[i];

		snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &mtsnd_pcm_ops);
		/* buffer pre-allocation */
		dma_set_mask_and_coherent(&chip->pci->dev, DMA_BIT_MASK(mtsnd_query_dma_mask(chip)));
		snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV,
							&chip->pci->dev,
							MT_HW_BUFBYTE_SIZE, MT_HW_BUFBYTE_MAX);
	}

	for (i = 0; i < get_codec_count(chip); i++) {
		chip->codec[i].params = kzalloc(sizeof(*chip->codec[i].params), GFP_KERNEL);
		if (!chip->codec[i].params)
			return -ENOMEM;

		chip->codec[i].daifmt = kzalloc(sizeof(*chip->codec[i].daifmt), GFP_KERNEL);
		if (!chip->codec[i].daifmt)
			return -ENOMEM;
	}

	return 0;
}

#define I2S_INT_STAT_EMPTY 1
#define I2S_INT_STAT_COUNT 2

void mtsnd_handle_pcm(struct mtsnd_pcm *pcm)
{
	u32 pcm_idx;
	struct mtsnd_chip *chip;
	u32 handle;

	if (!pcm)
		pr_err("mtsnd irq pcm NULL");

	pcm_idx = pcm->index;
	chip = (struct mtsnd_chip *)pcm->private_data;

	if (!chip)
		pr_err("mtsnd irq chip NULL, pcm:%llx", (u64)pcm);

	handle = mtsnd_snd_irq_handle(chip, pcm_idx);
	if (!chip->pcm[pcm_idx].pcm_running)
		return;

	if (handle & I2S_INT_STAT_EMPTY) {
		snd_pcm_stop_xrun(chip->pcm[pcm_idx].substream);
		pr_warn_ratelimited("mtsnd pcm, xrun\n");
	} else if (handle & I2S_INT_STAT_COUNT)
		snd_pcm_period_elapsed(chip->pcm[pcm_idx].substream);
}

void mtsnd_free_pcm(struct mtsnd_chip *chip)
{
	int i = 0;
#ifdef INTERRUPT_DEBUG
	for (i = 0; i < get_pcm_count(chip); i++) {
		mtsnd_pcm_irq_disable(chip, i);
	}
#endif
	for (i = 0; i < get_codec_count(chip); i++) {
		kfree(chip->codec[i].params);
		kfree(chip->codec[i].daifmt);
	}
}

void mtsnd_reset_pcm(struct mtsnd_chip *chip)
{
}

#ifdef CONFIG_PM_SLEEP
void mtsnd_suspend_pcm(struct mtsnd_chip *chip)
{
	int i = 0;
	dev_info(chip->card->dev, "PCM suspend\n");

	cancel_all_pnp_event(chip);

	for (i = 0; i < get_pcm_count(chip); i++) {
		if (!chip->pcm[i].substream || !chip->pcm[i].substream->runtime) {
			SND_DEBUG("no substream or not running, just skip\n");
			continue;
		}
#ifdef INTERRUPT_DEBUG
		mtsnd_pcm_irq_disable(chip, i);
#endif
		if (chip->pcm[i].substream->runtime->status->suspended_state == SNDRV_PCM_STATE_RUNNING)
			mtsnd_pcm_trigger(chip->pcm[i].substream, SNDRV_PCM_TRIGGER_STOP);

		mtsnd_do_pcm_suspend(chip, i);
	}
}

void mtsnd_resume_pcm(struct mtsnd_chip *chip)
{
	int i = 0;
	dev_info(chip->card->dev, "PCM resume\n");

	for (i = 0; i < get_pcm_count(chip); i++) {
		/* The card will lose power during S3, so clear the SW status */
		chip->pcm[i].iis_clock = 0;
		if (!chip->pcm[i].substream || !chip->pcm[i].substream->runtime) {
			SND_DEBUG("no substream or not running, just skip\n");
			continue;
		}

		mtsnd_do_pcm_resume(chip, chip->pcm[i].substream->runtime->dma_addr, i);
#ifdef INTERRUPT_DEBUG
		mtsnd_pcm_irq_enable(chip, i);
#endif		
	}
}
#endif
