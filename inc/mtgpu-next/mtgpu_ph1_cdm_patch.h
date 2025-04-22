/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_PH1_CDM_PATCH_H__
#define __MTGPU_PH1_CDM_PATCH_H__

int mtgpu_ph1_cdm_patch_init(PVRSRV_DEVICE_NODE *dev_node);
void mtgpu_ph1_cdm_patch_deinit(PVRSRV_DEVICE_NODE *dev_node);
int mtgpu_ph1_cdm_patch_vm_map(PVRSRV_DEVICE_NODE *dev_node);
void mtgpu_ph1_cdm_patch_vm_unmap(PVRSRV_DEVICE_NODE *dev_node);

#endif /* __MTGPU_PH1_CDM_PATCH_H__ */

