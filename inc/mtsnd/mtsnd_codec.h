// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_CODEC_H
#define _MTSND_CODEC_H

#include <linux/timer.h>

#include "mtsnd_drv.h"

struct mtsnd_timer {
	struct timer_list timer;
	struct mtsnd_chip *chip;
	int codec_index;
	int jack_status;
};

void cancel_all_pnp_event(struct mtsnd_chip *chip);

#endif /* _MTSND_CODEC_H */
