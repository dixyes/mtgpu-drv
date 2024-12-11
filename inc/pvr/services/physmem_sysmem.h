/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _PHYSMEM_SYSMEM_H_
#define _PHYSMEM_SYSMEM_H_

#include "img_types.h"
#include "pvrsrv_memallocflags.h"

struct page;

#ifdef CONFIG_ARM64
void PhysmemSetCpuPagesMapAttrCached(struct page **ppsPageArray,
				     IMG_UINT32 ui32PageNum,
				     IMG_UINT32 uiLog2PageSize,
				     IMG_UINT32 *pui32MapTable);

void PhysmemSetCpuPagesMapAttrUncached(struct page **ppsPageArray,
				       IMG_UINT32 ui32PageNum,
				       IMG_UINT32 uiLog2PageSize,
				       IMG_UINT32 *pui32MapTable);
#endif

PVRSRV_ERROR
PhysmemCreateSystemMemPMR(PHYS_HEAP *psPhysHeap,
			  CONNECTION_DATA *psConnection,
			  IMG_DEVMEM_SIZE_T uiSize,
			  IMG_DEVMEM_SIZE_T uiChunkSize,
			  IMG_UINT32 ui32NumPhysChunks,
			  IMG_UINT32 ui32NumVirtChunks,
			  IMG_UINT32 *pui32MappingTable,
			  IMG_UINT32 uiLog2AllocPageSize,
			  PVRSRV_MEMALLOCFLAGS_T uiFlags,
			  const IMG_CHAR *pszAnnotation,
			  IMG_PID uiPid,
			  PMR **ppsPMRPtr,
			  IMG_UINT32 ui32PDumpFlags);

#endif
