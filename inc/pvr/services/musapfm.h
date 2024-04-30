/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_PFM_H__
#define __MTGPU_PFM_H__

#include "devicemem_server.h"
#include "physmem.h"
#include "common_mm_bridge.h"
#include "devicemem_utils.h"
#include "devicemem_typedefs.h"

typedef struct _PFM_CONTEXT_ {
	DEVMEMINT_CTX *psPFMDevmemCtxInt;
	DEVMEMINT_HEAP *psPFMIntHeap;
	DEVMEMINT_MAPPING *psMappingInt;
	PMR *psPMR;
	DEVMEMINT_RESERVATION *psReservationInt;
	IMG_DEV_PHYADDR sPFMPCBaseAddr;
	IMG_UINT64 ui64GpuBufVa;
	void *psConnection;
	IMG_UINT32 ui32PFMContextID;
} PFM_CONTEXT;

typedef struct {
	IMG_UINT32 ui32MSSUseMEM;
	IMG_UINT32 ui32ContextId;
	IMG_UINT64 ui64PcBaseAddr;
	IMG_UINT64 ui64BufBaseAddr;
} PFM_MSS_MMU_CONFIG;

PVRSRV_ERROR PVRSRVMUSAPFMInit(CONNECTION_DATA *psConnection,
			       RGX_HWPERF_PFM_INIT_PARAM *psInitParam, IMG_HANDLE *hPMR);
PVRSRV_ERROR PVRSRVMUSAPFMSetGlobalConfig(CONNECTION_DATA *psConnection,
					  RGX_HWPERF_PFM_GLOBAL_CONFIG *psGlobalConfig);
PVRSRV_ERROR PVRSRVMUSAPFMSetInstanceConfig(PVRSRV_DEVICE_NODE *psDeviceNode,
					    RGX_HWPERF_PFM_INSTANCE_CONFIG *psInstanceConfig);
PVRSRV_ERROR PVRSRVMUSAPFMSetWrapperConfig(PVRSRV_DEVICE_NODE *psDeviceNode,
					   const RGX_HWPERF_PFM_WRAPPER_CONFIG *psWraperConfig);
PVRSRV_ERROR PVRSRVMUSAPFMSetMSSConfig(CONNECTION_DATA *psConnection,
				       const RGX_HWPERF_PFM_MSS_CONFIG *psMSSCfg);
PVRSRV_ERROR PVRSRVMUSAPFMDumpTrigger(PVRSRV_DEVICE_NODE *psDeviceNode,
				      RGX_HWPERF_PFM_DUMP_TRIGGER_CONFIG *psDumpTrigger);
PVRSRV_ERROR PVRSRVMUSAPFMDeinit(CONNECTION_DATA *psConnection, IMG_HANDLE hPMR);
#endif
