#ifndef _MTVPU_SMMU_H_
#define _MTVPU_SMMU_H_

#include "linux-types.h"
#include "mtvpu_drv.h"
#include "os-interface.h"

int vpu_smmu_map(struct mt_chip *chip,
			dma_addr_t dev_addr,
			size_t size,
			dma_addr_t *iova_addr);
void vpu_smmu_unmap(struct mt_chip *chip, dma_addr_t iova_addr);
int vpu_smmu_init(struct mt_chip *chip);
void vpu_smmu_deinit(struct mt_chip *chip);

#endif
