/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTVPU_CLK_H__
#define __MTVPU_CLK_H__

s64 mtvpu_get_clk(struct mt_chip *chip, int idx);
s64 mtvpu_get_max_clk(struct mt_chip *chip, int idx);

#endif  /* __MTVPU_CLK_H__ */
