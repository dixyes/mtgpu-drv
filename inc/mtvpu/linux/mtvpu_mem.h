/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_MEM_H_
#define _MTVPU_MEM_H_

#include "mtvpu_drv.h"
#ifndef SOC_MODE
#include "mtgpu_segment.h"
#endif

void vpu_fix_core_40bit(struct mt_chip *chip, int idx);
int vpu_init_mem(struct mt_chip *chip, int idx);
int vpu_init_guest_info(struct mt_chip *chip);
void vpu_free_mem(struct mt_chip *chip);

#endif /* _MTVPU_MEM_H_ */
