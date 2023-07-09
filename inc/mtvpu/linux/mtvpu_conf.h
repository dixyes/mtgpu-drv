/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_CONF_H_
#define _MTVPU_CONF_H_

#define CORE_MAX_SIZE 12
#define INST_MAX_SIZE 32
#define MEM_GROUP_MAX_SIZE 8
#define MEM_POOL_MAX_SIZE 256
#define VPU_REG_CORE_SIZE 0x4000

#define MTVPU_DEVICE_TYPE_MASK    (0x0F00)
#define MTVPU_DEVICE_TYPE_SUDI    (0x0100)
#define MTVPU_DEVICE_TYPE_QUYUAN1 (0x0200)
#define MTVPU_DEVICE_TYPE_QUYUAN2 (0x0300)

struct mt_chip;

enum {
	TYPE_SUDI,
	TYPE_QUYU1,
	TYPE_QUYU2,
	TYPE_QUYU3,
};

/*
  ------------
  boda950
  ------------
  wave517 x N
  ------------
  wave627 x N
  ------------
  top
  ------------
  ata
  ------------
*/

struct mt_conf {
	u32 type;

	u32 regs_base;
	u32 regs_chip_size;
	u32 regs_core_size;

	u32 regs_top_offset;
	u32 regs_ata_offset;

	u32 core_base;
	u32 core_size;

	u32 product[CORE_MAX_SIZE];

	u32 core_group[MEM_GROUP_MAX_SIZE][CORE_MAX_SIZE];
};

int vpu_init_conf(int devId, struct mt_chip *chip, bool guest);
int vpu_chip_add(struct mt_chip *chip);
int vpu_chip_del(struct mt_chip *chip);

struct mt_chip *get_chip(int coreIdx);
struct mt_core *get_core(int coreIdx);
u32 get_product(int coreIdx);

#endif
