/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_POOL_H_
#define _MTVPU_POOL_H_

#include "mtvpu_drv.h"

int vpu_mem_pool_alloc(struct mt_chip *chip, struct mtvpu_mmu_ctx *mmu_ctx, u32 pool_id, size_t size, dma_addr_t *addr);
int vpu_mem_pool_free(struct mt_chip *chip, u32 pool_id);

int vpu_create_mem_pool(struct mt_chip *chip, struct drm_device *drm, struct mtvpu_mmu_ctx *mmu_ctx,
			u32 group_id, size_t size, u32 *pool_id);
int vpu_destroy_mem_pool(struct mt_chip *chip, u32 pool_id);
int vpu_reset_mem_pool(struct mt_chip *chip, u32 pool_id);

#endif /* _MTVPU_POOL_H_ */
