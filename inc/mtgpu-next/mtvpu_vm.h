/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTVPU_VM_H_
#define _MTVPU_VM_H_

struct _PVRSRV_DEVICE_NODE_;
struct mtvpu_vm_context;
struct drm_device;

int mtvpu_vm_context_create(struct _PVRSRV_DEVICE_NODE_ *dev_node,
			    struct mtvpu_vm_context **vm_ctx_out,
			    u64 *pc_base_addr,
			    u64 *page_size);
int mtvpu_vm_context_destroy(struct mtvpu_vm_context *vm_ctx);
int mtvpu_vm_pmr_create_and_map(struct _PVRSRV_DEVICE_NODE_ *dev_node,
				struct mtvpu_vm_context *vm_ctx,
				u64 size,
				u64 flags,
				void **pmr_out,
				u64 *dev_vaddr,
				u64 *dev_paddr,
				void **cpu_vaddr_out);
int mtvpu_vm_pmr_unmap_and_destroy(struct mtvpu_vm_context *vm_ctx,
				   void *pmr_ptr,
				   u64 dev_vaddr,
				   void *cpu_vaddr);
int mtvpu_get_fw_ctx_pc_root(struct drm_device *drm, u64 *fw_ctx_pc_root);

#endif /* _MTVPU_VM_H_*/
