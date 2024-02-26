/*************************************************************************/ /*!
@File
@Title          Functions for creating and reading proc filesystem entries.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#ifndef PROCESS_STATS_H
#define PROCESS_STATS_H

#include <powervr/mem_types.h>

#include "pvrsrv_error.h"
#include "allocmem.h"
#include "cache_ops.h"
#include "device.h"
#include "osdi_impl.h"

/*
 * The publishing of Process Stats is controlled by the
 * PVRSRV_ENABLE_PROCESS_STATS build option. The recording of all Memory
 * allocations is controlled by the PVRSRV_ENABLE_MEMORY_STATS build option.
 *
 * Note: There will be a performance degradation with memory allocation
 *       recording enabled!
 */


/*
 * Memory types which can be tracked...
 */
typedef enum {
	PVRSRV_MEM_ALLOC_TYPE_KMALLOC,				/* memory allocated by kmalloc() */
	PVRSRV_MEM_ALLOC_TYPE_VMALLOC,				/* memory allocated by vmalloc() */
	PVRSRV_MEM_ALLOC_TYPE_ALLOC_PAGES_PT_UMA,	/* pages allocated from UMA to hold page table information */
	PVRSRV_MEM_ALLOC_TYPE_VMAP_PT_UMA,			/* ALLOC_PAGES_PT_UMA mapped to kernel address space */
	PVRSRV_MEM_ALLOC_TYPE_ALLOC_PAGES_PT_LMA,	/* pages allocated from LMA to hold page table information */
	PVRSRV_MEM_ALLOC_TYPE_IOREMAP_PT_LMA,		/* ALLOC_PAGES_PT_LMA mapped to kernel address space */
	PVRSRV_MEM_ALLOC_TYPE_ALLOC_LMA_PAGES,		/* pages allocated from LMA */
	PVRSRV_MEM_ALLOC_TYPE_ALLOC_UMA_PAGES,		/* pages allocated from UMA */
	PVRSRV_MEM_ALLOC_TYPE_MAP_UMA_LMA_PAGES,	/* mapped UMA/LMA pages */
	PVRSRV_MEM_ALLOC_TYPE_UMA_POOL_PAGES,		/* pages in the page pool */
	PVRSRV_MEM_ALLOC_TYPE_DMA_BUF_IMPORT,       /* dma-buf imports */
	PVRSRV_MEM_ALLOC_TYPE_ALLOC_SYSTEM_PAGES,   /* pages allocated from sysm memory pages */
	PVRSRV_MEM_ALLOC_TYPE_ALLOC_USER_PAGES, /* pages allocated from user memory pages */
	/* Must be the last enum...*/
	PVRSRV_MEM_ALLOC_TYPE_COUNT
} PVRSRV_MEM_ALLOC_TYPE;


/*
 * Functions for managing the processes recorded...
 */
PVRSRV_ERROR PVRSRVStatsInitialise(void);
void PVRSRVStatsDestroy(void);

PVRSRV_ERROR PVRSRVStatsRegisterProcess(IMG_HANDLE *phProcessStats, PVRSRV_DEVICE_NODE *psDevNode);

void PVRSRVStatsDeregisterProcess(IMG_HANDLE hProcessStats);

#define MAX_POWER_STAT_ENTRIES		51

/*
 * Functions for recording the statistics...
 */

void PVRSRVStatsAddMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE eAllocType,
				  void *pvCpuVAddr,
				  IMG_CPU_PHYADDR sCpuPAddr,
				  size_t uiBytes,
				  void *pvPrivateData,
				  IMG_PID uiPid,
				  PVRSRV_DEVICE_NODE *psDeivceNode
				  DEBUG_MEMSTATS_PARAMS);

void PVRSRVStatsRemoveMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE eAllocType,
				     IMG_UINT64 ui64Key,
				     IMG_PID uiPid,
				     PVRSRV_DEVICE_NODE *psDeivceNode);

