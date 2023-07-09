/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_POOL_H_
#define _MTVPU_POOL_H_

#include "mtvpu_drv.h"

#define VPU_IS_POOL_ID_VALID(pool_id)	(pool_id > 0 && pool_id <= MEM_POOL_MAX_SIZE)
#define VDI_POOL_ID_VALID(pool_id)	\
	(pool_id > 0xFF000000 && (pool_id - 0xFF000000) <= MEM_POOL_MAX_SIZE)

int vpu_alloc_mem_pool(struct mt_chip *chip, u32 pool_id, size_t size, dma_addr_t *addr);
int vpu_reset_mem_pool(struct mt_chip *chip, u32 pool_id);

int vpu_create_mem_pool(struct mt_chip *chip, struct drm_device *drm, u32 group_id, size_t size,
			u32 *pool_id);
int vpu_destroy_mem_pool(struct mt_chip *chip, u32 pool_id);
int vpu_reset_mem_pool(struct mt_chip *chip, u32 pool_id);
/* call this function only when uninstalling the driver */
void vpu_destroy_all_pools(struct mt_chip *chip);
#endif /* _MTVPU_POOL_H_ */
