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

int mtsnd_hw_init(struct mtsnd_chip *chip, u32 pcm_idx);
void mtsnd_pcm_ata_buffer(struct mtsnd_chip *chip, u32 pcm_idx, unsigned long dma_addr, u32 size);
int mtsnd_query_dma_mask(struct mtsnd_chip *chip);
int mtsnd_clock_set(struct mtsnd_chip *chip, struct pcm_info *pcm, u32 pcm_idx);
void codec_status_init(struct mtsnd_codec *codec);
int check_hw_status(struct mtsnd_chip *chip, u32 index);
bool codec_safety_check_start(struct mtsnd_chip *chip, u32 index);
bool codec_safety_check_stop(struct mtsnd_chip *chip, u32 index);

void mtsnd_snd_start(struct mtsnd_chip *chip, u32 pcm_idx);
void mtsnd_snd_stop(struct mtsnd_chip *chip, u32 pcm_idx);
u32 mtsnd_snd_pointer(struct mtsnd_chip *chip, u32 pcm_idx);
void mtsnd_snd_ack(struct mtsnd_chip *chip, unsigned long appl_ptr, unsigned long buffer_size,
		   u32 dma_bytes, u32 bytes, u32 pcm_idx);
u32 mtsnd_snd_irq_handle(struct mtsnd_chip *chip, u32 pcm_idx);
void mtsnd_do_pcm_suspend(struct mtsnd_chip *chip, u32 pcm_idx);
void mtsnd_do_pcm_resume(struct mtsnd_chip *chip, unsigned long dma_addr, u32 pcm_idx);
void pnp_post_handle(struct mtsnd_codec *codec);
u32 check_codec_state1(struct mtsnd_codec *codec);
int codec_status_changed(struct mtsnd_codec *codec, u32 old_state);
void update_codec_state2(struct mtsnd_codec *codec, u32 value);
u32 check_codec_state2(struct mtsnd_codec *codec);
u32 check_codec_state3(struct mtsnd_codec *codec);
void update_codec_state3(struct mtsnd_codec *codec, int status);
u32 check_codec_state4(struct mtsnd_codec *codec);
void update_codec_state4(struct mtsnd_codec *codec, int status);
bool check_codec_start(struct mtsnd_chip *chip, u32 index);
#endif /* _MTSND_PCM_HW_H */