void PVRSRVStatsIncrMemAllocStat(PVRSRV_MEM_ALLOC_TYPE eAllocType,
				 size_t uiBytes,
				 IMG_PID uiPid,
				 PVRSRV_DEVICE_NODE *psDeivceNode);

/*
 * Increases the memory stat for eAllocType. Tracks the allocation size value
 * by inserting a value into a hash table with uiCpuVAddr as key.
 * Pair with PVRSRVStatsDecrMemAllocStatAndUntrack().
 */
void PVRSRVStatsIncrMemAllocStatAndTrack(PVRSRV_MEM_ALLOC_TYPE eAllocType,
					 size_t uiBytes,
					 IMG_UINT64 uiCpuVAddr,
					 IMG_PID uiPid,
					 PVRSRV_DEVICE_NODE *psDeivceNode);

void PVRSRVStatsDecrMemAllocStat(PVRSRV_MEM_ALLOC_TYPE eAllocType,
				 size_t uiBytes,
				 IMG_PID uiPid,
				 PVRSRV_DEVICE_NODE *psDeivceNode);

void PVRSRVStatsDecrMemKAllocStat(size_t uiBytes,
								  IMG_PID decrPID,
								  PVRSRV_DEVICE_NODE *psDeivceNode);

/*
 * Decrease the memory stat for eAllocType. Takes the allocation size value
 * from the hash table with uiCpuVAddr as key.
 * Pair with PVRSRVStatsIncrMemAllocStatAndTrack().
 */
void PVRSRVStatsDecrMemAllocStatAndUntrack(PVRSRV_MEM_ALLOC_TYPE eAllocType,
					   IMG_UINT64 uiCpuVAddr,
					   PVRSRV_DEVICE_NODE *psDeivceNode);

void
PVRSRVStatsIncrMemAllocPoolStat(size_t uiBytes);

void
PVRSRVStatsDecrMemAllocPoolStat(size_t uiBytes);

void
PVRSRVStatsUpdateOOMStats(IMG_UINT32 ui32OOMStatType,
			  IMG_PID pidOwner,
			  PVRSRV_DEVICE_NODE *psDeviceNode);

PVRSRV_ERROR
PVRSRVServerUpdateOOMStats(IMG_UINT32 ui32OOMStatType,
			   IMG_PID pidOwner,
			   PVRSRV_DEVICE_NODE *psDeviceNode);

void PVRSRVStatsUpdateRenderContextStats(IMG_UINT32 ui32TotalNumPartialRenders,
										 IMG_UINT32 ui32TotalNumOutOfMemory,
										 IMG_UINT32 ui32TotalTAStores,
										 IMG_UINT32 ui32Total3DStores,
										 IMG_UINT32 ui32TotalCDMStores,
										 IMG_UINT32 ui32TotalTDMStores,
										 IMG_PID owner,
										 PVRSRV_DEVICE_NODE *psDeviceNode);

void PVRSRVStatsUpdateZSBufferStats(IMG_UINT32 ui32NumReqByApp,
									IMG_UINT32 ui32NumReqByFW,
									IMG_PID owner,
									PVRSRV_DEVICE_NODE *psDeviceNode);

void PVRSRVStatsUpdateFreelistStats(IMG_UINT32 ui32NumGrowReqByApp,
									IMG_UINT32 ui32NumGrowReqByFW,
									IMG_UINT32 ui32InitFLPages,
									IMG_UINT32 ui32NumHighPages,
									IMG_PID    ownerPid,
									PVRSRV_DEVICE_NODE *psDeviceNode);
