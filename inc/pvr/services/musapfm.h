/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_PFM_H__
#define __MTGPU_PFM_H__

#include "devicemem_server.h"
#include "physmem.h"
#include "common_mm_bridge.h"

#define PFM_INSTANCE_POINTER_BUFER_SIZE 32
#define PFM_SPACE_SIZE			4096

typedef struct _PFM_MEM_MGR_
{
	IMG_UINT64 	ui64GpuCounterBufVa;
	IMG_UINT64 	ui64GpuPointerBufVa;
	IMG_UINT8  	bIsUsed;
} PFM_MEM_MGR;

typedef struct _PFM_CONTEXT_
{
	DEVMEMINT_CTX           *psPFMMMUContext;  // The MMU context for PFM
	DEVMEMINT_HEAP          *psPFMIntHeap;
	DEVMEMINT_MAPPING       *psMappingInt;
	PMR                     *psPMR;
	IMG_HANDLE              hPMR;
	DEVMEMINT_RESERVATION   *psReservationInt;
	IMG_HANDLE              hPrivData;
	IMG_DEV_PHYADDR         sPFMPCBaseAddr;
	PFM_MEM_MGR             *psMemMgr;
	POS_SPINLOCK            hSpinLockPFMMem;
	void*                   psConnection;
	RGX_HWPERF_PFM_GLOBAL_CONFIG sGlobalConfig;
} PFM_CONTEXT;

/**
 * PVRSRVMUSAPFMSetGlobalConfig - Initialize and configure PFM GlobalConfig. If it is the first execution, 
 *                      it will create the corresponding PFM CONTEXT.
 * @global_config: Describe global information.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMSetGlobalConfig(CONNECTION_DATA *psConnection, RGX_HWPERF_PFM_GLOBAL_CONFIG *psGlobalConfig, IMG_HANDLE *hPMR);

/**
 * PVRSRVMUSAPFMSetInstanceConfig - Pass the PFM instance information to FW settings
 *
 * @instance_config: Describe instance information.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMSetInstanceConfig(PVRSRV_DEVICE_NODE *psDeviceNode, RGX_HWPERF_PFM_INSTANCE_CONFIG *psInstanceConfig);

/**
 * PVRSRVMUSAPFMSetWrapperConfig - Pass the PFM wrapper information to FW settings
 *
 * @wrapper_config: Describe wrapper information.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMSetWrapperConfig(PVRSRV_DEVICE_NODE *psDeviceNode, const RGX_HWPERF_PFM_WRAPPER_CONFIG *psWraperConfig);

/**
 * PVRSRVMUSAPFMDumpOnce - Trigger PFM dump once in normal mode.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMDumpOnce(PVRSRV_DEVICE_NODE *psDeviceNode);

/**
 * PVRSRVMUSAPFMStart - Enable PFM functionality.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMStart(PVRSRV_DEVICE_NODE *psDeviceNode);

/**
 * PVRSRVMUSAPFMStop - Disable PFM functionality.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMStop(PVRSRV_DEVICE_NODE *psDeviceNode);

/**
 * PVRSRVMUSAPFMClearConfig - Disable PFM functionality.
 *
 * Returning PVRSRV_OK indicates successful initialization.
 */
PVRSRV_ERROR PVRSRVMUSAPFMClearConfig(CONNECTION_DATA *psConnection);
#endif
