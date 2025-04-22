/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_GFX_H__
#define __MTGPU_GFX_H__

struct mtgpu_vm_context;
struct MTFW_FWIF_TAG;
struct _PVRSRV_RGXDEV_INFO_;

/* For sudi, auto set pb size 256M. */
#define MTGPU_SUDI_PB_DEFAULT_SIZE	0x10000000
/* For other gpu, auto set pb size 512M. */
#define MTGPU_PB_DEFAULT_SIZE		0x20000000

/* PM free list use 4K page size for SUDI and QUYUAN */
#define PM_PAGE_SIZE		(1 << 12)
#define PM_PAGE_SHIFT		12
/* PM free list use 64K page size for PH1 and later */
#define CBUF_PAGE_SIZE		(1 << 16)
#define CBUF_PAGE_SHIFT		16

#define PAGE_ENTRY_VA_ALIGN	(0x20)
/* align page entry addr. */
#define PM_PAGE_NUM_ALIGN	(PAGE_ENTRY_VA_ALIGN / sizeof(u32))

#define PB_DESC_COUNT \
		(MTFW_SCG_FREELIST_COUNT + \
		 MTFW_MCG2_FREELIST_COUNT + \
		 MTFW_MCG3_FREELIST_COUNT + \
		 MTFW_MCG4_FREELIST_COUNT + \
		 MTFW_MCG5_FREELIST_COUNT + \
		 MTFW_MCG6_FREELIST_COUNT + \
		 MTFW_MCG7_FREELIST_COUNT + \
		 MTFW_MCG8_FREELIST_COUNT)

#define PB_DESC_TOTAL_SIZE	(PB_DESC_COUNT * sizeof(union mtgpu_pb_desc))

#define PM_BUFFER_ALLOC_FLAG (PVRSRV_MEMALLOCFLAG_GPU_WRITEABLE | \
			      PVRSRV_MEMALLOCFLAG_GPU_READABLE | \
			      PVRSRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT)

union mtgpu_pb_desc {
	/* old freelist state, used for pm. */
	struct mtgpu_freelist_state {
		/* gpu va, required to be aligned to DWORD. */
		uint64_t freelist_page_array_base;
		uint32_t stack_ptr;
	} freelist_state;

	/* cbuf descriptor, for ph1 or later pmva. */
	struct mtgpu_cbuf_desc {
		uint64_t start_addr  : 43; // sizeof(CBUF_DESC) (256 bits) aligned
		uint64_t reserve0    : 21;
		uint64_t size_in_64k : 19;
		uint64_t reserve1    : 13;
		uint64_t alloc_ptr   : 18;
		uint64_t reserve2    : 14;
		uint64_t dealloc_ptr : 18;
		uint64_t reserve3    : 14;
		uint64_t full        : 1; // Set if all VA pages in C buffer are valid (initial state)
		uint64_t empty       : 1; // Set if no VA page in C buffer is valid
		uint64_t reserve4    : 30;
		uint64_t close_ptr   : 18;
		uint64_t reserve5    : 46;
	} cbuf_desc;

	/* used for force align. */
	uint32_t value[64];
};

void mtgpu_gfx_pb_deinit(PVRSRV_DEVICE_NODE *dev_node);
int mtgpu_gfx_pb_init(PVRSRV_DEVICE_NODE *dev_node);

void mtgpu_gfx_pb_vm_unmap(struct mtgpu_vm_context *vm_ctx);
int mtgpu_gfx_pb_vm_map(PVRSRV_DEVICE_NODE *dev_node,
			struct mtgpu_vm_context *vm_ctx);

void mtgpu_gfx_pb_fwif_init(struct MTFW_FWIF_TAG *fwif);

int mtgpu_freelist_recovery(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
#endif /* __MTGPU_GFX_H__ */
