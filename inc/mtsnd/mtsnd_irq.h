// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_IRQ_H
#define _MTSND_IRQ_H

// #define INTERRUPT_DEBUG

#ifdef INTERRUPT_DEBUG
bool check_pcm_irq(struct mtsnd_chip *chip, u32 pcm_idx, u32 src);
bool check_compr_irq(struct mtsnd_chip *chip, u32 src);
u32 get_pcm_compr_irq(struct mtsnd_chip *chip);
void clear_pcm_compr_irq(struct mtsnd_chip *chip, u32 src);
void mtsnd_pcm_irq_enable(struct mtsnd_chip *chip, u32 pcm_idx);
void mtsnd_pcm_irq_disable(struct mtsnd_chip *chip, u32 pcm_idx);
#endif
#endif /* _MTSND_IRQ_H */
