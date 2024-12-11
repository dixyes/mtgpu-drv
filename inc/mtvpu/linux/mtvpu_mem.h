/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_MEM_H_
#define _MTVPU_MEM_H_

#include "mtvpu_drv.h"
#include "mtgpu_segment.h"
#include "mtvpu_vm.h"

#define BASIC_VCPU_SIZE	        (0 << 10)	/* not reserve for vcpu buffer */

/* Limit 163M for at least 1 4K dec(H264, max=124M) + 1 1080P dec(H264, max=37M) + enc */
#define LIMITED_VCPU_SIZE       (163 << 20)
#define SAFE_VCPU_RESERVE       (80 << 20)      /* reserve 80M for keepping safe */

struct DEVMEM_CONTEXT_TAG;
struct DEVMEM_HEAP_TAG;
struct DEVMEM_MEMDESC_TAG;
typedef struct DEVMEM_CONTEXT_TAG DEVMEM_CONTEXT;
typedef struct DEVMEM_HEAP_TAG DEVMEM_HEAP;
typedef struct DEVMEM_MEMDESC_TAG DEVMEM_MEMDESC;

/* MMU context index range 0~31, and use 0xFF to indicate decided by firmware */
#define MTVPU_MMU_CTX_IDX_DECIDED_BY_FW	0xFF
/* MMU page size mask */
#define MTGPU_MMU_PAGE_SIZE_MASK_SHIFT	(48)
#define MTGPU_MMU_PAGE_SIZE_MASK	(0xFFLL << MTGPU_MMU_PAGE_SIZE_MASK_SHIFT)
#define MTVPU_MMU_PAGE_SIZE_MASK_SHIFT	(16)

#define SUPPORT_PH_M3D
struct mtvpu_mmu_ctx
{
	u32 mmu_enable;			/* mmu enable flag, set to 1 to enable for PH */
	DEVMEM_CONTEXT *devmem_ctx;	/* the devmem context */
	DEVMEM_HEAP *devmem_heap;	/* the vpu heap */
	struct mtvpu_vm_context *vpu_ctx;
	u64 root_page_table_addr;	/* the mmu root page table physical address */
	u64 page_size;			/* the mmu page size */
	u32 page_size_mask;		/* the mmu page size mask */
	u32 fw_mmu_ctx_id;		/* the mmu context id used in vpu firmware */
};

void vpu_fix_core_40bit(struct mt_chip *chip, int idx);
int vpu_alloc_fw_mem(struct mt_chip *chip, int idx);
void vpu_free_fw_mem(struct mt_chip *chip);
int vpu_init_guest_mem(struct mt_chip *chip);
void vm_update_vcpu_resource(struct mt_chip *chip, struct mt_virm *vm, int size, bool is_alloc);

int vpu_mmu_context_create(struct drm_device *drm, struct mtvpu_mmu_ctx **mmu_ctx);
int vpu_mmu_context_destroy(struct mtvpu_mmu_ctx *mmu_ctx);
void vpu_rpt_info_init(struct mtvpu_mmu_ctx *mmu_ctx, RootPageTableInfo *rptInfo);

#endif /* _MTVPU_MEM_H_ */