#if defined(PVRSRV_ENABLE_CACHEOP_STATS)
void PVRSRVStatsUpdateCacheOpStats(PVRSRV_CACHE_OP uiCacheOp,
#if defined(PVRSRV_ENABLE_GPU_MEMORY_INFO) && defined(DEBUG)
								   IMG_DEV_VIRTADDR sDevVAddr,
								   IMG_DEV_PHYADDR sDevPAddr,
#endif
								   IMG_DEVMEM_SIZE_T uiOffset,
								   IMG_DEVMEM_SIZE_T uiSize,
								   IMG_UINT64 ui64ExecuteTimeMs,
								   IMG_BOOL bUserModeFlush,
								   IMG_PID ownerPid,
								   PVRSRV_DEVICE_NODE *psDeviceNode);
#endif

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
/* Update pre/post power transition timing statistics */
void InsertPowerTimeStatistic(IMG_UINT64 ui64SysStartTime, IMG_UINT64 ui64SysEndTime,
                              IMG_UINT64 ui64DevStartTime, IMG_UINT64 ui64DevEndTime,
                              IMG_BOOL bForced, IMG_BOOL bPowerOn, IMG_BOOL bPrePower);

void InsertPowerTimeStatisticExtraPre(IMG_UINT64 ui64StartTimer, IMG_UINT64 ui64Stoptimer);
void InsertPowerTimeStatisticExtraPost(IMG_UINT64 ui64StartTimer, IMG_UINT64 ui64StopTimer);
#else
/* Update pre/post power transition timing statistics */
static inline
void InsertPowerTimeStatistic(IMG_UINT64 ui64SysStartTime, IMG_UINT64 ui64SysEndTime,
                              IMG_UINT64 ui64DevStartTime, IMG_UINT64 ui64DevEndTime,
                              IMG_BOOL bForced, IMG_BOOL bPowerOn, IMG_BOOL bPrePower) {}
static inline
void InsertPowerTimeStatisticExtraPre(IMG_UINT64 ui64StartTimer, IMG_UINT64 ui64Stoptimer) {}

static inline
void InsertPowerTimeStatisticExtraPost(IMG_UINT64 ui64StartTimer, IMG_UINT64 ui64StopTimer) {}
#endif

void SetFirmwareStartTime(IMG_UINT32 ui32TimeStamp);

void SetFirmwareHandshakeIdleTime(IMG_UINT64 ui64Duration);

/* Functions used for calculating the memory usage statistics of a process */
PVRSRV_ERROR PVRSRVFindProcessMemStats(IMG_PID pid, IMG_UINT32 ui32ArrSize,
				       IMG_BOOL bAllProcessStats,
				       IMG_UINT32 *pui32MemoryStats,
				       PVRSRV_DEVICE_NODE *psDeviceNode);

typedef struct {
	IMG_UINT32 ui32Pid;
	IMG_UINT32 ui32KernelMemUsage;
	IMG_UINT32 ui32GraphicsMemUsage;
} PVRSRV_PER_PROCESS_MEM_USAGE;

PVRSRV_ERROR PVRSRVGetProcessMemUsage(IMG_UINT32 *pui32TotalMem,
									  IMG_UINT32 *pui32NumberOfLivePids,
									  PVRSRV_PER_PROCESS_MEM_USAGE **ppsPerProcessMemUsageData);

IMG_UINT64 PVRSRVGetGlobalMemLMAAllocSize(void);

int GpuMemStatDumpShow(PVRSRV_DEVICE_NODE *psDeviceNode, OSDI_IMPL_ENTRY *psEntry);

#if (RGX_NUM_OS_SUPPORTED > 1)
/* Functions used to update the host and guest LMA memory usage */
void VgpuIncreaseStatValue(PVRSRV_DEVICE_NODE *psDeviceNode, size_t byte);
void VgpuDecreaseStatValue(PVRSRV_DEVICE_NODE *psDeviceNode, size_t byte);
IMG_UINT64 VgpuGetHostUsedLMAMem(PVRSRV_DEVICE_NODE *psDeviceNode);
IMG_UINT32 VgpuGetFWTraceBackcup(PVRSRV_DEVICE_NODE *psDeviceNode);
#endif

#endif /* PROCESS_STATS_H */
