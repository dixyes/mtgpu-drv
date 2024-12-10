/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _PHYSMEM_USERMEM_H
#define _PHYSMEM_USERMEM_H

#include "img_types.h"
#include "pvrsrv_error.h"
#include "pvrsrv_memallocflags.h"

PVRSRV_ERROR
PhysmemCreateUserMemPMR(PHYS_HEAP *psPhysHeap,
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

#endif /* _PHYSMEM_USERMEM_H */
