/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MISC_H_
#define _MISC_H_

void vpu_set_vm_core(struct mt_chip *chip);
void vpu_slice_mode_config(struct mt_chip *chip, int core_idx);
int vpu_fixed_mem_qy2(struct mt_chip *chip, int type);

#endif /*_MISC_H_ */
