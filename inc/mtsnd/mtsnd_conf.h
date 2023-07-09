// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_CONF_H
#define _MTSND_CONF_H

#define MAX_NUM_CARD		8

enum {
	CHIP_GEN1,
	CHIP_GEN2,
	CHIP_GEN3,
	CHIP_GEN4,
};

struct device;
struct mtsnd_chip;
struct mtsnd_codec;

typedef void (codec_ops_cb)(struct device *, bool);

struct pcm_info {
	unsigned int rate;
	unsigned int bit_depth;
	unsigned int channels;
	unsigned int period_bytes;
	unsigned int dma_bytes;
	bool big_endian;
};

int snd_init_conf(int devId, struct mtsnd_chip *chip);

unsigned int get_codec_count(struct mtsnd_chip *chip);
unsigned int get_chip_type(struct mtsnd_chip *chip);

const char *get_jack_name(struct mtsnd_chip *chip, unsigned int index);
const char *get_codec_name(struct mtsnd_chip *chip, unsigned int index);
const char *get_kctrl_name(struct mtsnd_chip *chip, unsigned int index);
codec_ops_cb *get_codec_cb(struct mtsnd_chip *chip, unsigned int index);
#endif /* _MTSND_CONF_H */
