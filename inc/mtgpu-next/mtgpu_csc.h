/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_CSC_H_
#define _MTGPU_CSC_H_

struct mtgpu_vm_context;

int mtgpu_csc_table_init(PVRSRV_DEVICE_NODE *dev_node);
void mtgpu_csc_table_deinit(PVRSRV_DEVICE_NODE *dev_node);
int mtgpu_csc_table_vm_map(PVRSRV_DEVICE_NODE *dev_node,
			   struct mtgpu_vm_context *vm_ctx);
void mtgpu_csc_table_vm_unmap(struct mtgpu_vm_context *vm_ctx);

#endif /* _MTGPU_CSC_H_ */
