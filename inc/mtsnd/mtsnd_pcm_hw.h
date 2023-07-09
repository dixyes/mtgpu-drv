// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_PCM_HW_H
#define _MTSND_PCM_HW_H

#ifdef USING_INBOUND
/* This is just for debug */
int mtsnd_copy_user(struct snd_pcm_substream *substream, int channel,
			unsigned long hwoff, void *buf,	unsigned long bytes);
int snd_pcm_lib_malloc_pages_inbound(struct snd_pcm_substream *substream, size_t size);
#endif

int mtsnd_hw_init(struct mtsnd_chip *chip);
unsigned long mtsnd_pcm_ata_buffer(struct mtsnd_chip *chip, unsigned long dma_addr);
int mtsnd_clock_set(struct mtsnd_chip *chip, struct pcm_info *pcm);
void codec_status_init(struct mtsnd_codec *codec);
int check_hw_status(struct mtsnd_chip *chip, int index);
bool codec_safety_check_start(struct mtsnd_chip *chip, unsigned int index);
bool codec_safety_check_stop(struct mtsnd_chip *chip, unsigned int index);

void mtsnd_snd_start(struct mtsnd_chip *chip);
void mtsnd_snd_stop(struct mtsnd_chip *chip);
unsigned int mtsnd_snd_pointer(struct mtsnd_chip *chip);
void mtsnd_snd_ack(struct mtsnd_chip *chip, unsigned long appl_ptr, unsigned long buffer_size,
		   unsigned int dma_bytes, unsigned int bytes);
unsigned int mtsnd_snd_irq_handle(struct mtsnd_chip *chip);
void mtsnd_do_pcm_suspend(struct mtsnd_chip *chip);
void mtsnd_do_pcm_resume(struct mtsnd_chip *chip, unsigned long dma_addr);
void pnp_post_handle(struct mtsnd_codec *codec);
unsigned int check_codec_state1(struct mtsnd_codec *codec);
int codec_status_changed(struct mtsnd_codec *codec, unsigned int old_state);
void update_codec_state2(struct mtsnd_codec *codec, unsigned int value);
unsigned int check_codec_state2(struct mtsnd_codec *codec);
unsigned int check_codec_state3(struct mtsnd_codec *codec);
void update_codec_state3(struct mtsnd_codec *codec, int status);
unsigned int check_codec_state4(struct mtsnd_codec *codec);
void update_codec_state4(struct mtsnd_codec *codec, int status);
bool check_codec_start(struct mtsnd_chip *chip, int index);
#endif /* _MTSND_PCM_HW_H */
