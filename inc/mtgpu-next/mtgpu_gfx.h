/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_GFX_H__
#define __MTGPU_GFX_H__

struct mtgpu_vm_context;
struct MTFW_FWIF_TAG;

/* TODO: This size may be assigned in configuration later. */
#define PM_BUFFER_SIZE		(512 * 0x100000)
#define PM_PAGE_SIZE		(1 << 12)
#define FREELIST_PAGE_COUNT	(PM_BUFFER_SIZE / PM_PAGE_SIZE)
#define PAGE_ENTRY_VA_ALIGN	(0x20)
/* align page entry addr. */
#define PM_PAGE_NUM_ALIGN	(PAGE_ENTRY_VA_ALIGN / sizeof(u32))

#define FREELIST_STATE_COUNT \
		(MTFW_SCG_FREELIST_COUNT + \
		 MTFW_MCG2_FREELIST_COUNT + \
		 MTFW_MCG3_FREELIST_COUNT + \
		 MTFW_MCG4_FREELIST_COUNT + \
		 MTFW_MCG5_FREELIST_COUNT + \
		 MTFW_MCG6_FREELIST_COUNT + \
		 MTFW_MCG7_FREELIST_COUNT + \
		 MTFW_MCG8_FREELIST_COUNT)

#define FREELIST_STATE_TOTAL_SIZE \
		(FREELIST_STATE_COUNT * sizeof(struct mtgpu_pm_freelist_state))
#define	FREELIST_PMR_SIZE \
		(FREELIST_STATE_TOTAL_SIZE + FREELIST_PAGE_COUNT * sizeof(u32))

#define PM_BUFFER_ALLOC_FLAG (PVRSRV_MEMALLOCFLAG_GPU_WRITEABLE | \
			      PVRSRV_MEMALLOCFLAG_GPU_READABLE | \
			      PVRSRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT)

struct mtgpu_pm_freelist_state
{
	/* gpu va, required to be aligned to DWORD. */
	uint64_t freelist_page_array_base;
	uint32_t stack_ptr;
	uint32_t paddings[61];
};

void mtgpu_gfx_pb_deinit(PVRSRV_DEVICE_NODE *dev_node);
int mtgpu_gfx_pb_init(PVRSRV_DEVICE_NODE *dev_node);

void mtgpu_gfx_pb_vm_unmap(struct mtgpu_vm_context *vm_ctx);
int mtgpu_gfx_pb_vm_map(PVRSRV_DEVICE_NODE *dev_node,
			struct mtgpu_vm_context *vm_ctx);

void mtgpu_gfx_pb_fwif_init(struct MTFW_FWIF_TAG *fwif);

#endif /* __MTGPU_GFX_H__ */
