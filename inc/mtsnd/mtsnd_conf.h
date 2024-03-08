// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_CONF_H
#define _MTSND_CONF_H

#define MTSND_DEVICE_TYPE_MASK    (0xFF00)
#define MTSND_DEVICE_TYPE_SUDI    (0x0100)
#define MTSND_DEVICE_TYPE_QUYUAN1 (0x0200)
#define MTSND_DEVICE_TYPE_QUYUAN2 (0x0300)
#define MTSND_DEVICE_TYPE_PH1     (0x0400)

enum {
	CHIP_GEN1 = 1,
	CHIP_GEN2,
	CHIP_GEN3,
	CHIP_GEN4,
};

struct device;
struct mtsnd_chip;
struct mtsnd_codec;

typedef void (codec_ops_cb)(struct device *, bool);

struct pcm_info {
	u32 rate;
	u32 bit_depth;
	u32 channels;
	u32 period_bytes;
	u32 dma_bytes;
	bool big_endian;
};

int snd_init_conf(int devId, struct mtsnd_chip *chip);
void bind_pcm_codec(struct mtsnd_chip *chip);

u32 get_pcm_count(struct mtsnd_chip *chip);
u32 get_codec_count(struct mtsnd_chip *chip);
u32 get_codec_pcm_index(struct mtsnd_chip *chip, int codec_idx);
u32 get_chip_type(struct mtsnd_chip *chip);

const char *get_jack_name(struct mtsnd_chip *chip, u32 index);
const char *get_codec_name(struct mtsnd_chip *chip, u32 index);
const char *get_kctrl_name(struct mtsnd_chip *chip, u32 index);
codec_ops_cb *get_codec_cb(struct mtsnd_chip *chip, u32 index);
#endif /* _MTSND_CONF_H */
