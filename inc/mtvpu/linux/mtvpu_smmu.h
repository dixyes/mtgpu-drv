#ifndef _MTVPU_SMMU_H_
#define _MTVPU_SMMU_H_

#include "linux-types.h"
#include "mtvpu_drv.h"
#include "os-interface.h"

int vpu_smmu_map_sg(struct mt_chip *chip,
			struct sg_table *sgt,
			size_t size,
			u32 group_id,
			dma_addr_t *iova_addr);
void vpu_smmu_unmap(struct mt_chip *chip, dma_addr_t iova_addr);
int vpu_smmu_init(struct mt_chip *chip);
void vpu_smmu_deinit(struct mt_chip *chip);

#endif
