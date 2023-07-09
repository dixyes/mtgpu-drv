/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_MEM_H_
#define _MTVPU_MEM_H_

#include "mtvpu_drv.h"
#include "mtgpu_segment.h"

void vpu_fix_core_40bit(struct mt_chip *chip, int idx);
int vpu_init_mem(struct mt_chip *chip, int idx);
int vpu_init_guest_mem(struct mt_chip *chip);
void vpu_free_mem(struct mt_chip *chip);
int vpu_init_segment(struct mt_chip *chip);
int vpu_assign_segment_to_core(struct mt_chip *chip);
int vpu_assign_heap_to_core(struct mt_chip *chip);

#endif /* _MTVPU_MEM_H_ */
