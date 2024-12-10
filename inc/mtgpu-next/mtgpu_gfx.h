/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_GFX_H__
#define __MTGPU_GFX_H__

struct mtgpu_vm_context;
struct MTFW_FWIF_TAG;

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
