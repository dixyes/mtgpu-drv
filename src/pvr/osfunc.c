/*************************************************************************/ /*!
@File
@Title          Environment related functions
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

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_print.h>
#endif
#include <asm/page.h>
#include <asm/div64.h>
#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/platform_device.h>
#include <linux/hugetlb.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/genalloc.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <asm/hardirq.h>
#include <asm/tlbflush.h>
#include <linux/timer.h>
#include <linux/capability.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/dmaengine.h>
#include <linux/kthread.h>
#include <linux/utsname.h>
#include <linux/scatterlist.h>
#include <linux/nospec.h>
#include <linux/dma-mapping.h>
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/timekeeping.h>
#include <linux/firmware.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/verification.h>
#include <crypto/public_key.h>
#include <linux/sync_file.h>

#if defined(OS_FUNC_ARCH_LINEAR_MAP_ATTR_SET_CACHED_EXIST)
#include <linux/dma-map-ops.h>
#endif

#if defined(OS_LINUX_MODULE_SIGNATURE_H_EXIST)
#include <linux/module_signature.h>
#else
#define PKEY_ID_PKCS7 2
#endif /* OS_LINUX_MODULE_SIGNATURE_H_EXIST */

#include <linux/dma-buf.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
#include <linux/pfn_t.h>
#include <linux/pfn.h>
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/clock.h>
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)) */

#include "log2.h"
#include "osfunc.h"
#include "cache_km.h"
#include "img_defs.h"
#include "img_types.h"
#include "allocmem.h"
#include "devicemem_server_utils.h"
#include "event.h"
#include "linkage.h"
#include "pvr_uaccess.h"
#include "pvr_debug.h"
#include "pvr_drv.h"
#include "pvr_bridge_k.h"
#include "pvrsrv_memallocflags.h"
#include "physmem_dmabuf.h"
#include "module_common.h"
#if defined(PVRSRV_ENABLE_PROCESS_STATS)
#include "process_stats.h"
#endif
#if !defined(NO_HARDWARE)
#include "mtgpu.h"
#endif
#include "mtgpu_drm_drv.h"
#include "physmem_osmem_linux.h"
#include "dma_support.h"
#include "kernel_compatibility.h"

#include "pvrsrv_sync_server.h"
#include "lock.h"
#include "oskm_apphint.h"
#if defined(SUPPORT_DMA_TRANSFER)
#include "mtgpu_dma.h"
#endif

#ifndef untagged_addr
#include <linux/bitops.h>
#define __untagged_addr(addr)	\
	((__force __typeof__(addr))sign_extend64((__force u64)(addr), 55))

#define untagged_addr(addr)	({					\
	u64 __addr = (__force u64)(addr);				\
	__addr &= __untagged_addr(__addr);				\
	(__force __typeof__(addr))__addr;				\
})
#endif

#if defined(VIRTUAL_PLATFORM)
#define EVENT_OBJECT_TIMEOUT_US		(120000000ULL)
#else
#if defined(EMULATOR) || defined(TC_APOLLO_TCF5)
#define EVENT_OBJECT_TIMEOUT_US		(2000000ULL)
#else
#define EVENT_OBJECT_TIMEOUT_US		(100000ULL)
#endif /* EMULATOR */
#endif

const IMG_UINT64 OSPvrValue[] =
{
#define X(VALUE) VALUE,
	DECLEAR_OS_PVR_VALUE
#undef X
};

static void OSSetVMAFlags(struct vm_area_struct *psVma, unsigned long ulFlags)
{
#ifdef OS_VM_FLAGS_IS_NOT_CONST
	psVma->vm_flags |= ulFlags;
#else
	vm_flags_set(psVma, (vm_flags_t)ulFlags);
#endif
}

#if defined(RGX_FW_SIGNED)
#define OS_MODULE_SIG_STRING MODULE_SIG_STRING
#endif /* RGX_FW_SIGNED */

typedef struct {
	struct task_struct *kthread;
	PFN_THREAD pfnThread;
	void *hData;
	IMG_CHAR *pszThreadName;
	IMG_BOOL   bIsThreadRunning;
	IMG_BOOL   bIsSupportingThread;
	PFN_THREAD_DEBUG_DUMP pfnDebugDumpCB;
	DLLIST_NODE sNode;
} OSThreadData;

void OSSuspendTaskInterruptible(void)
{
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();
}

static DLLIST_NODE gsThreadListHead;

static void _ThreadListAddEntry(OSThreadData *psThreadListNode)
{
	dllist_add_to_tail(&gsThreadListHead, &(psThreadListNode->sNode));
}

static void _ThreadListRemoveEntry(OSThreadData *psThreadListNode)
{
	dllist_remove_node(&(psThreadListNode->sNode));
}

static void _ThreadSetStopped(OSThreadData *psOSThreadData)
{
	psOSThreadData->bIsThreadRunning = IMG_FALSE;
}

static void _OSInitThreadList(void)
{
	dllist_init(&gsThreadListHead);
}

void OSThreadDumpInfo(DUMPDEBUG_PRINTF_FUNC* pfnDumpDebugPrintf,
                      void *pvDumpDebugFile)
{
	PDLLIST_NODE psNodeCurr, psNodeNext;

	dllist_foreach_node(&gsThreadListHead, psNodeCurr, psNodeNext)
	{
		OSThreadData *psThreadListNode;
		psThreadListNode = IMG_CONTAINER_OF(psNodeCurr, OSThreadData, sNode);

		PVR_DUMPDEBUG_LOG("  %s : %s",
				  psThreadListNode->pszThreadName,
				  (psThreadListNode->bIsThreadRunning) ? "Running" : "Stopped");

		if (psThreadListNode->pfnDebugDumpCB)
		{
			psThreadListNode->pfnDebugDumpCB(pfnDumpDebugPrintf, pvDumpDebugFile);
		}
	}
}

PVRSRV_ERROR OSPhyContigPagesAlloc(PHYS_HEAP *psPhysHeap, size_t uiSize,
							PG_HANDLE *psMemHandle, IMG_DEV_PHYADDR *psDevPAddr,
							IMG_PID uiPid)
{
	PVRSRV_DEVICE_NODE *psDevNode = PhysHeapDeviceNode(psPhysHeap);
	struct device *psDev = psDevNode->psDevConfig->pvOSDevice;
	IMG_CPU_PHYADDR sCpuPAddr;
	struct page *psPage;
	IMG_UINT32	ui32Order=0;
	gfp_t gfp_flags;

	PVR_ASSERT(uiSize != 0);
	/*Align the size to the page granularity */
	uiSize = PAGE_ALIGN(uiSize);

	/*Get the order to be used with the allocation */
	ui32Order = get_order(uiSize);

	gfp_flags = GFP_KERNEL;

#if !defined(PVR_LINUX_PHYSMEM_USE_HIGHMEM_ONLY)
	if (psDev)
	{
		if (*psDev->dma_mask == DMA_BIT_MASK(32))
		{
			/* Limit to 32 bit.
			 * Achieved by setting __GFP_DMA32 for 64 bit systems */
			gfp_flags |= __GFP_DMA32;
		}
		else if (*psDev->dma_mask < DMA_BIT_MASK(32))
		{
			/* Limit to whatever the size of DMA zone is. */
			gfp_flags |= __GFP_DMA;
		}
	}
#else
	PVR_UNREFERENCED_PARAMETER(psDev);
#endif

	/*allocate the pages */
	psPage = alloc_pages(gfp_flags, ui32Order);
	if (psPage == NULL)
	{
		return PVRSRV_ERROR_OUT_OF_MEMORY;
	}
	uiSize = (1 << ui32Order) * PAGE_SIZE;

	psMemHandle->u.pvHandle = psPage;
	psMemHandle->uiOrder = ui32Order;
	sCpuPAddr.uiAddr = IMG_CAST_TO_CPUPHYADDR_UINT(page_to_phys(psPage));

	/*
	 * Even when more pages are allocated as base MMU object we still need one single physical address because
	 * they are physically contiguous.
	 */
	PhysHeapCpuPAddrToDevPAddr(psPhysHeap, 1, psDevPAddr, &sCpuPAddr);

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
#if !defined(PVRSRV_ENABLE_MEMORY_STATS)
	PVRSRVStatsIncrMemAllocStatAndTrack(PVRSRV_MEM_ALLOC_TYPE_ALLOC_PAGES_PT_UMA,
					    uiSize,
					    (IMG_UINT64)(uintptr_t)psPage,
					    uiPid,
					    psDevNode);
#else
	PVRSRVStatsAddMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE_ALLOC_PAGES_PT_UMA,
				     psPage,
				     sCpuPAddr,
				     uiSize,
				     NULL,
				     uiPid,
				     psDevNode
				     DEBUG_MEMSTATS_VALUES);
#endif
#else
	PVR_UNREFERENCED_PARAMETER(uiPid);
#endif

	return PVRSRV_OK;
}

void OSPhyContigPagesFree(PHYS_HEAP *psPhysHeap, PG_HANDLE *psMemHandle)
{
	struct page *psPage = (struct page*) psMemHandle->u.pvHandle;
	IMG_UINT32	uiSize, uiPageCount=0, ui32Order;

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
	PVRSRV_DEVICE_NODE *psDevNode = PhysHeapDeviceNode(psPhysHeap);
#endif
	ui32Order = psMemHandle->uiOrder;
	uiPageCount = (1 << ui32Order);
	uiSize = (uiPageCount * PAGE_SIZE);

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
#if !defined(PVRSRV_ENABLE_MEMORY_STATS)
	PVRSRVStatsDecrMemAllocStatAndUntrack(PVRSRV_MEM_ALLOC_TYPE_ALLOC_PAGES_PT_UMA,
					      (IMG_UINT64)(uintptr_t)psPage,
					      psDevNode);
#else
	PVRSRVStatsRemoveMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE_ALLOC_PAGES_PT_UMA,
					(IMG_UINT64)(uintptr_t) psPage,
					OSGetCurrentClientProcessIDKM(),
					psDevNode);
#endif
#endif

	__free_pages(psPage, ui32Order);
	psMemHandle->uiOrder = 0;
}

PVRSRV_ERROR OSPhyContigPagesMap(PHYS_HEAP *psPhysHeap, PG_HANDLE *psMemHandle,
						size_t uiSize, IMG_DEV_PHYADDR *psDevPAddr,
						void **pvPtr)
{
#if defined(PVRSRV_ENABLE_PROCESS_STATS)
	PVRSRV_DEVICE_NODE *psDevNode = PhysHeapDeviceNode(psPhysHeap);
#endif
	size_t actualSize = 1 << (PAGE_SHIFT + psMemHandle->uiOrder);
	*pvPtr = kmap((struct page*)psMemHandle->u.pvHandle);

	PVR_UNREFERENCED_PARAMETER(psDevPAddr);

	PVR_UNREFERENCED_PARAMETER(actualSize); /* If we don't take an #ifdef path */
	PVR_UNREFERENCED_PARAMETER(uiSize);

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
#if !defined(PVRSRV_ENABLE_MEMORY_STATS)
	PVRSRVStatsIncrMemAllocStat(PVRSRV_MEM_ALLOC_TYPE_VMAP_PT_UMA,
				    actualSize,
				    OSGetCurrentClientProcessIDKM(),
				    psDevNode);
#else
	{
		IMG_CPU_PHYADDR sCpuPAddr;
		sCpuPAddr.uiAddr = 0;

		PVRSRVStatsAddMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE_VMAP_PT_UMA,
					     *pvPtr,
					     sCpuPAddr,
					     actualSize,
					     NULL,
					     OSGetCurrentClientProcessIDKM(),
					     psDevNode
					     DEBUG_MEMSTATS_VALUES);
	}
#endif
#endif

	return PVRSRV_OK;
}

void OSPhyContigPagesUnmap(PHYS_HEAP *psPhysHeap, PG_HANDLE *psMemHandle, void *pvPtr)
{
#if defined(PVRSRV_ENABLE_PROCESS_STATS)
	PVRSRV_DEVICE_NODE * psDevNode = PhysHeapDeviceNode(psPhysHeap);
#if !defined(PVRSRV_ENABLE_MEMORY_STATS)
	/* Mapping is done a page at a time */
	PVRSRVStatsDecrMemAllocStat(PVRSRV_MEM_ALLOC_TYPE_VMAP_PT_UMA,
				    (1 << (PAGE_SHIFT + psMemHandle->uiOrder)),
				    OSGetCurrentClientProcessIDKM(),
				    psDevNode);
#else
	PVRSRVStatsRemoveMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE_VMAP_PT_UMA,
					(IMG_UINT64)(uintptr_t)pvPtr,
					OSGetCurrentClientProcessIDKM(),
					psDevNode);
#endif
#endif

	PVR_UNREFERENCED_PARAMETER(psPhysHeap);
	PVR_UNREFERENCED_PARAMETER(pvPtr);

	kunmap((struct page*) psMemHandle->u.pvHandle);
}

PVRSRV_ERROR OSPhyContigPagesClean(PHYS_HEAP *psPhysHeap,
                                   PG_HANDLE *psMemHandle,
                                   IMG_UINT32 uiOffset,
                                   IMG_UINT32 uiLength)
{
	PVRSRV_DEVICE_NODE *psDevNode = PhysHeapDeviceNode(psPhysHeap);
	PVRSRV_ERROR eError = PVRSRV_OK;
	struct page* psPage = (struct page*) psMemHandle->u.pvHandle;

	void* pvVirtAddrStart = kmap(psPage) + uiOffset;
	IMG_CPU_PHYADDR sPhysStart, sPhysEnd;

	IMG_UINT32 ui32Order;

	if (uiLength == 0)
	{
		goto e0;
	}

	ui32Order = psMemHandle->uiOrder;
	if ((uiOffset + uiLength) > ((1 << ui32Order) * PAGE_SIZE))
	{
		PVR_DPF((PVR_DBG_ERROR,
				"%s: Invalid size params, uiOffset %u, uiLength %u",
				__func__,
				uiOffset,
				uiLength));
		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto e0;
	}

	sPhysStart.uiAddr = page_to_phys(psPage) + uiOffset;
	sPhysEnd.uiAddr = sPhysStart.uiAddr + uiLength;

	CacheOpExec(psDevNode,
				pvVirtAddrStart,
				pvVirtAddrStart + uiLength,
				sPhysStart,
				sPhysEnd,
				PVRSRV_CACHE_OP_CLEAN);

e0:
	kunmap(psPage);

	return eError;
}

#if defined(__GNUC__)
#define PVRSRV_MEM_ALIGN __attribute__ ((aligned (0x8)))
#define PVRSRV_MEM_ALIGN_MASK (0x7)
#else
#error "PVRSRV Alignment macros need to be defined for this compiler"
#endif

IMG_UINT32 OSCPUCacheAttributeSize(OS_CPU_CACHE_ATTRIBUTE eCacheAttribute)
{
	IMG_UINT32 uiSize = 0;

	switch (eCacheAttribute)
	{
		case OS_CPU_CACHE_ATTRIBUTE_LINE_SIZE:
			uiSize = cache_line_size();
			break;

		default:
			PVR_DPF((PVR_DBG_ERROR, "%s: Invalid cache attribute type %d",
					__func__, (IMG_UINT32)eCacheAttribute));
			PVR_ASSERT(0);
			break;
	}

	return uiSize;
}

IMG_UINT32 OSVSScanf(const IMG_CHAR *pStr, const IMG_CHAR *pszFormat, ...)
{
	va_list argList;
	IMG_INT32 iCount = 0;

	va_start(argList, pszFormat);
	iCount = vsscanf(pStr, pszFormat, argList);
	va_end(argList);

	return iCount;
}

IMG_INT OSMemCmp(void *pvBufA, void *pvBufB, size_t uiLen)
{
	return (IMG_INT)memcmp(pvBufA, pvBufB, uiLen);
}

void *OSMemSet(void *pvBuf, IMG_INT ui32Str, size_t uiLen)
{
	return memset(pvBuf, ui32Str, uiLen);
}

void OSMemSetIO(void *pvBuf, IMG_INT ui32Str, size_t uiLen)
{
	return memset_io(pvBuf, ui32Str, uiLen);
}

void *OSMemCopy(void *pvDest, const void *pvSrc, size_t uiLen)
{
	return memcpy(pvDest, pvSrc, uiLen);
}

size_t OSStringSCpy(IMG_CHAR *pszDest, const IMG_CHAR *pszSrc, size_t uDstSize)
{
	return strscpy(pszDest, pszSrc, uDstSize);
}

size_t OSStringLCat(IMG_CHAR *pszDest, const IMG_CHAR *pszSrc, size_t uDstSize)
{
	/*
	 * Let strlcat handle any truncation cases correctly.
	 * We will definitely get a NUL-terminated string set in pszDest
	 */
	size_t uSrcSize = strlcat(pszDest, pszSrc, uDstSize);

#if defined(PVR_DEBUG_STRLCPY) && defined(DEBUG)
	/* Handle truncation by dumping calling stack if debug allows */
	if (uSrcSize >= uDstSize)
	{
		PVR_DPF((PVR_DBG_WARNING,
			"%s: String truncated Src = '<%s>' %ld bytes, Dest = '%s'",
			__func__, pszSrc, (long)uDstSize, pszDest));
		OSDumpStack();
	}
#endif /* defined(PVR_DEBUG_STRLCPY) && defined(DEBUG) */

	return uSrcSize;
}

IMG_CHAR *OSStringChr(const IMG_CHAR *pStr, IMG_INT iChar)
{
	return strchr(pStr, iChar);
}

IMG_INT32 OSSNPrintf(IMG_CHAR *pStr, size_t ui32Size, const IMG_CHAR *pszFormat, ...)
{
	va_list argList;
	IMG_INT32 iCount;

	va_start(argList, pszFormat);
	iCount = vsnprintf(pStr, (size_t)ui32Size, pszFormat, argList);
	va_end(argList);

	return iCount;
}

IMG_INT32 OSVSNPrintf(IMG_CHAR *pStr, size_t ui32Size, const IMG_CHAR* pszFormat, va_list vaArgs)
{
	return vsnprintf(pStr, ui32Size, pszFormat, vaArgs);
}

size_t OSStringLength(const IMG_CHAR *pStr)
{
	return strlen(pStr);
}

size_t OSStringNLength(const IMG_CHAR *pStr, size_t uiCount)
{
	return strnlen(pStr, uiCount);
}

IMG_INT32 OSStringCompare(const IMG_CHAR *pStr1, const IMG_CHAR *pStr2)
{
	return strcmp(pStr1, pStr2);
}

IMG_INT32 OSStringNCompare(const IMG_CHAR *pStr1, const IMG_CHAR *pStr2,
                          size_t uiSize)
{
#if defined(DEBUG)
	/* Double-check that we are not passing NULL parameters in. If we are we
	 * return -1 (for arg1 == NULL, arg2 != NULL)
	 * 0 (for arg1 == NULL, arg2 == NULL
	 * +1 (for arg1 != NULL, arg2 == NULL)
	 * strncmp(arg1, arg2, size) otherwise
	 */
	if (pStr1 == NULL)
	{
		if (pStr2 == NULL)
		{
			PVR_DPF((PVR_DBG_ERROR, "%s(%p, %p, %d): Both args NULL",
				 __func__, pStr1, pStr2, (int)uiSize));
			OSDumpStack();
			return 0;	/* Both NULL */
		}
		else
		{
			PVR_DPF((PVR_DBG_ERROR, "%s(%p, %p, %d): arg1 NULL",
				 __func__, pStr1, pStr2, (int)uiSize));
			OSDumpStack();
			return -1;	/* NULL < non-NULL */
		}
	}
	else
	{
		if (pStr2 == NULL)
		{
			PVR_DPF((PVR_DBG_ERROR, "%s(%p, %p, %d): arg2 NULL",
				 __func__, pStr1, pStr2, (int)uiSize));
			OSDumpStack();
			return +1;	/* non-NULL > NULL */
		}
		else
		{
			return strncmp(pStr1, pStr2, uiSize);
		}
	}
#else
	return strncmp(pStr1, pStr2, uiSize);
#endif
}

IMG_CHAR *OSStringSeparate(IMG_CHAR **pStr1, const IMG_CHAR *pStr2)
{
	return strsep(pStr1, pStr2);
}

int OSStringCaseCompare(const IMG_CHAR *pStr1, const IMG_CHAR *pStr2)
{
	return strcasecmp(pStr1, pStr2);
}

PVRSRV_ERROR OSStringToUINT64(const IMG_CHAR *pStr, IMG_UINT64 ui64Base,
			      IMG_UINT64 *ui64Result)
{
	if (kstrtou64(pStr, ui64Base, ui64Result) != 0)
		return PVRSRV_ERROR_CONVERSION_FAILED;

	return PVRSRV_OK;
}

PVRSRV_ERROR OSStringToUINT32(const IMG_CHAR *pStr, IMG_UINT32 ui32Base,
                              IMG_UINT32 *ui32Result)
{
	if (kstrtou32(pStr, ui32Base, ui32Result) != 0)
		return PVRSRV_ERROR_CONVERSION_FAILED;

	return PVRSRV_OK;
}

IMG_UINT32 OSStringUINT32ToStr(IMG_CHAR *pszBuf, size_t uSize,
							IMG_UINT32 ui32Num)
{
	IMG_UINT32 ui32i, ui32Len = 0, ui32NumCopy = ui32Num;

	/* calculate string length required to hold the number string */
	do
	{
		ui32Len++;
		ui32NumCopy /= 10;
	} while (ui32NumCopy != 0);

	if (unlikely(ui32Len >= uSize))
	{
		/* insufficient buffer */
		return 0;
	}

	for (ui32i = 0; ui32i < ui32Len; ui32i++)
	{
		pszBuf[ui32Len - (ui32i + 1)] = '0' + ui32Num % 10;
		ui32Num = ui32Num / 10;
	}

	pszBuf[ui32Len] = '\0';
	return ui32Len;
}

IMG_CHAR *OSStringString(const IMG_CHAR *str1, const IMG_CHAR *str2)
{
	return strstr(str1, str2);
}

#if defined(SUPPORT_NATIVE_FENCE_SYNC) || defined(SUPPORT_BUFFER_SYNC)
static struct workqueue_struct *gpFenceStatusWq;

static PVRSRV_ERROR _NativeSyncInit(void)
{
#if defined(PVRSRV_ENABLE_WORK_QUEUE_OPTIMIZATION)
	gpFenceStatusWq = alloc_workqueue("%s", __WQ_LEGACY | WQ_FREEZABLE | WQ_CPU_INTENSIVE |
					  WQ_MEM_RECLAIM, 1, "musa_fence_status");
#else
	gpFenceStatusWq = create_freezable_workqueue("musa_fence_status");
#endif
	if (!gpFenceStatusWq)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Failed to create foreign fence status workqueue",
				 __func__));
		return PVRSRV_ERROR_INIT_FAILURE;
	}

	return PVRSRV_OK;
}

static void _NativeSyncDeinit(void)
{
	destroy_workqueue(gpFenceStatusWq);
}

struct workqueue_struct *NativeSyncGetFenceStatusWq(void)
{
	if (!gpFenceStatusWq)
	{
#if defined(DEBUG)
		PVR_ASSERT(gpFenceStatusWq);
#endif
		return NULL;
	}

	return gpFenceStatusWq;
}
#endif

PVRSRV_ERROR OSInitEnvData(void)
{
	PVRSRV_ERROR eError = PVRSRV_OK;

	LinuxInitPhysmem();

	_OSInitThreadList();

#if defined(SUPPORT_NATIVE_FENCE_SYNC) || defined(SUPPORT_BUFFER_SYNC)
	eError = _NativeSyncInit();
#endif

	return eError;
}

void OSDeInitEnvData(void)
{
#if defined(SUPPORT_NATIVE_FENCE_SYNC) || defined(SUPPORT_BUFFER_SYNC)
	_NativeSyncDeinit();
#endif

	LinuxDeinitPhysmem();
}

void OSReleaseThreadQuanta(void)
{
	schedule();
}

void OSMemoryBarrier(volatile void *hReadback)
{
	mb();

	if (hReadback)
	{
		/* Force a read-back to memory to avoid posted writes on certain buses
		 * e.g. PCI(E)
		 */
		(void) OSReadDeviceMem32(hReadback);
	}
}

void OSWriteMemoryBarrier(volatile void *hReadback)
{
	wmb();

	if (hReadback)
	{
		/* Force a read-back to memory to avoid posted writes on certain buses
		 * e.g. PCI(E)
		 */
		(void) OSReadDeviceMem32(hReadback);
	}
}

/* Not matching/aligning this API to the Clockus() API above to avoid necessary
 * multiplication/division operations in calling code.
 */
static inline IMG_UINT64 Clockns64(void)
{
	IMG_UINT64 timenow;

	/* Kernel thread preempt protection. Some architecture implementations
	 * (ARM) of sched_clock are not preempt safe when the kernel is configured
	 * as such e.g. CONFIG_PREEMPT and others.
	 */
	preempt_disable();

	/* Using sched_clock instead of ktime_get since we need a time stamp that
	 * correlates with that shown in kernel logs and trace data not one that
	 * is a bit behind. */
	timenow = sched_clock();

	preempt_enable();

	return timenow;
}

IMG_UINT64 OSClockns64(void)
{
	return Clockns64();
}

IMG_UINT64 OSClockus64(void)
{
	IMG_UINT64 timenow = Clockns64();
	IMG_UINT32 remainder;

	return OSDivide64r64(timenow, 1000, &remainder);
}

IMG_UINT32 OSClockus(void)
{
	return (IMG_UINT32) OSClockus64();
}

IMG_UINT32 OSClockms(void)
{
	IMG_UINT64 timenow = Clockns64();
	IMG_UINT32 remainder;

	return OSDivide64(timenow, 1000000, &remainder);
}

static inline IMG_UINT64 KClockns64(void)
{
	ktime_t sTime = ktime_get();

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	return sTime;
#else
	return sTime.tv64;
#endif
}

PVRSRV_ERROR OSClockMonotonicns64(IMG_UINT64 *pui64Time)
{
	*pui64Time = KClockns64();
	return PVRSRV_OK;
}

PVRSRV_ERROR OSClockMonotonicus64(IMG_UINT64 *pui64Time)
{
	IMG_UINT64 timenow = KClockns64();
	IMG_UINT32 remainder;

	*pui64Time = OSDivide64r64(timenow, 1000, &remainder);
	return PVRSRV_OK;
}

#if defined(OS_FUNC_GETRAWMONOTONIC_EXIST) && defined(OS_STRUCT_TIMESPEC_EXIST)
IMG_UINT64 OSClockMonotonicRawns64(void)
{
	struct timespec ts;

	getrawmonotonic(&ts);
	return (IMG_UINT64) ts.tv_sec * 1000000000 + ts.tv_nsec;
}
#else
IMG_UINT64 OSClockMonotonicRawns64(void)
{
	struct timespec64 ts;

	ktime_get_raw_ts64(&ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}
#endif

IMG_UINT64 OSClockMonotonicRawus64(void)
{
	IMG_UINT32 rem;
	return OSDivide64r64(OSClockMonotonicRawns64(), 1000, &rem);
}

/*
	OSWaitus
*/
void OSWaitus(IMG_UINT32 ui32Timeus)
{
	udelay(ui32Timeus);
}


/*
	OSSleepms
*/
void OSSleepms(IMG_UINT32 ui32Timems)
{
	msleep(ui32Timems);
}


INLINE IMG_UINT64 OSGetCurrentProcessVASpaceSize(void)
{
	return (IMG_UINT64)TASK_SIZE;
}

INLINE IMG_PID OSGetCurrentProcessID(void)
{
	if (in_interrupt())
	{
		return KERNEL_ID;
	}

	return (IMG_PID)task_tgid_nr(current);
}

INLINE IMG_PID OSGetCurrentVirtualProcessID(void)
{
	if (in_interrupt())
	{
		return KERNEL_ID;
	}

	return (IMG_PID)task_tgid_vnr(current);
}

INLINE IMG_CHAR *OSGetCurrentProcessName(void)
{
	return current->comm;
}

INLINE uintptr_t OSGetCurrentThreadID(void)
{
	if (in_interrupt())
	{
		return KERNEL_ID;
	}

	return current->pid;
}

IMG_PID OSGetCurrentClientProcessIDKM(void)
{
	return OSGetCurrentProcessID();
}

IMG_CHAR *OSGetCurrentClientProcessNameKM(void)
{
	return OSGetCurrentProcessName();
}

uintptr_t OSGetCurrentClientThreadIDKM(void)
{
	return OSGetCurrentThreadID();
}

pid_t OSGetCurrentTgid(void)
{
	return current->tgid;
}

size_t OSGetPageSize(void)
{
	return PAGE_SIZE;
}

size_t OSGetPageShift(void)
{
	return PAGE_SHIFT;
}

size_t OSGetPageMask(void)
{
	return (OSGetPageSize()-1);
}

size_t OSGetOrder(size_t uSize)
{
	return get_order(PAGE_ALIGN(uSize));
}

IMG_UINT64 OSGetRAMSize(void)
{
	struct sysinfo SI;
	si_meminfo(&SI);

	return (PAGE_SIZE * SI.totalram);
}

typedef struct
{
	int os_error;
	PVRSRV_ERROR pvr_error;
} error_map_t;

/* return -ve versions of POSIX errors as they are used in this form */
static const error_map_t asErrorMap[] =
{
	{-EFAULT, PVRSRV_ERROR_BRIDGE_EFAULT},
	{-EINVAL, PVRSRV_ERROR_BRIDGE_EINVAL},
	{-ENOMEM, PVRSRV_ERROR_BRIDGE_ENOMEM},
	{-ERANGE, PVRSRV_ERROR_BRIDGE_ERANGE},
	{-EPERM,  PVRSRV_ERROR_BRIDGE_EPERM},
	{-ENOTTY, PVRSRV_ERROR_BRIDGE_ENOTTY},
	{-ENOTTY, PVRSRV_ERROR_BRIDGE_CALL_FAILED},
	{-ERANGE, PVRSRV_ERROR_BRIDGE_BUFFER_TOO_SMALL},
	{-ENOMEM, PVRSRV_ERROR_OUT_OF_MEMORY},
	{-EINVAL, PVRSRV_ERROR_INVALID_PARAMS},

	{0,       PVRSRV_OK}
};

int PVRSRVToNativeError(PVRSRV_ERROR e)
{
	int os_error = -EFAULT;
	int i;

	for (i = 0; i < ARRAY_SIZE(asErrorMap); i++)
	{
		if (e == asErrorMap[i].pvr_error)
		{
			os_error = asErrorMap[i].os_error;
			break;
		}
	}
	return os_error;
}

typedef struct  _MISR_DATA_ {
	struct workqueue_struct *psWorkQueue;
	struct work_struct sMISRWork;
	const IMG_CHAR* pszName;
	PFN_MISR pfnMISR;
	void *hData;
} MISR_DATA;

/*
	MISRWrapper
*/
static void MISRWrapper(struct work_struct *data)
{
	MISR_DATA *psMISRData = container_of(data, MISR_DATA, sMISRWork);

	PVR_DPF((PVR_DBG_MESSAGE, "Waking up '%s' MISR %p", psMISRData->pszName, psMISRData));

	psMISRData->pfnMISR(psMISRData->hData);
}

/*
	OSInstallMISR
*/
PVRSRV_ERROR OSInstallMISR(IMG_HANDLE *hMISRData, PFN_MISR pfnMISR,
							void *hData, const IMG_CHAR *pszMisrName)
{
	MISR_DATA *psMISRData;

	psMISRData = OSAllocMem(sizeof(*psMISRData));
	PVR_LOG_RETURN_IF_NOMEM(psMISRData, "psMISRData");

	psMISRData->hData = hData;
	psMISRData->pfnMISR = pfnMISR;
	psMISRData->pszName = pszMisrName;

	PVR_DPF((PVR_DBG_MESSAGE, "Installing MISR with cookie %p", psMISRData));

#if defined(PVRSRV_ENABLE_WORK_QUEUE_OPTIMIZATION)
	if (pszMisrName)
		psMISRData->psWorkQueue = alloc_workqueue("%s", __WQ_ORDERED |
							  __WQ_ORDERED_EXPLICIT |
							  __WQ_LEGACY | WQ_MEM_RECLAIM |
							  WQ_CPU_INTENSIVE, 1, pszMisrName);
	else
		psMISRData->psWorkQueue = alloc_workqueue("%s", __WQ_ORDERED |
							  __WQ_ORDERED_EXPLICIT |
							  __WQ_LEGACY | WQ_MEM_RECLAIM |
							  WQ_CPU_INTENSIVE, 1, "MUSA_Misr");
#else
	if (pszMisrName)
		psMISRData->psWorkQueue = create_singlethread_workqueue(pszMisrName);
	else
		psMISRData->psWorkQueue = create_singlethread_workqueue("MUSA_Misr");
#endif

	if (psMISRData->psWorkQueue == NULL)
	{
#if defined(PVRSRV_ENABLE_WORK_QUEUE_OPTIMIZATION)
		PVR_DPF((PVR_DBG_ERROR, "OSInstallMISR: alloc_workqueue failed"));
#else
		PVR_DPF((PVR_DBG_ERROR, "OSInstallMISR: create_singlethreaded_workqueue failed"));
#endif
		OSFreeMem(psMISRData);
		return PVRSRV_ERROR_UNABLE_TO_CREATE_THREAD;
	}

	INIT_WORK(&psMISRData->sMISRWork, MISRWrapper);

	*hMISRData = (IMG_HANDLE) psMISRData;

	return PVRSRV_OK;
}

/*
	OSUninstallMISR
*/
PVRSRV_ERROR OSUninstallMISR(IMG_HANDLE hMISRData)
{
	MISR_DATA *psMISRData = (MISR_DATA *) hMISRData;

	PVR_DPF((PVR_DBG_MESSAGE, "Uninstalling MISR with cookie %p", psMISRData));

	destroy_workqueue(psMISRData->psWorkQueue);
	OSFreeMem(psMISRData);

	return PVRSRV_OK;
}

/*
	OSScheduleMISR
*/
PVRSRV_ERROR OSScheduleMISR(IMG_HANDLE hMISRData)
{
	MISR_DATA *psMISRData = (MISR_DATA *) hMISRData;
#if defined(PVRSRV_ENABLE_WORK_QUEUE_OPTIMIZATION)
	const IMG_CHAR *pszAppHintDefault = PVRSRV_APPHINT_MISRWORKONSPECIFIEDCPU;
	void *pvAppHintState = NULL;
	static IMG_UINT32 ui32SelectedCpu[MAX_CPU_NUM];
	static IMG_UINT32 ui32SelectedCpuCount;
	static IMG_UINT32 ui32CurrentCpu;
	static IMG_BOOL has_parsed;
	IMG_CHAR szMISRWorkOnSpecifiedCPUAppHint[256];
	IMG_CHAR *pszToken;
	IMG_UINT32 ui32OfflineCpuCount = 0;
	IMG_CHAR *pszAppintStr = szMISRWorkOnSpecifiedCPUAppHint;

	if (!has_parsed)
	{
		OSCreateKMAppHintState(&pvAppHintState);
		OSGetKMAppHintSTRING(APPHINT_NO_DEVICE,
				     pvAppHintState,
				     MISRWorkOnSpecifiedCPU,
				     pszAppHintDefault,
				     szMISRWorkOnSpecifiedCPUAppHint,
				     sizeof(szMISRWorkOnSpecifiedCPUAppHint));
		OSFreeKMAppHintState(pvAppHintState);

		pszToken = strsep(&pszAppintStr, ",");
		while (pszToken) {
			kstrtou32(pszToken, 0, &ui32SelectedCpu[ui32SelectedCpuCount]);
			ui32SelectedCpuCount++;
			pszToken = strsep(&pszAppintStr, ",");
		}

		has_parsed = true;
	}

	while (!cpu_online(ui32SelectedCpu[ui32CurrentCpu]))
	{
		PVR_DPF((PVR_DBG_ERROR, "%d CPU is offline", ui32SelectedCpu[ui32CurrentCpu]));
		ui32CurrentCpu++;
		ui32OfflineCpuCount++;
		if (ui32OfflineCpuCount == ui32SelectedCpuCount) {
			PVR_DPF((PVR_DBG_ERROR, " All Selected CPUs are offline"));
			return PVRSRV_ERROR_NOT_READY;
		}

		if (ui32CurrentCpu == ui32SelectedCpuCount)
			ui32CurrentCpu = 0;
	}
#endif
	/*
		Note:

		In the case of NO_HARDWARE we want the driver to be synchronous so
		that we don't have to worry about waiting for previous operations
		to complete
	*/
#if defined(NO_HARDWARE)
	psMISRData->pfnMISR(psMISRData->hData);
	return PVRSRV_OK;
#else
	{
#if defined(PVRSRV_ENABLE_WORK_QUEUE_OPTIMIZATION)
		bool rc = queue_work_on(ui32SelectedCpu[ui32CurrentCpu],
					psMISRData->psWorkQueue, &psMISRData->sMISRWork);

		if (ui32CurrentCpu == ui32SelectedCpuCount - 1)
			ui32CurrentCpu = 0;
		else
			ui32CurrentCpu++;
#else
		bool rc = queue_work(psMISRData->psWorkQueue, &psMISRData->sMISRWork);
#endif
		return rc ? PVRSRV_OK : PVRSRV_ERROR_ALREADY_EXISTS;
	}
#endif
}

/* OS specific values for thread priority */
static const IMG_INT32 ai32OSPriorityValues[OS_THREAD_LAST_PRIORITY] =
{
	  0, /* OS_THREAD_NOSET_PRIORITY */
	-20, /* OS_THREAD_HIGHEST_PRIORITY */
	-10, /* OS_THREAD_HIGH_PRIORITY */
	  0, /* OS_THREAD_NORMAL_PRIORITY */
	  9, /* OS_THREAD_LOW_PRIORITY */
	 19, /* OS_THREAD_LOWEST_PRIORITY */
};

static int OSThreadRun(void *data)
{
	OSThreadData *psOSThreadData = data;

	/* count freezable threads */
	LinuxBridgeNumActiveKernelThreadsIncrement();

	/* Returns true if the thread was frozen, should we do anything with this
	 * information? What do we return? Which one is the error case? */
	set_freezable();

	PVR_DPF((PVR_DBG_MESSAGE, "Starting Thread '%s'...", psOSThreadData->pszThreadName));

	/* Call the client's kernel thread with the client's data pointer */
	psOSThreadData->pfnThread(psOSThreadData->hData);

	if (psOSThreadData->bIsSupportingThread)
	{
		_ThreadSetStopped(psOSThreadData);
	}

	/* Wait for OSThreadDestroy() to call kthread_stop() */
	while (!kthread_freezable_should_stop(NULL))
	{
		schedule();
	}

	LinuxBridgeNumActiveKernelThreadsDecrement();

	return 0;
}

PVRSRV_ERROR OSThreadCreate(IMG_HANDLE *phThread,
                            IMG_CHAR *pszThreadName,
                            PFN_THREAD pfnThread,
                            PFN_THREAD_DEBUG_DUMP pfnDebugDumpCB,
                            IMG_BOOL bIsSupportingThread,
                            void *hData)
{
	return OSThreadCreatePriority(phThread, pszThreadName, pfnThread,
	                              pfnDebugDumpCB, bIsSupportingThread, hData,
	                              OS_THREAD_NOSET_PRIORITY);
}

PVRSRV_ERROR OSThreadCreatePriority(IMG_HANDLE *phThread,
                                    IMG_CHAR *pszThreadName,
                                    PFN_THREAD pfnThread,
                                    PFN_THREAD_DEBUG_DUMP pfnDebugDumpCB,
                                    IMG_BOOL bIsSupportingThread,
                                    void *hData,
                                    OS_THREAD_LEVEL eThreadPriority)
{
	OSThreadData *psOSThreadData;
	PVRSRV_ERROR eError;

	psOSThreadData = OSAllocZMem(sizeof(*psOSThreadData));
	PVR_LOG_GOTO_IF_NOMEM(psOSThreadData, eError, fail_alloc);

	psOSThreadData->pfnThread = pfnThread;
	psOSThreadData->hData = hData;
	psOSThreadData->kthread = kthread_run(OSThreadRun, psOSThreadData, "%s", pszThreadName);

	if (IS_ERR(psOSThreadData->kthread))
	{
		eError = PVRSRV_ERROR_OUT_OF_MEMORY;
		goto fail_kthread;
	}

	if (bIsSupportingThread)
	{
		psOSThreadData->pszThreadName = pszThreadName;
		psOSThreadData->pfnDebugDumpCB = pfnDebugDumpCB;
		psOSThreadData->bIsThreadRunning = IMG_TRUE;
		psOSThreadData->bIsSupportingThread = IMG_TRUE;

		_ThreadListAddEntry(psOSThreadData);
	}

	if (eThreadPriority != OS_THREAD_NOSET_PRIORITY &&
	    eThreadPriority < OS_THREAD_LAST_PRIORITY)
	{
		set_user_nice(psOSThreadData->kthread,
		              ai32OSPriorityValues[eThreadPriority]);
	}

	*phThread = psOSThreadData;

	return PVRSRV_OK;

fail_kthread:
	OSFreeMem(psOSThreadData);
fail_alloc:
	PVR_ASSERT(eError != PVRSRV_OK);
	return eError;
}

PVRSRV_ERROR OSThreadDestroy(IMG_HANDLE hThread)
{
	OSThreadData *psOSThreadData = hThread;
	int ret;

	/* Let the thread know we are ready for it to end and wait for it. */
	ret = kthread_stop(psOSThreadData->kthread);
	if (0 != ret)
	{
		PVR_DPF((PVR_DBG_WARNING, "kthread_stop failed(%d)", ret));
		return PVRSRV_ERROR_RETRY;
	}

	if (psOSThreadData->bIsSupportingThread)
	{
		_ThreadListRemoveEntry(psOSThreadData);
	}

	OSFreeMem(psOSThreadData);

	return PVRSRV_OK;
}

#if defined(__linux__) && defined(__KERNEL__) && !defined(DOXYGEN)
int OSWarnOn(IMG_BOOL bCondition)
{
	return WARN_ON(bCondition);
}
#endif

void OSPanic(void)
{
	BUG();

#if defined(__KLOCWORK__)
	/* Klocwork does not understand that BUG is terminal... */
	abort();
#endif
}

void *
OSMapPhysToLin(IMG_CPU_PHYADDR BasePAddr,
			   size_t ui32Bytes,
			   PVRSRV_MEMALLOCFLAGS_T uiMappingFlags)
{
	void __iomem *pvLinAddr;

	if (uiMappingFlags & ~(PVRSRV_MEMALLOCFLAG_CPU_CACHE_MODE_MASK))
	{
		PVR_ASSERT(!"Found non-cpu cache mode flag when mapping to the cpu");
		return NULL;
	}

	if (! PVRSRV_VZ_MODE_IS(NATIVE))
	{
		/*
		  This is required to support DMA physheaps for GPU virtualization.
		  Unfortunately, if a region of kernel managed memory is turned into
		  a DMA buffer, conflicting mappings can come about easily on Linux
		  as the original memory is mapped by the kernel as normal cached
		  memory whilst DMA buffers are mapped mostly as uncached device or
		  cache-coherent device memory. In both cases the system will have
		  two conflicting mappings for the same memory region and will have
		  "undefined behaviour" for most processors notably ARMv6 onwards
		  and some x86 micro-architectures. As a result, perform ioremapping
		  manually for DMA physheap allocations by translating from CPU/VA
		  to BUS/PA thereby preventing the creation of conflicting mappings.
		*/
		pvLinAddr = (void __iomem *) SysDmaDevPAddrToCpuVAddr(BasePAddr.uiAddr, ui32Bytes);
		if (pvLinAddr != NULL)
		{
			return (void __force *) pvLinAddr;
		}
	}

	switch (uiMappingFlags)
	{
		case PVRSRV_MEMALLOCFLAG_CPU_UNCACHED:
			pvLinAddr = (void __iomem *)ioremap(BasePAddr.uiAddr, ui32Bytes);
			break;
		case PVRSRV_MEMALLOCFLAG_CPU_UNCACHED_WC:
#if defined(CONFIG_X86) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
			pvLinAddr = (void __iomem *)ioremap_wc(BasePAddr.uiAddr, ui32Bytes);
#else
			pvLinAddr = (void __iomem *)ioremap(BasePAddr.uiAddr, ui32Bytes);
#endif
			break;
		case PVRSRV_MEMALLOCFLAG_CPU_CACHED:
#if defined(CONFIG_X86) || defined(CONFIG_ARM)
			pvLinAddr = (void __iomem *)ioremap_cache(BasePAddr.uiAddr, ui32Bytes);
#else
			pvLinAddr = (void __iomem *)ioremap(BasePAddr.uiAddr, ui32Bytes);
#endif
			break;
		case PVRSRV_MEMALLOCFLAG_CPU_CACHE_COHERENT:
		case PVRSRV_MEMALLOCFLAG_CPU_CACHE_INCOHERENT:
			PVR_ASSERT(!"Unexpected cpu cache mode");
			pvLinAddr = NULL;
			break;
		default:
			PVR_ASSERT(!"Unsupported cpu cache mode");
			pvLinAddr = NULL;
			break;
	}

	return (void __force *) pvLinAddr;
}


IMG_BOOL
OSUnMapPhysToLin(void *pvLinAddr, size_t ui32Bytes)
{
	PVR_UNREFERENCED_PARAMETER(ui32Bytes);

	if (!PVRSRV_VZ_MODE_IS(NATIVE))
	{
		if (SysDmaCpuVAddrToDevPAddr(pvLinAddr))
		{
			return IMG_TRUE;
		}
	}

	iounmap((void __iomem *) pvLinAddr);

	return IMG_TRUE;
}

#define OS_MAX_TIMERS	8

/* Timer callback structure used by OSAddTimer */
typedef struct TIMER_CALLBACK_DATA_TAG
{
	IMG_BOOL			bInUse;
	PFN_TIMER_FUNC		pfnTimerFunc;
	void				*pvData;
	struct timer_list	sTimer;
	IMG_UINT32			ui32Delay;
	IMG_BOOL			bActive;
	struct work_struct	sWork;
}TIMER_CALLBACK_DATA;

static struct workqueue_struct *psTimerWorkQueue;

static TIMER_CALLBACK_DATA sTimers[OS_MAX_TIMERS];

static DEFINE_MUTEX(sTimerStructLock);

static void OSTimerCallbackBody(TIMER_CALLBACK_DATA *psTimerCBData)
{
	if (!psTimerCBData->bActive)
		return;

	/* call timer callback */
	psTimerCBData->pfnTimerFunc(psTimerCBData->pvData);

	/* reset timer */
	mod_timer(&psTimerCBData->sTimer, psTimerCBData->sTimer.expires + psTimerCBData->ui32Delay);
}


#if defined(from_timer)
/*************************************************************************/ /*!
@Function       OSTimerCallbackWrapper
@Description    OS specific timer callback wrapper function
@Input          psTimer    Timer list structure
*/ /**************************************************************************/
static void OSTimerCallbackWrapper(struct timer_list *psTimer)
{
	TIMER_CALLBACK_DATA *psTimerCBData = from_timer(psTimerCBData, psTimer, sTimer);
#else
/*************************************************************************/ /*!
@Function       OSTimerCallbackWrapper
@Description    OS specific timer callback wrapper function
@Input          uData    Timer callback data
*/ /**************************************************************************/
static void OSTimerCallbackWrapper(uintptr_t uData)
{
	TIMER_CALLBACK_DATA *psTimerCBData = (TIMER_CALLBACK_DATA*)uData;
#endif
	int res;

	res = queue_work(psTimerWorkQueue, &psTimerCBData->sWork);
	if (res == 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "OSTimerCallbackWrapper: work already queued"));
	}
}


static void OSTimerWorkQueueCallBack(struct work_struct *psWork)
{
	TIMER_CALLBACK_DATA *psTimerCBData = container_of(psWork, TIMER_CALLBACK_DATA, sWork);

	OSTimerCallbackBody(psTimerCBData);
}

IMG_HANDLE OSAddTimer(PFN_TIMER_FUNC pfnTimerFunc, void *pvData, IMG_UINT32 ui32MsTimeout)
{
	TIMER_CALLBACK_DATA *psTimerCBData;
	IMG_UINT32		ui32i;

	/* check callback */
	if (!pfnTimerFunc)
	{
		PVR_DPF((PVR_DBG_ERROR, "OSAddTimer: passed invalid callback"));
		return NULL;
	}

	/* Allocate timer callback data structure */
	mutex_lock(&sTimerStructLock);
	for (ui32i = 0; ui32i < OS_MAX_TIMERS; ui32i++)
	{
		psTimerCBData = &sTimers[ui32i];
		if (!psTimerCBData->bInUse)
		{
			psTimerCBData->bInUse = IMG_TRUE;
			break;
		}
	}
	mutex_unlock(&sTimerStructLock);
	if (ui32i >= OS_MAX_TIMERS)
	{
		PVR_DPF((PVR_DBG_ERROR, "OSAddTimer: all timers are in use"));
		return NULL;
	}

	psTimerCBData->pfnTimerFunc = pfnTimerFunc;
	psTimerCBData->pvData = pvData;
	psTimerCBData->bActive = IMG_FALSE;

	/*
		HZ = ticks per second
		ui32MsTimeout = required ms delay
		ticks = (Hz * ui32MsTimeout) / 1000
	*/
	psTimerCBData->ui32Delay = ((HZ * ui32MsTimeout) < 1000)
								?	1
								:	((HZ * ui32MsTimeout) / 1000);

	/* initialise object */
#if defined(from_timer)
	timer_setup(&psTimerCBData->sTimer, OSTimerCallbackWrapper, 0);
#else
	init_timer(&psTimerCBData->sTimer);

	/* setup timer object */
	psTimerCBData->sTimer.function = (void *)OSTimerCallbackWrapper;
	psTimerCBData->sTimer.data = (uintptr_t)psTimerCBData;
#endif

	return (IMG_HANDLE)(uintptr_t)(ui32i + 1);
}


static inline TIMER_CALLBACK_DATA *GetTimerStructure(IMG_HANDLE hTimer)
{
	IMG_UINT32 ui32i = (IMG_UINT32)((uintptr_t)hTimer) - 1;

	PVR_ASSERT(ui32i < OS_MAX_TIMERS);

	return &sTimers[ui32i];
}

PVRSRV_ERROR OSRemoveTimer (IMG_HANDLE hTimer)
{
	TIMER_CALLBACK_DATA *psTimerCBData = GetTimerStructure(hTimer);

	PVR_ASSERT(psTimerCBData->bInUse);
	PVR_ASSERT(!psTimerCBData->bActive);

	/* free timer callback data struct */
	psTimerCBData->bInUse = IMG_FALSE;

	return PVRSRV_OK;
}

PVRSRV_ERROR OSEnableTimer (IMG_HANDLE hTimer)
{
	TIMER_CALLBACK_DATA *psTimerCBData = GetTimerStructure(hTimer);

	PVR_ASSERT(psTimerCBData->bInUse);
	PVR_ASSERT(!psTimerCBData->bActive);

	/* Start timer arming */
	psTimerCBData->bActive = IMG_TRUE;

	/* set the expire time */
	psTimerCBData->sTimer.expires = psTimerCBData->ui32Delay + jiffies;

	/* Add the timer to the list */
	add_timer(&psTimerCBData->sTimer);

	return PVRSRV_OK;
}


PVRSRV_ERROR OSDisableTimer (IMG_HANDLE hTimer)
{
	TIMER_CALLBACK_DATA *psTimerCBData = GetTimerStructure(hTimer);

	PVR_ASSERT(psTimerCBData->bInUse);
	PVR_ASSERT(psTimerCBData->bActive);

	/* Stop timer from arming */
	psTimerCBData->bActive = IMG_FALSE;
	smp_mb();

	flush_workqueue(psTimerWorkQueue);

	/* remove timer */
	del_timer_sync(&psTimerCBData->sTimer);

	/*
	 * This second flush is to catch the case where the timer ran
	 * before we managed to delete it, in which case, it will have
	 * queued more work for the workqueue. Since the bActive flag
	 * has been cleared, this second flush won't result in the
	 * timer being rearmed.
	 */
	flush_workqueue(psTimerWorkQueue);

	return PVRSRV_OK;
}


PVRSRV_ERROR OSEventObjectCreate(const IMG_CHAR *pszName, IMG_HANDLE *hEventObject)
{
	PVR_UNREFERENCED_PARAMETER(pszName);

	PVR_LOG_RETURN_IF_INVALID_PARAM(hEventObject, "hEventObject");

	return LinuxEventObjectListCreate(hEventObject);
}


PVRSRV_ERROR OSEventObjectDestroy(IMG_HANDLE hEventObject)
{
	PVR_LOG_RETURN_IF_INVALID_PARAM(hEventObject, "hEventObject");

	return LinuxEventObjectListDestroy(hEventObject);
}

#define _FREEZABLE IMG_TRUE
#define _NON_FREEZABLE IMG_FALSE

/*
 * EventObjectWaitTimeout()
 */
static PVRSRV_ERROR EventObjectWaitTimeout(IMG_HANDLE hOSEventKM,
										   IMG_UINT64 uiTimeoutus)
{
	PVRSRV_ERROR eError;

	if (hOSEventKM && uiTimeoutus > 0)
	{
		eError = LinuxEventObjectWait(hOSEventKM, uiTimeoutus, _NON_FREEZABLE);
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR, "OSEventObjectWait: invalid arguments %p, %lld", hOSEventKM, uiTimeoutus));
		eError = PVRSRV_ERROR_INVALID_PARAMS;
	}

	return eError;
}

PVRSRV_ERROR OSEventObjectWaitTimeout(IMG_HANDLE hOSEventKM, IMG_UINT64 uiTimeoutus)
{
	return EventObjectWaitTimeout(hOSEventKM, uiTimeoutus);
}

PVRSRV_ERROR OSEventObjectWait(IMG_HANDLE hOSEventKM)
{
	return OSEventObjectWaitTimeout(hOSEventKM, EVENT_OBJECT_TIMEOUT_US);
}

PVRSRV_ERROR OSEventObjectWaitKernel(IMG_HANDLE hOSEventKM,
                                     IMG_UINT64 uiTimeoutus)
{
	PVRSRV_ERROR eError;

#if defined(PVRSRV_SERVER_THREADS_INDEFINITE_SLEEP)
	if (hOSEventKM)
	{
		if (uiTimeoutus > 0)
			eError = LinuxEventObjectWait(hOSEventKM, uiTimeoutus,
			                              _FREEZABLE);
		else
			eError = LinuxEventObjectWaitUntilSignalled(hOSEventKM);
	}
#else /* defined(PVRSRV_SERVER_THREADS_INDEFINITE_SLEEP) */
	if (hOSEventKM && uiTimeoutus > 0)
	{
		eError = LinuxEventObjectWait(hOSEventKM, uiTimeoutus,
		                              _FREEZABLE);
	}
#endif /* defined(PVRSRV_SERVER_THREADS_INDEFINITE_SLEEP) */
	else
	{
		PVR_DPF((PVR_DBG_ERROR, "OSEventObjectWaitKernel: invalid arguments %p",
		        hOSEventKM));
		eError = PVRSRV_ERROR_INVALID_PARAMS;
	}

	return eError;
}

void OSEventObjectDumpDebugInfo(IMG_HANDLE hOSEventKM)
{
	LinuxEventObjectDumpDebugInfo(hOSEventKM);
}

PVRSRV_ERROR OSEventObjectOpen(IMG_HANDLE hEventObject, IMG_HANDLE *phOSEvent)
{
	PVRSRV_ERROR eError;

	PVR_LOG_RETURN_IF_INVALID_PARAM(phOSEvent, "phOSEvent");
	PVR_LOG_GOTO_IF_INVALID_PARAM(hEventObject, eError, error);

	eError = LinuxEventObjectAdd(hEventObject, phOSEvent);
	PVR_LOG_GOTO_IF_ERROR(eError, "LinuxEventObjectAdd", error);

	return PVRSRV_OK;

error:
	*phOSEvent = NULL;
	return eError;
}

PVRSRV_ERROR OSEventObjectClose(IMG_HANDLE hOSEventKM)
{
	PVR_LOG_RETURN_IF_INVALID_PARAM(hOSEventKM, "hOSEventKM");

	return LinuxEventObjectDelete(hOSEventKM);
}

PVRSRV_ERROR OSEventObjectSignal(IMG_HANDLE hEventObject)
{
	PVR_LOG_RETURN_IF_INVALID_PARAM(hEventObject, "hEventObject");

	return LinuxEventObjectSignal(hEventObject);
}

PVRSRV_ERROR OSCopyToUser(void *pvProcess,
						  void __user *pvDest,
						  const void *pvSrc,
						  size_t ui32Bytes)
{
	PVR_UNREFERENCED_PARAMETER(pvProcess);

	if (pvr_copy_to_user(pvDest, pvSrc, ui32Bytes)==0)
		return PVRSRV_OK;
	else
		return PVRSRV_ERROR_FAILED_TO_COPY_VIRT_MEMORY;
}

PVRSRV_ERROR OSCopyFromUser(void *pvProcess,
							void *pvDest,
							const void __user *pvSrc,
							size_t ui32Bytes)
{
	PVR_UNREFERENCED_PARAMETER(pvProcess);

	if (likely(pvr_copy_from_user(pvDest, pvSrc, ui32Bytes)==0))
		return PVRSRV_OK;
	else
		return PVRSRV_ERROR_FAILED_TO_COPY_VIRT_MEMORY;
}

IMG_UINT64 OSDivide64r64(IMG_UINT64 ui64Divident, IMG_UINT32 ui32Divisor, IMG_UINT32 *pui32Remainder)
{
	*pui32Remainder = do_div(ui64Divident, ui32Divisor);

	return ui64Divident;
}

IMG_UINT32 OSDivide64(IMG_UINT64 ui64Divident, IMG_UINT32 ui32Divisor, IMG_UINT32 *pui32Remainder)
{
	*pui32Remainder = do_div(ui64Divident, ui32Divisor);

	return (IMG_UINT32) ui64Divident;
}

/* One time osfunc initialisation */
PVRSRV_ERROR PVROSFuncInit(void)
{
	{
		PVR_ASSERT(!psTimerWorkQueue);

		psTimerWorkQueue = create_freezable_workqueue("pvr_timer");
		if (psTimerWorkQueue == NULL)
		{
			PVR_DPF((PVR_DBG_ERROR, "%s: couldn't create timer workqueue",
					 __func__));
			return PVRSRV_ERROR_UNABLE_TO_CREATE_THREAD;
		}
	}

	{
		IMG_UINT32 ui32i;

		for (ui32i = 0; ui32i < OS_MAX_TIMERS; ui32i++)
		{
			TIMER_CALLBACK_DATA *psTimerCBData = &sTimers[ui32i];

			INIT_WORK(&psTimerCBData->sWork, OSTimerWorkQueueCallBack);
		}
	}
	return PVRSRV_OK;
}

/*
 * Osfunc deinitialisation.
 * Note that PVROSFuncInit may not have been called
 */
void PVROSFuncDeInit(void)
{
	if (psTimerWorkQueue != NULL)
	{
		destroy_workqueue(psTimerWorkQueue);
		psTimerWorkQueue = NULL;
	}
}

void OSDumpStack(void)
{
	dump_stack();
}

PVRSRV_ERROR OSChangeSparseMemCPUAddrMap(void **psPageArray,
                                         IMG_UINT64 sCpuVAddrBase,
                                         IMG_CPU_PHYADDR sCpuPAHeapBase,
                                         IMG_UINT32 ui32AllocPageCount,
                                         IMG_UINT32 *pai32AllocIndices,
                                         IMG_UINT32 ui32FreePageCount,
                                         IMG_UINT32 *pai32FreeIndices,
                                         IMG_BOOL bIsLMA)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
	pfn_t sPFN;
#else
	IMG_UINT64 uiPFN;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */

	PVRSRV_ERROR eError;

	struct mm_struct *psMM = current->mm;
	struct vm_area_struct *psVMA = NULL;
	struct address_space *psMapping = NULL;
	struct page *psPage = NULL;

	IMG_UINT64 uiCPUVirtAddr = 0;
	IMG_UINT32 ui32Loop = 0;
	IMG_UINT32 ui32PageSize = OSGetPageSize();
	IMG_BOOL bMixedMap = IMG_FALSE;

	/*
	 * Acquire the lock before manipulating the VMA
	 * In this case only mmap_sem lock would suffice as the pages associated with this VMA
	 * are never meant to be swapped out.
	 *
	 * In the future, in case the pages are marked as swapped, page_table_lock needs
	 * to be acquired in conjunction with this to disable page swapping.
	 */

	/* Find the Virtual Memory Area associated with the user base address */
	psVMA = find_vma(psMM, (uintptr_t)sCpuVAddrBase);
	if (NULL == psVMA)
	{
		eError = PVRSRV_ERROR_PMR_NO_CPU_MAP_FOUND;
		return eError;
	}

	/* Acquire the memory sem */
	mmap_write_lock(psMM);

	psMapping = psVMA->vm_file->f_mapping;

	/* Set the page offset to the correct value as this is disturbed in MMAP_PMR func */
	psVMA->vm_pgoff = (psVMA->vm_start >>  PAGE_SHIFT);

	/* Delete the entries for the pages that got freed */
	if (ui32FreePageCount && (pai32FreeIndices != NULL))
	{
		for (ui32Loop = 0; ui32Loop < ui32FreePageCount; ui32Loop++)
		{
			uiCPUVirtAddr = (uintptr_t)(sCpuVAddrBase + (pai32FreeIndices[ui32Loop] * ui32PageSize));

			unmap_mapping_range(psMapping, uiCPUVirtAddr, ui32PageSize, 1);

#ifndef PVRSRV_UNMAP_ON_SPARSE_CHANGE
			/*
			 * Still need to map pages in case remap flag is set.
			 * That is not done until the remap case succeeds
			 */
#endif
		}
		eError = PVRSRV_OK;
	}

	if ((psVMA->vm_flags & VM_MIXEDMAP) || bIsLMA)
	{
		OSSetVMAFlags(psVMA, VM_MIXEDMAP);
		bMixedMap = IMG_TRUE;
	}
	else
	{
		if (ui32AllocPageCount && (NULL != pai32AllocIndices))
		{
			for (ui32Loop = 0; ui32Loop < ui32AllocPageCount; ui32Loop++)
			{

				psPage = (struct page *)psPageArray[pai32AllocIndices[ui32Loop]];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
				sPFN = page_to_pfn_t(psPage);

				if (!pfn_t_valid(sPFN) || page_count(pfn_t_to_page(sPFN)) == 0)
#else
				uiPFN = page_to_pfn(psPage);

				if (!pfn_valid(uiPFN) || (page_count(pfn_to_page(uiPFN)) == 0))
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */
				{
					OSSetVMAFlags(psVMA, VM_MIXEDMAP);
					break;
				}
			}
		}
	}

	/* Map the pages that got allocated */
	if (ui32AllocPageCount && (NULL != pai32AllocIndices))
	{
		for (ui32Loop = 0; ui32Loop < ui32AllocPageCount; ui32Loop++)
		{
			int err;

			uiCPUVirtAddr = (uintptr_t)(sCpuVAddrBase + (pai32AllocIndices[ui32Loop] * ui32PageSize));
			unmap_mapping_range(psMapping, uiCPUVirtAddr, ui32PageSize, 1);

			if (bIsLMA)
			{
				phys_addr_t uiAddr = sCpuPAHeapBase.uiAddr +
				                     ((IMG_DEV_PHYADDR *)psPageArray)[pai32AllocIndices[ui32Loop]].uiAddr;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
				sPFN = phys_to_pfn_t(uiAddr, 0);
				psPage = pfn_t_to_page(sPFN);
#else
				uiPFN = uiAddr >> PAGE_SHIFT;
				psPage = pfn_to_page(uiPFN);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */
			}
			else
			{
				psPage = (struct page *)psPageArray[pai32AllocIndices[ui32Loop]];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
				sPFN = page_to_pfn_t(psPage);
#else
				uiPFN = page_to_pfn(psPage);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */
			}

			if (bMixedMap)
			{
#if defined(OS_FUNC_VMF_INSERT_MIXED_EXIST)
				vm_fault_t vmf;

				vmf = vmf_insert_mixed(psVMA, uiCPUVirtAddr, sPFN);
				if (vmf & VM_FAULT_ERROR)
				{
					err = vm_fault_to_errno(vmf, 0);
				}
				else
				{
					err = 0;
				}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
				err = vm_insert_mixed(psVMA, uiCPUVirtAddr, sPFN);
#else
				err = vm_insert_mixed(psVMA, uiCPUVirtAddr, uiPFN);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */
			}
			else
			{
				err = vm_insert_page(psVMA, uiCPUVirtAddr, psPage);
			}

			if (err)
			{
				PVR_DPF((PVR_DBG_MESSAGE, "Remap failure error code: %d", err));
				eError = PVRSRV_ERROR_PMR_CPU_PAGE_MAP_FAILED;
				goto eFailed;
			}
		}
	}

	eError = PVRSRV_OK;
eFailed:
	mmap_write_unlock(psMM);

	return eError;
}

/*************************************************************************/ /*!
@Function       OSDebugSignalPID
@Description    Sends a SIGTRAP signal to a specific PID in user mode for
                debugging purposes. The user mode process can register a handler
                against this signal.
                This is necessary to support the Rogue debugger. If the Rogue
                debugger is not used then this function may be implemented as
                a stub.
@Input          ui32PID    The PID for the signal.
@Return         PVRSRV_OK on success, a failure code otherwise.
*/ /**************************************************************************/
PVRSRV_ERROR OSDebugSignalPID(IMG_UINT32 ui32PID)
{
	int err;
	struct pid *psPID;

	psPID = find_vpid(ui32PID);
	if (psPID == NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Failed to get PID struct.", __func__));
		return PVRSRV_ERROR_NOT_FOUND;
	}

	err = kill_pid(psPID, SIGTRAP, 0);
	if (err != 0)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Signal Failure %d", __func__, err));
		return PVRSRV_ERROR_SIGNAL_FAILED;
	}

	return PVRSRV_OK;
}

/*************************************************************************/ /*!
@Function       OSIsKernelThread
@Description    This API determines if the current running thread is a kernel
                thread (i.e. one not associated with any userland process,
                typically an MISR handler.)
@Return         IMG_TRUE if it is a kernel thread, otherwise IMG_FALSE.
*/ /**************************************************************************/
IMG_BOOL OSIsKernelThread(void)
{
	/*
	 * Kernel threads have a NULL memory descriptor.
	 *
	 * See https://www.kernel.org/doc/Documentation/vm/active_mm.txt
	 */
	return current->mm == NULL;
}

void OSDumpVersionInfo(DUMPDEBUG_PRINTF_FUNC *pfnDumpDebugPrintf,
					   void *pvDumpDebugFile)
{
	PVR_DUMPDEBUG_LOG("OS kernel info: %s %s %s %s",
					utsname()->sysname,
					utsname()->release,
					utsname()->version,
					utsname()->machine);
}
#if defined(SUPPORT_DMA_TRANSFER)

typedef struct _OS_CLEANUP_DATA_
{
	IMG_BOOL bSucceed;
	IMG_BOOL bAdvanceTimeline;
	IMG_UINT uiRefCount;
	IMG_UINT uiNumDMA;
	IMG_UINT uiCount;

	struct dma_async_tx_descriptor** ppsDescriptors;


	PVRSRV_DEVICE_NODE *psDevNode;
	PFN_SERVER_CLEANUP pfnServerCleanup;
	void* pvServerCleanupData;

	enum dma_transfer_direction eDirection;
	struct sg_table **ppsSg;
	struct page ***pages;
	IMG_UINT32* puiNumPages;
	spinlock_t spinlock;

	struct completion start_cleanup;
	struct completion *sync_completion;

	/* Sparse PMR transfer information */
	IMG_BOOL *pbIsSparse;
	IMG_UINT *uiNumValidPages;
	struct sg_table ***ppsSgSparse;
	struct dma_async_tx_descriptor*** ppsDescriptorsSparse;

} OS_CLEANUP_DATA;

static int cleanup_thread(void *pvData)
{
	IMG_UINT32 i, j;
	struct completion *sync_completion = NULL;
	OS_CLEANUP_DATA *psOSCleanup = (OS_CLEANUP_DATA*)pvData;
	IMG_BOOL bSucceed = psOSCleanup->bSucceed;

	sync_completion = psOSCleanup->sync_completion;

#if defined(DMA_VERBOSE)
	PVR_DPF((PVR_DBG_ERROR, "Cleanup thread waiting (%p) on completion", pvData));
#endif

	wait_for_completion(&psOSCleanup->start_cleanup);

#if defined(DMA_VERBOSE)
	PVR_DPF((PVR_DBG_ERROR, "Cleanup thread notified (%p)", pvData));
#endif
	/* Free resources */
	for (i=0; i<psOSCleanup->uiCount; i++)
	{
		if (!psOSCleanup->pbIsSparse[i])
		{
			dma_sync_sg_for_cpu(psOSCleanup->psDevNode->psDevConfig->pvOSDevice,
					    psOSCleanup->ppsSg[i]->sgl,
					    psOSCleanup->ppsSg[i]->nents,
					    (enum dma_data_direction)psOSCleanup->eDirection);

			dma_unmap_sg(psOSCleanup->psDevNode->psDevConfig->pvOSDevice,
				     psOSCleanup->ppsSg[i]->sgl,
				     psOSCleanup->ppsSg[i]->nents,
				     (enum dma_data_direction)psOSCleanup->eDirection);

			sg_free_table(psOSCleanup->ppsSg[i]);

			OSFreeMem(psOSCleanup->ppsSg[i]);

			/* Unpin pages */
			for (j=0; j<psOSCleanup->puiNumPages[i]; j++)
			{
				if (psOSCleanup->eDirection == DMA_DEV_TO_MEM)
				{
					set_page_dirty_lock(psOSCleanup->pages[i][j]);
				}
				put_page(psOSCleanup->pages[i][j]);
			}
		}
		else
		{
			for (j = 0; j < psOSCleanup->puiNumPages[i]; j++)
			{
				if (psOSCleanup->ppsSgSparse[i][j]) {
					dma_sync_sg_for_cpu(psOSCleanup->psDevNode->psDevConfig->pvOSDevice,
							psOSCleanup->ppsSgSparse[i][j]->sgl,
							psOSCleanup->ppsSgSparse[i][j]->nents,
						(enum dma_data_direction)psOSCleanup->eDirection);


					dma_unmap_sg(psOSCleanup->psDevNode->psDevConfig->pvOSDevice,
							psOSCleanup->ppsSgSparse[i][j]->sgl,
							psOSCleanup->ppsSgSparse[i][j]->nents,
						(enum dma_data_direction)psOSCleanup->eDirection);

					sg_free_table(psOSCleanup->ppsSgSparse[i][j]);

					OSFreeMem(psOSCleanup->ppsSgSparse[i][j]);

				}
			}

			OSFreeMem(psOSCleanup->ppsSgSparse[i]);
			OSFreeMem(psOSCleanup->ppsDescriptorsSparse[i]);

			/* Unpin pages */
			for (j=0; j<psOSCleanup->puiNumPages[i]*2; j++)
			{
				/*
				 * Some pages might've been pinned twice
				 * Others may have not been pinned at all
				 */
				if (psOSCleanup->pages[i][j])
				{
					if (psOSCleanup->eDirection == DMA_DEV_TO_MEM)
					{
						set_page_dirty_lock(psOSCleanup->pages[i][j]);
					}
					put_page(psOSCleanup->pages[i][j]);
				}
			}
		}

		OSFreeMem(psOSCleanup->pages[i]);
	}

	psOSCleanup->pfnServerCleanup(psOSCleanup->pvServerCleanupData,
								  psOSCleanup->bAdvanceTimeline);

	OSFreeMem(psOSCleanup->ppsSg);
	OSFreeMem(psOSCleanup->pages);
	OSFreeMem(psOSCleanup->puiNumPages);
	OSFreeMem(psOSCleanup->ppsSgSparse);
	OSFreeMem(psOSCleanup->ppsDescriptorsSparse);
	OSFreeMem(psOSCleanup->ppsDescriptors);
	OSFreeMem(psOSCleanup->pbIsSparse);
	OSFreeMem(psOSCleanup->uiNumValidPages);
	OSFreeMem(psOSCleanup);

	if (sync_completion && bSucceed)
	{
		complete(sync_completion);
	}

	return 0;
}

static void dma_callback(void *pvOSCleanup)
{
	OS_CLEANUP_DATA *psOSCleanup = (OS_CLEANUP_DATA*)pvOSCleanup;
	unsigned long flags;

#if defined(DMA_VERBOSE)
	PVR_DPF((PVR_DBG_ERROR, "dma_callback (%p) refcount decreased to %d", psOSCleanup, psOSCleanup->uiRefCount - 1));
#endif
	spin_lock_irqsave(&psOSCleanup->spinlock, flags);

	psOSCleanup->uiRefCount--;

	if (psOSCleanup->uiRefCount==0)
	{
		/* Notify the cleanup thread */
		spin_unlock_irqrestore(&psOSCleanup->spinlock, flags);
		complete(&psOSCleanup->start_cleanup);
		return;
	}

	spin_unlock_irqrestore(&psOSCleanup->spinlock, flags);
}

#if defined(SUPPORT_VALIDATION) && defined(PVRSRV_DEBUG_DMA)
static void
DMADumpPhysicalAddresses(struct page **ppsHostMemPages,
						 IMG_UINT32 uiNumPages,
						 IMG_DMA_ADDR *sDmaAddr,
						 IMG_UINT64 ui64Offset)
{
	IMG_CPU_PHYADDR sPagePhysAddr;
	IMG_UINT32 uiIdx;

	PVR_DPF((PVR_DBG_MESSAGE, "DMA Transfer Address Dump"));
	PVR_DPF((PVR_DBG_MESSAGE, "Hostmem phys addresses:"));

	for (uiIdx = 0; uiIdx < uiNumPages; uiIdx++)
	{
		sPagePhysAddr.uiAddr = page_to_phys(ppsHostMemPages[uiIdx]);
		if (uiIdx == 0)
		{
			sPagePhysAddr.uiAddr += ui64Offset;
			PVR_DPF((PVR_DBG_MESSAGE, "\tHost mem start at 0x%llX", sPagePhysAddr.uiAddr));
		}
		else
		{
			PVR_DPF((PVR_DBG_MESSAGE, "\tHost Mem Page %d at 0x%llX", uiIdx,
					 sPagePhysAddr.uiAddr));
		}
	}
	PVR_DPF((PVR_DBG_MESSAGE, "Devmem CPU phys address: 0x%llX",
			 sDmaAddr->uiAddr));
}
#endif

PVRSRV_ERROR OSDmaSubmitTransfer(PVRSRV_DEVICE_NODE *psDevNode, void *pvOSData,
				    void *pvChan, IMG_BOOL bSynchronous)
{
	OS_CLEANUP_DATA *psOSCleanup = (OS_CLEANUP_DATA*)pvOSData;
	struct completion* sync_completion = NULL;

	psOSCleanup->bSucceed = IMG_TRUE;
	psOSCleanup->bAdvanceTimeline = IMG_TRUE;

	if (bSynchronous)
	{
		sync_completion = OSAllocZMem(sizeof(struct completion));
		init_completion(sync_completion);
	}

	PVR_UNREFERENCED_PARAMETER(psDevNode);
	/* Wait only on number of ops scheduled. This might be different to NumDMAs
	in certain error conditions */
	psOSCleanup->uiRefCount = psOSCleanup->uiCount;
	psOSCleanup->sync_completion = sync_completion;

	{
		IMG_UINT32 i,j;
		for (i=0; i<psOSCleanup->uiCount; i++)
		{
			if (psOSCleanup->pbIsSparse[i])
			{
				for (j=0; j<psOSCleanup->puiNumPages[i]; j++)
				{
					if (psOSCleanup->ppsDescriptorsSparse[i][j])
						dmaengine_submit(psOSCleanup->ppsDescriptorsSparse[i][j]);
				}
			}
			else
			{
				dmaengine_submit(psOSCleanup->ppsDescriptors[i]);
			}
		}
	}

	dma_async_issue_pending(pvChan);

	if (bSynchronous)
	{
		wait_for_completion(sync_completion);
		OSFreeMem(sync_completion);
	}

	return PVRSRV_OK;
}

void OSDmaForceCleanup(PVRSRV_DEVICE_NODE *psDevNode, void *pvChan,
					   void *pvOSData, void *pvServerCleanupParam,
					   PFN_SERVER_CLEANUP pfnServerCleanup)
{
	OS_CLEANUP_DATA *psOSCleanup = (OS_CLEANUP_DATA *)pvOSData;
	IMG_UINT ui32Retries;

	PVR_UNREFERENCED_PARAMETER(psDevNode);

	psOSCleanup->bSucceed = IMG_FALSE;
	psOSCleanup->bAdvanceTimeline = IMG_TRUE;

	/* Need to wait for outstanding DMA Engine ops before advancing the
	   user-supplied timeline in case of error. dmaengine_terminate_sync
	   cannot be called from within atomic context, so cannot invoke it
	   from inside the cleanup kernel thread. */
	for (ui32Retries = 0; ui32Retries < DMA_ERROR_SYNC_RETRIES; ui32Retries++)
	{
		if (dmaengine_terminate_sync(pvChan) == 0)
		{
			break;
		}
	}
	if (ui32Retries == DMA_ERROR_SYNC_RETRIES)
	{
		/* We cannot guarantee all outstanding DMAs were terminated
		 * so we let the UM fence time out as a fallback mechanism */
		psOSCleanup->bAdvanceTimeline = IMG_FALSE;
	}

	if (psOSCleanup->uiCount > 0)
	{
		complete(&psOSCleanup->start_cleanup);
	}
	else
	{
		/* Cleanup kthread never run, need to manually wind down */
		pfnServerCleanup(pvServerCleanupParam, psOSCleanup->bAdvanceTimeline);

		OSFreeMem(psOSCleanup->ppsSg);
		OSFreeMem(psOSCleanup->pages);
		OSFreeMem(psOSCleanup->puiNumPages);
		OSFreeMem(psOSCleanup->ppsSgSparse);
		OSFreeMem(psOSCleanup->pbIsSparse);
		OSFreeMem(psOSCleanup->uiNumValidPages);
		OSFreeMem(psOSCleanup->ppsDescriptors);
		OSFreeMem(psOSCleanup->ppsDescriptorsSparse);

		OSFreeMem(psOSCleanup);
	}
}

PVRSRV_ERROR OSDmaAllocData(PVRSRV_DEVICE_NODE *psDevNode, IMG_UINT32 uiNumDMA, void **pvOutData)
{
	PVRSRV_ERROR eError;
	OS_CLEANUP_DATA *psOSCleanup = OSAllocZMem(sizeof(OS_CLEANUP_DATA));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup, eError, e0);

	psOSCleanup->uiNumDMA = uiNumDMA;
	psOSCleanup->psDevNode = psDevNode;

	spin_lock_init(&psOSCleanup->spinlock);

	init_completion(&psOSCleanup->start_cleanup);

	psOSCleanup->ppsDescriptors = OSAllocZMem(uiNumDMA * sizeof(struct dma_async_tx_descriptor*));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->ppsDescriptors, eError, e0);

	psOSCleanup->ppsDescriptorsSparse = OSAllocZMem(uiNumDMA * sizeof(struct dma_async_tx_descriptor*));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->ppsDescriptorsSparse, eError, e11);

	psOSCleanup->ppsSg = OSAllocZMem(uiNumDMA * sizeof(struct sg_table*));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->ppsSg, eError, e1);

	psOSCleanup->ppsSgSparse = OSAllocZMem(uiNumDMA * sizeof(struct sg_table*));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->ppsSgSparse, eError, e12);

	psOSCleanup->pbIsSparse = OSAllocZMem(uiNumDMA * sizeof(IMG_BOOL));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->pbIsSparse, eError, e13);

	psOSCleanup->uiNumValidPages = OSAllocZMem(uiNumDMA * sizeof(IMG_UINT));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->uiNumValidPages, eError, e14);

	psOSCleanup->pages = OSAllocZMem(uiNumDMA * sizeof(struct page **));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->pages, eError, e2);

	psOSCleanup->puiNumPages = OSAllocZMem(uiNumDMA * sizeof(IMG_UINT32));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanup->puiNumPages, eError, e3);

	*pvOutData = psOSCleanup;

	return PVRSRV_OK;

e3:
	OSFreeMem(psOSCleanup->pages);
e2:
	OSFreeMem(psOSCleanup->uiNumValidPages);
e14:
	OSFreeMem(psOSCleanup->pbIsSparse);
e13:
	OSFreeMem(psOSCleanup->ppsSgSparse);
e12:
	OSFreeMem(psOSCleanup->ppsSg);
e1:
	OSFreeMem(psOSCleanup->ppsDescriptorsSparse);
e11:
	OSFreeMem(psOSCleanup->ppsDescriptors);
e0:
	OSFreeMem(psOSCleanup);
	return eError;
}

/*************************************************************************/ /*!
@Function       OSDmaTransfer
@Description    This API is used to ask OS to perform a DMA transfer operation
@Return
*/ /**************************************************************************/
PVRSRV_ERROR OSDmaPrepareTransfer(PVRSRV_DEVICE_NODE *psDevNode,
							   void* pvChan,
							   IMG_DMA_ADDR* psDmaAddr, IMG_UINT64* puiAddress,
							   IMG_UINT64 uiSize, IMG_BOOL bMemToDev,
							   void* pvOSData,
							   void* pvServerCleanupParam, PFN_SERVER_CLEANUP pfnServerCleanup, IMG_BOOL bFirst)
{

	IMG_INT iRet;
	PVRSRV_ERROR eError;
	PVRSRV_DEVICE_CONFIG *psDevConfig = psDevNode->psDevConfig;
	OS_CLEANUP_DATA* psOSCleanupData = pvOSData;

	struct dma_slave_config sConfig = {0};
	struct dma_async_tx_descriptor *psDesc;

	unsigned long offset = (unsigned long)puiAddress & ((1 << PAGE_SHIFT) - 1);
	unsigned int num_pages = (uiSize + offset + PAGE_SIZE - 1) >> PAGE_SHIFT;
	int num_pinned_pages = 0;
	unsigned int gup_flags = 0;

	struct sg_table *psSg = OSAllocZMem(sizeof(struct sg_table));
	PVR_LOG_GOTO_IF_NOMEM(psSg, eError, e0);

	psOSCleanupData->pages[psOSCleanupData->uiCount] = OSAllocZMem(num_pages * sizeof(struct page *));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanupData->pages[psOSCleanupData->uiCount], eError, e1);

	gup_flags |= bMemToDev ? 0 : FOLL_WRITE;

	num_pinned_pages = get_user_pages_fast(
			(unsigned long)puiAddress,
			(int)num_pages,
			gup_flags,
			psOSCleanupData->pages[psOSCleanupData->uiCount]);
	if (num_pinned_pages != num_pages)
	{
		PVR_DPF((PVR_DBG_ERROR, "get_user_pages_fast failed: (%d - %u)", num_pinned_pages, num_pages));
		eError = PVRSRV_ERROR_OUT_OF_MEMORY;
		goto e2;
	}

#if defined(SUPPORT_VALIDATION) && defined(PVRSRV_DEBUG_DMA)
	DMADumpPhysicalAddresses(psOSCleanupData->pages[psOSCleanupData->uiCount],
							 num_pages, psDmaAddr, offset);
#endif

	psOSCleanupData->puiNumPages[psOSCleanupData->uiCount] = num_pinned_pages;

	if (sg_alloc_table_from_pages(psSg, psOSCleanupData->pages[psOSCleanupData->uiCount], num_pages, offset, uiSize, GFP_KERNEL) != 0)
	{
		eError = PVRSRV_ERROR_BAD_MAPPING;
		PVR_DPF((PVR_DBG_ERROR, "sg_alloc_table_from_pages failed"));
		goto e3;
	}

	if (bMemToDev)
	{
		sConfig.direction = DMA_MEM_TO_DEV;
		sConfig.src_addr = 0;
		sConfig.dst_addr = psDmaAddr->uiAddr;
	}
	else
	{
		sConfig.direction = DMA_DEV_TO_MEM;
		sConfig.src_addr = psDmaAddr->uiAddr;
		sConfig.dst_addr = 0;
	}
	dmaengine_slave_config(pvChan, &sConfig);

	iRet = dma_map_sg(psDevConfig->pvOSDevice, psSg->sgl, psSg->nents,
			  (enum dma_data_direction)sConfig.direction);
	if (!iRet)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Error mapping SG list", __func__));
		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto e4;
	}

	dma_sync_sg_for_device(psDevConfig->pvOSDevice, psSg->sgl, (unsigned int)iRet,
			       (enum dma_data_direction)sConfig.direction);

	psDesc = dmaengine_prep_slave_sg(pvChan, psSg->sgl, (unsigned int)iRet, sConfig.direction, 0);
	if (!psDesc)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: dmaengine_prep_slave_sg failed", __func__));
		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto e5;
	}

	psOSCleanupData->eDirection = sConfig.direction;
	psOSCleanupData->ppsSg[psOSCleanupData->uiCount] = psSg;
	psOSCleanupData->pfnServerCleanup = pfnServerCleanup;
	psOSCleanupData->pvServerCleanupData = pvServerCleanupParam;

	psDesc->callback_param = psOSCleanupData;
	psDesc->callback = dma_callback;

	if	(bFirst)
	{
		struct task_struct* t1;
		t1 = kthread_run(cleanup_thread, psOSCleanupData, "dma-cleanup-thread");
	}
	psOSCleanupData->ppsDescriptors[psOSCleanupData->uiCount] = psDesc;

	psOSCleanupData->uiCount++;

	return PVRSRV_OK;

e5:
	dma_unmap_sg(psDevConfig->pvOSDevice, psSg->sgl, psSg->nents,
		     (enum dma_data_direction)sConfig.direction);
e4:
	sg_free_table(psSg);
e3:
	{
		IMG_UINT32 i;
		/* Unpin pages */
		for (i=0; i<psOSCleanupData->puiNumPages[psOSCleanupData->uiCount]; i++)
		{
			put_page(psOSCleanupData->pages[psOSCleanupData->uiCount][i]);
		}
	}
e2:
	OSFreeMem(psOSCleanupData->pages[psOSCleanupData->uiCount]);
e1:
	OSFreeMem(psSg);
e0:
	return eError;
}

static IMG_UINT32
CalculateValidPages(IMG_BOOL *pbValid,
					IMG_UINT32 ui32SizeInPages)
{
	IMG_UINT32 ui32nValid;
	IMG_UINT32 ui32Idx;

	for (ui32Idx = 0, ui32nValid = 0; ui32Idx < ui32SizeInPages; ui32Idx++)
	{
		ui32nValid += pbValid[ui32Idx] ? 1 : 0;
	}

	return ui32nValid;
}

PVRSRV_ERROR OSDmaPrepareTransferSparse(PVRSRV_DEVICE_NODE *psDevNode,
										void* pvChan,
										IMG_DMA_ADDR* psDmaAddr,
										IMG_BOOL *pbValid,
										IMG_UINT64* puiAddress,
										IMG_UINT64 uiSize,
										IMG_UINT32 uiOffsetInFirstPMRPage,
										IMG_UINT32 ui32SizeInPages,
										IMG_BOOL bMemToDev,
										void* pvOSData,
										void* pvServerCleanupParam,
										PFN_SERVER_CLEANUP pfnServerCleanup,
										IMG_BOOL bFirst)
{

	IMG_INT iRet;
	PVRSRV_ERROR eError = PVRSRV_OK;
	PVRSRV_DEVICE_CONFIG *psDevConfig = psDevNode->psDevConfig;
	OS_CLEANUP_DATA* psOSCleanupData = pvOSData;
	IMG_UINT32 ui32PageSize = OSGetPageSize();
	void *pvNextAddress = puiAddress;
	IMG_UINT32 ui32Idx;
	IMG_INT32 i32Rwd;

	struct dma_slave_config sConfig = {0};
	struct dma_async_tx_descriptor *psDesc;

	unsigned long offset = (unsigned long)puiAddress & ((1 << PAGE_SHIFT) - 1);
	unsigned int num_pages = (uiSize + offset + PAGE_SIZE - 1) >> PAGE_SHIFT;
	unsigned int num_valid_pages = CalculateValidPages(pbValid, ui32SizeInPages);
	unsigned int num_pinned_pages = 0;
	unsigned int gup_flags = 0;
	unsigned int valid_idx;
	size_t transfer_size;
	struct page ** next_pages;
	struct sg_table *psSg;

	psOSCleanupData->uiNumValidPages[psOSCleanupData->uiCount] = num_valid_pages;
	psOSCleanupData->pbIsSparse[psOSCleanupData->uiCount] = IMG_TRUE;

	/*
	 * If an SG transfer from virtual memory to card memory goes over a page boundary in
	 * main memory, it'll span two different pages - therefore, total number of pages to
	 * keep track of should be twice as many as for a simple transfer. This twice-as-big
	 * allocation is also necessary because the same virtual memory page might be present
	 * in more than one SG DMA transfer, because of differences in first-page offset between
	 * the sparse device PMR and the virtual memory buffer.
	 */
	psOSCleanupData->pages[psOSCleanupData->uiCount] = OSAllocZMem(2*num_valid_pages * sizeof(struct page *));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanupData->pages[psOSCleanupData->uiCount], eError, e0);

	psOSCleanupData->ppsSgSparse[psOSCleanupData->uiCount] = OSAllocZMem(num_valid_pages * sizeof(struct sg_table *));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanupData->ppsSgSparse[psOSCleanupData->uiCount], eError, e1);

	psOSCleanupData->ppsDescriptorsSparse[psOSCleanupData->uiCount] = OSAllocZMem(num_valid_pages * sizeof(struct dma_async_tx_descriptor *));
	PVR_LOG_GOTO_IF_NOMEM(psOSCleanupData->ppsDescriptorsSparse[psOSCleanupData->uiCount], eError, e11);

	gup_flags |= bMemToDev ? 0 : FOLL_WRITE;

	for (ui32Idx = 0, valid_idx = 0; ui32Idx < ui32SizeInPages; ui32Idx++)
	{
		if (valid_idx == num_valid_pages)
		{
			break;
		}
		if (!pbValid[ui32Idx])
		{
			pvNextAddress += (ui32Idx == 0) ? ui32PageSize - uiOffsetInFirstPMRPage : ui32PageSize;
			continue;
		}

		/* Pick transfer size */
		if (ui32Idx == 0)
		{
			if (uiOffsetInFirstPMRPage + uiSize <= ui32PageSize)
			{
				PVR_ASSERT(num_valid_pages == 1);
				transfer_size = uiSize;
			}
			else
			{
				transfer_size = ui32PageSize - uiOffsetInFirstPMRPage;
			}
		}
		else
		{
			/* Last valid LMA page */
			if (valid_idx == num_valid_pages - 1)
			{
				transfer_size = ((uiOffsetInFirstPMRPage + uiSize - 1) % ui32PageSize) + 1;
			}
			else
			{
				transfer_size = ui32PageSize;
			}
		}

		if (((unsigned long long)pvNextAddress & (ui32PageSize - 1)) + transfer_size > ui32PageSize)
		{
			num_pages = 2;
		}
		else
		{
			num_pages = 1;
		}

		next_pages = psOSCleanupData->pages[psOSCleanupData->uiCount] + (valid_idx * 2);

		num_pinned_pages = get_user_pages_fast(
			(unsigned long)pvNextAddress,
			(int)num_pages,
			gup_flags,
			next_pages);
		if (num_pinned_pages != num_pages)
		{
			PVR_DPF((PVR_DBG_ERROR, "get_user_pages_fast for sparse failed: (%d - %u)", num_pinned_pages, num_pages));
			eError = PVRSRV_ERROR_OUT_OF_MEMORY;
			goto e2;
		}

#if defined(SUPPORT_VALIDATION) && defined(PVRSRV_DEBUG_DMA)
		DMADumpPhysicalAddresses(next_pages, num_pages,
								 &psDmaAddr[ui32Idx],
								 (unsigned long)pvNextAddress & (ui32PageSize - 1));
#endif

		psSg = OSAllocZMem(sizeof(struct sg_table));
		PVR_LOG_GOTO_IF_NOMEM(psSg, eError, e3);

		if (sg_alloc_table_from_pages(psSg, next_pages, num_pages,
									  (unsigned long)pvNextAddress & (ui32PageSize - 1),
									  transfer_size,
									  GFP_KERNEL) != 0)
		{
			eError = PVRSRV_ERROR_BAD_MAPPING;
			PVR_DPF((PVR_DBG_ERROR, "sg_alloc_table_from_pages failed"));
			goto e4;
		}

		pvNextAddress += transfer_size;

		if (bMemToDev)
		{
			sConfig.direction = DMA_MEM_TO_DEV;
			sConfig.src_addr = 0;
			sConfig.dst_addr = psDmaAddr[ui32Idx].uiAddr;
		}
		else
		{
			sConfig.direction = DMA_DEV_TO_MEM;
			sConfig.src_addr = psDmaAddr[ui32Idx].uiAddr;
			sConfig.dst_addr = 0;
		}
		dmaengine_slave_config(pvChan, &sConfig);

		iRet = dma_map_sg(psDevConfig->pvOSDevice, psSg->sgl, psSg->nents,
				  (enum dma_data_direction)sConfig.direction);
		if (!iRet)
		{
			PVR_DPF((PVR_DBG_ERROR, "%s: Error mapping SG list", __func__));
			eError = PVRSRV_ERROR_INVALID_PARAMS;
			goto e5;
		}
		dma_sync_sg_for_device(psDevConfig->pvOSDevice, psSg->sgl, (unsigned int)iRet,
				       (enum dma_data_direction)sConfig.direction);

		psDesc = dmaengine_prep_slave_sg(pvChan, psSg->sgl, (unsigned int)iRet, sConfig.direction, 0);
		if (!psDesc)
		{
			PVR_DPF((PVR_DBG_ERROR, "%s: dmaengine_prep_slave_sg failed", __func__));
			eError = PVRSRV_ERROR_INVALID_PARAMS;

		goto e6;
		}

		psOSCleanupData->ppsSgSparse[psOSCleanupData->uiCount][valid_idx] = psSg;
		psOSCleanupData->ppsDescriptorsSparse[psOSCleanupData->uiCount][valid_idx] = psDesc;
		psOSCleanupData->puiNumPages[psOSCleanupData->uiCount] = ++valid_idx;

		if (valid_idx == num_valid_pages)
		{
			psDesc->callback_param = psOSCleanupData;
			psDesc->callback = dma_callback;

			if (bFirst)
			{
				struct task_struct* t1;

				psOSCleanupData->eDirection = sConfig.direction;
				psOSCleanupData->pfnServerCleanup = pfnServerCleanup;
				psOSCleanupData->pvServerCleanupData = pvServerCleanupParam;

				t1 = kthread_run(cleanup_thread, psOSCleanupData, "dma-cleanup-thread");
			}

			psOSCleanupData->uiCount++;
		}

	}

	return PVRSRV_OK;

e6:
	dma_unmap_sg(psDevConfig->pvOSDevice, psSg->sgl, psSg->nents,
		     (enum dma_data_direction)sConfig.direction);
e5:
	sg_free_table(psSg);
e4:
	OSFreeMem(psSg);
e3:
	/* Unpin last */
	put_page(psOSCleanupData->pages[psOSCleanupData->uiCount][valid_idx]);
	if (psOSCleanupData->pages[psOSCleanupData->uiCount][valid_idx+1])
	{
		put_page(psOSCleanupData->pages[psOSCleanupData->uiCount][valid_idx+1]);
	}
e2:
	/* rewind */
	for (i32Rwd=valid_idx-1; i32Rwd >= 0; i32Rwd--)
	{
		IMG_UINT32 i;

		psSg = psOSCleanupData->ppsSgSparse[psOSCleanupData->uiCount][i32Rwd];
		dma_unmap_sg(psDevConfig->pvOSDevice, psSg->sgl, psSg->nents,
			     (enum dma_data_direction)sConfig.direction);
		sg_free_table(psSg);

		/* Unpin pages */
		for (i=0; i < psOSCleanupData->puiNumPages[psOSCleanupData->uiCount]*2; i++)
		{
			if (psOSCleanupData->pages[psOSCleanupData->uiCount][i])
			{
				put_page(psOSCleanupData->pages[psOSCleanupData->uiCount][i]);
			}
		}
	}
	OSFreeMem(psOSCleanupData->ppsDescriptorsSparse[psOSCleanupData->uiCount]);
e11:
	OSFreeMem(psOSCleanupData->ppsSgSparse[psOSCleanupData->uiCount]);
e1:
	OSFreeMem(psOSCleanupData->pages[psOSCleanupData->uiCount]);
e0:
	return eError;
}

#endif /* SUPPORT_DMA_TRANSFER */

IMG_UINT8 OSReadByte(const void __iomem *pvAddr)
{
	return readb(pvAddr);
}

IMG_UINT16 OSReadWord(const void __iomem *pvAddr)
{
	return readw(pvAddr);
}

IMG_UINT32 OSReadDWord(const void __iomem *pvAddr)
{
	return readl(pvAddr);
}

void OSWriteByte(IMG_UINT8 ui8Val, void __iomem *pvAddr)
{
	return writeb(ui8Val, pvAddr);
}

void OSWriteWord(IMG_UINT16 ui16Val, void __iomem *pvAddr)
{
	return writew(ui16Val, pvAddr);
}

void OSWriteDWord(IMG_UINT32 ui32Val, void __iomem *pvAddr)
{
	return writel(ui32Val, pvAddr);
}

#if !defined(__linux__) || defined(CACHEFLUSH_NO_KMRBF_USING_UMVA)
void __user *OSValidateAndGetCPUUserVA(PMR *psPMR,
				       IMG_CPU_VIRTADDR pvAddress,
				       IMG_DEVMEM_OFFSET_T uiOffset,
				       IMG_DEVMEM_SIZE_T uiSize)
{
	return NULL;
}
#else
void __user *OSValidateAndGetCPUUserVA(PMR *psPMR,
				       IMG_CPU_VIRTADDR pvAddress,
				       IMG_DEVMEM_OFFSET_T uiOffset,
				       IMG_DEVMEM_SIZE_T uiSize)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	void __user *pvAddr;

	/* Validate VA, assume most basic address limit access_ok() check */
	pvAddr = (void __user *)(uintptr_t)((uintptr_t)pvAddress + uiOffset);
	if (!access_ok(pvAddr, uiSize)) {
		return NULL;
	} else if (mm) {
		mmap_read_lock(mm);
		vma = find_vma(mm, (unsigned long)(uintptr_t)pvAddr);

		if (!vma ||
		    vma->vm_start > (unsigned long)(uintptr_t)pvAddr ||
		    vma->vm_end < (unsigned long)(uintptr_t)pvAddr + uiSize ||
		    vma->vm_private_data != psPMR) {
			/*
			 * Request range is not fully mapped or is not matching the PMR
			 * Ignore request's VA.
			 */
			pvAddr = NULL;
		}
		mmap_read_unlock(mm);
	}

	return pvAddr;
}
#endif

/* round the given value down to nearest power of two */
IMG_INT OSRoundDownPowOfTwo(IMG_UINT32 n)
{
	return rounddown_pow_of_two(n);
}

struct device *OSGetPcieDeviceFromDeviceNode(PVRSRV_DEVICE_NODE *psDevNode)
{
	struct device *osDevice;

	PVR_ASSERT(psDevNode);
	PVR_ASSERT(psDevNode->psDevConfig);
	osDevice = (struct device *)psDevNode->psDevConfig->pvOSDevice;
	return osDevice->parent->parent;
}

bool OSDeviceIsPCI(struct device *psDev)
{
	return dev_is_pci(psDev);
}

bool OSDevToNode(struct device *psDev)
{
	return dev_to_node(psDev);
}

struct device *OSGetOSDeviceFromDeviceNode(PVRSRV_DEVICE_NODE *psDevNode)
{
	PVR_ASSERT(psDevNode);
	PVR_ASSERT(psDevNode->psDevConfig);
	return (struct device *)psDevNode->psDevConfig->pvOSDevice;
}

IMG_CPU_VIRTADDR OSDmaAllocCoherent(struct device *pvOSDevice,
				    size_t uiSize,
				    dma_addr_t *pHandle)
{
	return dma_alloc_coherent(pvOSDevice, uiSize, pHandle, GFP_KERNEL);
}

void OSDmaFreeCoherent(struct device *pvOSDevice,
		       size_t uiSize,
		       IMG_CPU_VIRTADDR pvVirtAddr,
		       dma_addr_t Handle)
{
	return dma_free_coherent(pvOSDevice, uiSize, pvVirtAddr, Handle);
}

IMG_PID HostPid2ContainerPID(IMG_PID hostPid)
{
	struct pid *cur_task_pid;
	pid_t converted_pid;
	struct task_struct *cur_task;
	struct pid_namespace *cur_pid_ns;

	cur_task_pid = get_task_pid(current, PIDTYPE_PID);
	if (!cur_task_pid) {
		PVR_DPF((PVR_DBG_WARNING, "Get current task_pid failed, pid:%d", hostPid));
		return 0;
	}

	cur_task = get_pid_task(find_pid_ns(hostPid, &init_pid_ns), PIDTYPE_PID);
	if (!cur_task) {
		PVR_DPF((PVR_DBG_WARNING, "Find task for pid:%d failed", hostPid));
		return 0;
	}

	cur_pid_ns = task_active_pid_ns(current);
	if (!cur_pid_ns) {
		PVR_DPF((PVR_DBG_WARNING, "Get current pid ns failed host pid:%d", hostPid));
		return 0;
	}

	converted_pid = task_pid_nr_ns(cur_task, cur_task_pid->numbers[cur_pid_ns->level].ns);
	return converted_pid;
}

IMG_PCHAR OSGetProcessNameByPid(IMG_PID uiPid)
{
	struct task_struct *task;

	task = pid_task(find_vpid(uiPid), PIDTYPE_PID);
	if (!task)
		return NULL;

	return task->comm;
}

struct pci_dev *OSGetPcieDeviceFromVendor(IMG_UINT32 ui32VendorID,
					  IMG_UINT32 ui32DeviceID,
					  struct pci_dev *psPCIDev)
{
	return pci_get_device(ui32VendorID, ui32DeviceID, psPCIDev);
}

IMG_UINT16 OSGetPcieDeviceIDFromPdev(struct pci_dev *psPCIDev)
{
	return psPCIDev->device;
}

IMG_UINT16 OSGetPcieDeviceID(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	struct device *dev = psDeviceNode->psDevConfig->pvOSDevice;
	struct pci_dev *pdev = to_pci_dev(dev->parent->parent);

	return pdev->device;
}

void *OSGetMtgpuDevice(struct platform_device *pdev)
{
	return dev_get_drvdata(pdev->dev.parent->parent);
}

struct platform_device *OSGetPlatformDevice(void *pvOSDevice)
{
	return to_platform_device((struct device *)pvOSDevice);
}

struct resource *OSPlatformGetResourceIRQ(struct platform_device *psPlatformDevice, IMG_UINT32 uiIndex)
{
	return platform_get_resource(psPlatformDevice, IORESOURCE_IRQ, uiIndex);
}

IMG_UINT64 OSGetResourceStart(struct resource *psResource)
{
	return psResource->start;
}

void *OSGetPlatformDeviceParent(struct platform_device *pdev)
{
	return pdev->dev.parent->parent;
}

void *OSGetPlatformData(struct platform_device *pdev)
{
	return pdev->dev.platform_data;
}

#if !defined(NO_HARDWARE)
void OSDeviceConfigInit(PVRSRV_DEVICE_CONFIG *psDevConfig,
			struct platform_device *pdev,
			struct resource *registers)
{
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

	psDevConfig->pvOSDevice = &pdev->dev;
	psDevConfig->sRegsCpuPBase.uiAddr = registers->start;
	psDevConfig->ui32RegsSize = resource_size(registers);
	psDevConfig->ui32IRQ = res->start;
}

struct resource *OSRequestMemRes(void *pvOSDevice)
{
	struct resource *registers;
	struct platform_device *pdev = to_platform_device((struct device *)pvOSDevice);

	registers = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpu-regs");
	if (!registers) {
		PVR_DPF((PVR_DBG_ERROR,
			 "%s: Failed to get GPU register information",
			 __func__));
		return NULL;
	}

	if (!request_mem_region(registers->start,
				resource_size(registers),
				MTGPU_DEVICE_NAME)) {
		PVR_DPF((PVR_DBG_ERROR,
			 "%s: GPU register memory region not available",
			 __func__));

		return NULL;
	}

	return registers;
}

void OSReleaseMemRes(struct resource *res)
{
	release_mem_region(res->start, resource_size(res));
}
#endif /* if !defined(NO_HARDWARE) */

void *OSGetPcieDeviceFromOSDevice(struct device *psOSDevice)
{
	return psOSDevice->parent->parent;
}

PVRSRV_ERROR OSLockCreateNoStats(POS_LOCK *phLock)
{
	PVRSRV_ERROR e = PVRSRV_ERROR_OUT_OF_MEMORY;
	*phLock = OSAllocMemNoStats(sizeof(struct mutex));
	if (*phLock) {
		mutex_init(*phLock);
		e = PVRSRV_OK;
	}

	return e;
}

PVRSRV_ERROR OSLockCreate(POS_LOCK *phLock)
{
	PVRSRV_ERROR e = PVRSRV_ERROR_OUT_OF_MEMORY;
	*phLock = OSAllocMem(sizeof(struct mutex));
	if (*phLock) {
		mutex_init(*phLock);
		e = PVRSRV_OK;
	}

	return e;
}

DEFINE_MUTEX(g_sOSLMALeakMutex);

void OSLockDestroyNoStats(POS_LOCK hLock)
{
	mutex_destroy(hLock);
	OSFreeMemNoStats(hLock);
}

void OSLockDestroy(POS_LOCK hLock)
{
	mutex_destroy(hLock);
	OSFreeMem(hLock);
}

void OSLockAcquire(POS_LOCK hLock)
{
	mutex_lock(hLock);
}

void OSLockAcquireNested(POS_LOCK hLock, unsigned int subclass)
{
	mutex_lock_nested(hLock, subclass);
}

void OSLockRelease(POS_LOCK hLock)
{
	mutex_unlock(hLock);
}

IMG_BOOL OSLockIsLocked(POS_LOCK hLock)
{
	return mutex_is_locked(hLock) ? IMG_TRUE : IMG_FALSE;
}

IMG_BOOL OSTryLockAcquire(POS_LOCK hLock)
{
	return mutex_trylock(hLock) ? IMG_TRUE : IMG_FALSE;
}

DEFINE_SPINLOCK(g_sOSKMallocSpinLock);
DEFINE_SPINLOCK(g_sOSKMallocLeakSpinLock);

PVRSRV_ERROR OSSpinLockCreate(POS_SPINLOCK *_ppsLock)
{
	PVRSRV_ERROR e = PVRSRV_ERROR_OUT_OF_MEMORY;
	*_ppsLock = OSAllocMem(sizeof(spinlock_t));
	if (*_ppsLock) {
		spin_lock_init(*_ppsLock);
		e = PVRSRV_OK;
	}

	return e;
}

void OSSpinLockDestroy(POS_SPINLOCK _psLock)
{
	OSFreeMem(_psLock);
}

void OSSpinLockAcquire(POS_SPINLOCK _pLock, unsigned long *_flags)
{
	spin_lock_irqsave(_pLock, *_flags);
}

void OSSpinLockRelease(POS_SPINLOCK _pLock, unsigned long _flags)
{
	spin_unlock_irqrestore(_pLock, _flags);
}

void OSSpinLock(POS_SPINLOCK _pLock)
{
	spin_lock(_pLock);
}

void OSSpinUnlock(POS_SPINLOCK _pLock)
{
	spin_unlock(_pLock);
}

void OSBitmapSet(unsigned long *map, IMG_UINT uiStart, IMG_UINT uiNbits)
{
	bitmap_set(map, uiStart, uiNbits);
}

void OSBitmapClear(unsigned long *map, IMG_UINT start, IMG_UINT nbits)
{
        bitmap_clear(map, start, nbits);
}

unsigned long OSFindFirstZeroBit(const unsigned long *addr, unsigned long size)
{
	return find_first_zero_bit(addr, size);
}

IMG_INT OSAtomicRead(const ATOMIC_T *pCounter)
{
	return atomic_read(pCounter);
}

void OSAtomicWrite(ATOMIC_T *pCounter, IMG_INT iVal)
{
	atomic_set(pCounter, iVal);
}

IMG_INT OSAtomicIncrement(ATOMIC_T *pCounter)
{
	return atomic_inc_return(pCounter);
}

IMG_INT OSAtomicDecrement(ATOMIC_T *pCounter)
{
	return atomic_dec_return(pCounter);
}

IMG_INT64 OSAtomic64Read(const ATOMIC64_T *pCounter)
{
	return atomic64_read(pCounter);
}

void OSAtomic64Write(ATOMIC64_T *pCounter, IMG_INT64 iVal)
{
	atomic64_set(pCounter, iVal);
}

IMG_INT64 OSAtomic64Increment(ATOMIC64_T *pCounter)
{
	return atomic64_inc_return(pCounter);
}

IMG_INT64 OSAtomic64Decrement(ATOMIC64_T *pCounter)
{
	return atomic64_dec_return(pCounter);
}

IMG_INT64 OSAtomic64Add(ATOMIC64_T *pCounter, IMG_INT64 iVal)
{
	return atomic64_add_return(iVal, pCounter);
}

IMG_INT64 OSAtomic64Subtract(ATOMIC64_T *pCounter, IMG_INT64 iVal)
{
	return atomic64_add_return(-iVal, pCounter);
}

IMG_INT OSAtomicCompareExchange(ATOMIC_T *pCounter, IMG_INT iOldVal, IMG_INT iNewVal)
{
	return atomic_cmpxchg(pCounter, iOldVal, iNewVal);
}

IMG_INT OSAtomicExchange(ATOMIC_T *pCounter, IMG_INT iNewVal)
{
	return atomic_xchg(pCounter, iNewVal);
}

IMG_INT OSAtomicAdd(ATOMIC_T *pCounter, IMG_INT iVal)
{
	return atomic_add_return(iVal, pCounter);
}

IMG_BOOL OSAtomicAddUnless(ATOMIC_T *pCounter, IMG_INT iVal, IMG_INT iTest)
{
	return atomic_add_unless(pCounter, iVal, iTest);
}

IMG_INT OSAtomicSubtract(ATOMIC_T *pCounter, IMG_INT iVal)
{
	return atomic_add_return(-iVal, pCounter);
}

PVRSRV_ERROR OSWRLockCreate(POSWR_LOCK *ppsLock)
{
	PVRSRV_ERROR e = PVRSRV_ERROR_OUT_OF_MEMORY;
	*ppsLock = OSAllocMem(sizeof(struct rw_semaphore));
	if (*ppsLock) {
		init_rwsem(*ppsLock);
		e = PVRSRV_OK;
	}

	return e;
}

void OSWRLockDestroy(POSWR_LOCK psLock)
{
	OSFreeMem(psLock);
}

void OSWRLockAcquireRead(POSWR_LOCK psLock)
{
	down_read(psLock);
}

void OSWRLockAcquireReadNested(POSWR_LOCK psLock, IMG_UINT uiSubclass)
{
	down_read_nested(psLock, uiSubclass);
}

void OSWRLockReleaseRead(POSWR_LOCK psLock)
{
	up_read(psLock);
}

void OSWRLockAcquireWrite(POSWR_LOCK psLock)
{
	down_write(psLock);
}

void OSWRLockReleaseWrite(POSWR_LOCK psLock)
{
	up_write(psLock);
}

struct page *OSGetPageByIndex(struct page *psPage, IMG_UINT32 ui32Index)
{
	return psPage + ui32Index;
}

struct page *OSPhysToPage(IMG_UINT64 ui64Paddr)
{
	return pfn_to_page(ui64Paddr >> PAGE_SHIFT);
}

struct page *OSPfnToPage(IMG_UINT64 ui64PageFrameNum)
{
	return pfn_to_page(ui64PageFrameNum);
}

IMG_UINT64 OSPageToPhys(struct page *psPage)
{
	return page_to_phys(psPage);
}

void *OSVMapCached(struct page **ppsPage, IMG_UINT32 ui32PageNums)
{
	return vmap(ppsPage, ui32PageNums, VM_MAP, PAGE_KERNEL);
}

void *OSVMapNoncached(struct page **ppsPage, IMG_UINT32 ui32PageNums)
{
	return vmap(ppsPage, ui32PageNums, VM_MAP, pgprot_noncached(PAGE_KERNEL));
}

void OSVUnmap(void *pvPtr)
{
	return vunmap(pvPtr);
}

IMG_UINT64 OSDmaMapPage(struct device *psDev, struct page *psPage,
			size_t uiOffset, size_t uiSize, int eDirection)
{
	IMG_UINT64 ui64DmaAddr;

	ui64DmaAddr = dma_map_page(psDev, psPage, uiOffset, uiSize, eDirection);

	return ui64DmaAddr;
}

void OSDmaUnmapPage(struct device *psDev, IMG_UINT64 ui64DmaAddr, size_t uiSize, int eDirection)
{
	return dma_unmap_page(psDev, ui64DmaAddr, uiSize, eDirection);
}

IMG_UINT64 OSDmaMapResource(struct device *psDev, IMG_UINT64 ui64CpuPhysAddr,
			    size_t uiSize, int eDirection,
			    unsigned long ulAttrs)
{
	return dma_map_resource(psDev, ui64CpuPhysAddr, uiSize, eDirection, ulAttrs);
}

void OSDmaUnmapResource(struct device *psDev, IMG_UINT64 ui64DmaAddr,
			size_t uiSize, int eDirection)
{
	dma_unmap_resource(psDev, ui64DmaAddr, uiSize, eDirection, 0);
}

IMG_INT OSDmaMappingError(struct device *psDev, IMG_UINT64 ui64DmaAddr)
{
	return dma_mapping_error(psDev, ui64DmaAddr);
}

void OSSetPageReserved(struct page *psPage)
{
	SetPageReserved(psPage);
}

void OSClearPageReserved(struct page *psPage)
{
	ClearPageReserved(psPage);
}

void *OSPageAddress(const struct page *psPage)
{
	return page_address(psPage);
}

struct page *OSAllocUserPages(IMG_UINT32 ui32Order)
{
	struct page *psPage = NULL;
	IMG_UINT32 ui32Cnt = 0;
	gfp_t gfp_mask = GFP_USER;

	if (ui32Order)
	{
		gfp_mask |= __GFP_COMP;
	}

	/*
	 * When system pmr use huge page, and the os has
	 * severe memory fragmentation, pages allocation
	 * may failed, so add 5 times retrying for pages
	 * allocation.
	 */
	while (!psPage && ui32Cnt < 5)
	{
		psPage = alloc_pages(gfp_mask, ui32Order);
		ui32Cnt++;
	}

	return psPage;
}

IMG_INT OSGetUserPages(IMG_INT64 ui64Start, IMG_INT iPageCount, struct page **ppsPages)
{
	IMG_UINT32 ui32GpuFlags = 0;
	struct vm_area_struct *psVMA;

	mmap_read_lock(current->mm);
	psVMA = find_vma(current->mm, untagged_addr(ui64Start));
	mmap_read_unlock(current->mm);
	if (unlikely(!psVMA))
		return 0;

	if (psVMA->vm_flags & VM_WRITE)
		ui32GpuFlags |= FOLL_WRITE;

	return get_user_pages_fast(ui64Start, iPageCount, ui32GpuFlags, ppsPages);
}

void OSPutPage(struct page *psPage)
{
	put_page(psPage);
}

void OSFreePages(struct page *psPage, IMG_UINT32 ui32Order)
{
	return __free_pages(psPage, ui32Order);
}

IMG_UINT64 OSUntaggedAddr(IMG_UINT64 ui64Addr)
{
	return untagged_addr(ui64Addr);
}

struct pvr_drm_private *OSGetPrivateDataFromDrm(struct drm_device *psDrmDevice)
{
	return psDrmDevice->dev_private;
}

PVRSRV_DEVICE_NODE *OSGetDeviceNodeFromDrm(struct drm_device *psDrmDevice)
{
	struct mtgpu_drm_private *priv = psDrmDevice->dev_private;

	if (!priv) {
		PVR_DPF((PVR_DBG_ERROR, "%s: no private in drm_devive\n", __func__));
		return NULL;
	}

	return priv->pvr_private.dev_node;
}

struct drm_device *OSGetDrmFromDeviceNode(PVRSRV_DEVICE_NODE *psDevNode)
{
	struct device *dev = psDevNode->psDevConfig->pvOSDevice;
	struct drm_device *ddev = dev_get_drvdata(dev);

	return ddev;
}

void OSPgprotNoncached(void *sPageProt)
{
	*(pgprot_t *)sPageProt = pgprot_noncached(*(pgprot_t *)sPageProt);
}

void OSPgprotWriteCombine(void *sPageProt)
{
	*(pgprot_t *)sPageProt = pgprot_writecombine(*(pgprot_t *)sPageProt);
}

IMG_BOOL OSIsErr(const void *pvPtr)
{
	return IS_ERR(pvPtr);
}

long OSPtrErr(const void *pvPtr)
{
	return PTR_ERR(pvPtr);
}

IMG_UINT32 OSConfineArrayIndexNoSpeculation(IMG_UINT32 ui32Index, IMG_UINT32 ui32Size)
{
	return array_index_nospec(ui32Index, ui32Size);
}

IMG_INT OSSScanf(const IMG_CHAR *pcBuf, const IMG_CHAR *pcFmt, ...)
{
	va_list args;
	IMG_INT i;

	va_start(args, pcFmt);
	i = vsscanf(pcBuf, pcFmt, args);
	va_end(args);

	return i;
}

struct bus_type *OSGetDevBusType(struct device *psDev)
{
#ifdef OS_BUS_TYPE_IS_NOT_CONST
	return psDev->bus;
#else
	return (struct bus_type *)psDev->bus;
#endif
}

IMG_BOOL OSIsIOMMUOn(struct bus_type *psBusType)
{
	return iommu_present(psBusType);
}

IMG_BOOL
OSIsPcieEndpoint(struct device *psDev)
{
	struct pci_dev *psPciDev = to_pci_dev(psDev);
	int pcie_type = pci_pcie_type(psPciDev);

	return pcie_type == PCI_EXP_TYPE_ENDPOINT ||
	       pcie_type == PCI_EXP_TYPE_LEG_END;
}

struct device *OSPciUpStreamBridge(struct device *psDev)
{
	struct pci_dev *psPciDev = to_pci_dev(psDev);
	struct pci_dev *psBridge;

	psBridge = pci_upstream_bridge(psPciDev);
	if (psBridge)
	{
		return &psBridge->dev;
	}

	return NULL;
}

static IMG_BOOL
OSIsPcieDownstreamPort(const struct device *psDev)
{
	struct pci_dev *psPciDev = to_pci_dev(psDev);
	int type = pci_pcie_type(psPciDev);

	return type == PCI_EXP_TYPE_ROOT_PORT ||
	       type == PCI_EXP_TYPE_DOWNSTREAM ||
	       type == PCI_EXP_TYPE_PCIE_BRIDGE;
}

IMG_BOOL
OSIsPcieRootPort(const struct device *psDev)
{
	struct pci_dev *psPciDev = to_pci_dev(psDev);
	int type = pci_pcie_type(psPciDev);

	return type == PCI_EXP_TYPE_ROOT_PORT;
}

IMG_BOOL
OSIsPciSiblingDownstreamPort(struct device *psDev1, struct device *psDev2)
{
	struct pci_dev *psDp1 = to_pci_dev(psDev1);
	struct pci_dev *psDp2 = to_pci_dev(psDev2);

	if (OSIsPcieDownstreamPort(&psDp1->dev) && OSIsPcieDownstreamPort(&psDp2->dev))
	{
		/* DP1 and DP2 have some domain and bus number,
		 * means that they are sliblings.
		 */
		if (pci_domain_nr(psDp1->bus) == pci_domain_nr(psDp2->bus) &&
		    psDp1->bus->number == psDp2->bus->number)
		{
			return IMG_TRUE;
		}
	}

	return IMG_FALSE;
}

IMG_INT64 OSGetKernelRealTimeSeconds(void)
{
	struct timespec64 tv;

	ktime_get_real_ts64(&tv);

	return tv.tv_sec;
}

static DEFINE_MUTEX(sHardwareResetStructLock);

void OSHardwareResetLockAcquire(void)
{
	mutex_lock(&sHardwareResetStructLock);
}

void OSHardwareResetLockRelease(void)
{
	mutex_unlock(&sHardwareResetStructLock);
}

/* The mmap code has its own mutex, to prevent possible re-entrant issues
 * when the same PMR is mapped from two different connections/processes.
 */
static DEFINE_MUTEX(g_sMMapMutex);

void OSMMapLockAcquire(void)
{
	mutex_lock(&g_sMMapMutex);
}

void OSMMapLockRelease(void)
{
	mutex_unlock(&g_sMMapMutex);
}

struct file *OSGetFileFromDrmFile(void *pDRMFile)
{
	return ((struct drm_file *)pDRMFile)->filp;
}

#if defined(__drm_debug_enabled)
	#define OS_DEBUG_CONDITION(x)	(!__drm_debug_enabled(x))
#elif defined(OS_FUNC_DRM_DEBUG_ENABLED_EXIST)
	#define OS_DEBUG_CONDITION(x)	(!drm_debug_enabled(x))
#elif defined(OS_GLOBAL_VARIABLE_DRM_DEBUG_EXIST)
	#define OS_DEBUG_CONDITION(x)	(!(drm_debug & (x)))
#else
	#define OS_DEBUG_CONDITION(x)	false
#endif

void OSPvrDrmDbg(unsigned int category, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	if (OS_DEBUG_CONDITION(category))
	{
		return;
	}

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;

	printk(KERN_DEBUG "[" DRM_NAME ":%ps] %pV",
	       __builtin_return_address(0), &vaf);

	va_end(args);
}

void OSPvrDrmPrintk(const char *level, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;

	printk("%s" "[" DRM_NAME ":%ps] %pV",
	       level, __builtin_return_address(0), &vaf);

	va_end(args);
}

IMG_ULONG OSGetVMAreaPageOffset(void *ps_vma)
{
	return ((struct vm_area_struct *)ps_vma)->vm_pgoff;
}

PVRSRV_ERROR OSWaitQueueHeadCreate(struct wait_queue_head **ppsWaitQHead)
{
	*ppsWaitQHead = OSAllocZMem(sizeof(struct wait_queue_head));
	if (*ppsWaitQHead)
	{
		init_waitqueue_head(*ppsWaitQHead);
		return PVRSRV_OK;
	}
	return PVRSRV_ERROR_OUT_OF_MEMORY;
}

void OSWakeUp(struct wait_queue_head *psWaitQHead)
{
	wake_up(psWaitQHead);
}

void OSWakeUpInterruptible(struct wait_queue_head *psWaitQHead)
{
	wake_up_interruptible(psWaitQHead);
}

void OSWakeUpAll(struct wait_queue_head *psWaitQHead)
{
	wake_up_all(psWaitQHead);
}

void OSWaitQueueHeadDestroy(struct wait_queue_head *psWaitQHead)
{
	kfree(psWaitQHead);
}

PVRSRV_ERROR OSReadWritelockCreate(void **ppvReadWriteLock)
{
	*ppvReadWriteLock = OSAllocZMem(sizeof(rwlock_t));
	if (*ppvReadWriteLock)
	{
		return PVRSRV_OK;
	}
	return PVRSRV_ERROR_OUT_OF_MEMORY;
}

void OSReadWriteLockInit(void *pvReadWriteLock)
{
	rwlock_init((rwlock_t *)pvReadWriteLock);
}

void OSWriteLockBottomHalfDisable(void *pvReadWriteLock)
{
	write_lock_bh((rwlock_t *)pvReadWriteLock);
}

void OSWriteUnlockBottomHalfEnable(void *pvReadWriteLock)
{
	write_unlock_bh((rwlock_t *)pvReadWriteLock);
}

void OSReadLockBottomHalfDisable(void *pvReadWriteLock)
{
	read_lock_bh((rwlock_t *)pvReadWriteLock);
}

void OSReadUnlockBottomHalfEnable(void *pvReadWriteLock)
{
	read_unlock_bh((rwlock_t *)pvReadWriteLock);
}

void OSReadWriteLockDestroy(void *pvReadWriteLock)
{
	kfree(pvReadWriteLock);
}

bool OSTryToFreeze(void)
{
	return try_to_freeze();
}

unsigned long OSMsecsToJiffies(const unsigned int msec)
{
	return msecs_to_jiffies(msec);
}

unsigned long OSUsecsToJiffies(const unsigned int usec)
{
	return usecs_to_jiffies(usec);
}

PVRSRV_ERROR OSWaitQueueEntryCreate(struct wait_queue_entry **ppsWaitQEntry)
{
	*ppsWaitQEntry = OSAllocZMem(sizeof(struct wait_queue_entry));
	if (*ppsWaitQEntry)
	{
		(*ppsWaitQEntry)->private = current;
		(*ppsWaitQEntry)->func = autoremove_wake_function;
		(*ppsWaitQEntry)->entry.next = &((*ppsWaitQEntry)->entry);
		(*ppsWaitQEntry)->entry.prev = &((*ppsWaitQEntry)->entry);
		return PVRSRV_OK;
	}
	return PVRSRV_ERROR_OUT_OF_MEMORY;
}

void OSWaitQueueEntryDestroy(struct wait_queue_entry *psWaitQEntry)
{
	kfree(psWaitQEntry);
}

void OSFinishWait(struct wait_queue_head *psWaitQHead, struct wait_queue_entry *psWaitQEntry)
{
	finish_wait(psWaitQHead, psWaitQEntry);
}

void OSPrepareToWait(struct wait_queue_head *psWaitQHead, struct wait_queue_entry *psWaitQEntry, int state)
{
	prepare_to_wait(psWaitQHead, psWaitQEntry, state);
}

int OSSignalPending(void)
{
#ifdef OS_FUNC_TASK_SIGPENDING_EXIST
	return task_sigpending(current);
#else
	return signal_pending(current);
#endif
}

long OSScheduleTimeout(long timeout)
{
	return schedule_timeout(timeout);
}

void OSSchedule(void)
{
	schedule();
}

int OSPrintk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	return r;
}

void OSVFree(const void *addr)
{
	vfree(addr);
}

size_t OSKSize(const void *objp)
{
	return ksize(objp);
}

void OSKFree(const void *x)
{
	kfree(x);
}

bool OSIsVMallocAddr(const void *x)
{
    return is_vmalloc_addr(x);
}

void *OSKMalloc(size_t size)
{
	return kmalloc(size, GFP_KERNEL);
}

void *OSKMallocNode(size_t size, int node)
{
	return kmalloc_node(size, GFP_KERNEL, node);
}

void *OSKZalloc(size_t size)
{
	return kzalloc(size, GFP_KERNEL);
}

void *OSVMalloc(unsigned long size)
{
	return vmalloc(size);
}

void *OSVMallocNode(unsigned long size, int node)
{
	return vmalloc_node(size, node);
}

void *OSVZalloc(unsigned long size)
{
	return vzalloc(size);
}

unsigned int OSBigEndian32ToCPU(unsigned int uiNum)
{
	return be32_to_cpu(uiNum);
}

int OSVerifyPkcs7Signature(const void *pvData, size_t uiLen,
			   const void *pvRawPkcs7, size_t uiPkcs7Len,
			   struct key *psTrusted_keys,
			   enum key_being_used_for eUsage,
			   int (*pfnViewContent)(void *pvCtx,
						 const void *pvData, size_t uilen,
						 size_t uiAsn1hdrlen),
			   void *pvCtx)
{
#ifdef CONFIG_SYSTEM_DATA_VERIFICATION
	return verify_pkcs7_signature(pvData, uiLen, pvRawPkcs7, uiPkcs7Len,
				      psTrusted_keys, eUsage,
				      pfnViewContent, pvCtx);
#else
	return 0;
#endif
}

int OSRequestFirmware(const struct firmware **ppsFw, const char *pszName, struct device *psDevice)
{
	return request_firmware(ppsFw, pszName, psDevice);
}

void OSReleaseFirmware(const struct firmware *psFw)
{
	release_firmware(psFw);
}

size_t OSGetFirmwareSize(const struct firmware *psFw)
{
	return psFw->size;
}

const IMG_UINT8 *OSGetFirmwareData(const struct firmware *psFw)
{
	return psFw->data;
}

int OSIdrCreate(struct idr **ppsIdr)
{
	*ppsIdr = OSAllocMemNoStats(sizeof(struct idr));
	if (*ppsIdr)
	{
		idr_init(*ppsIdr);
		return 0;
	}

	return -ENOMEM;
}

void OSIdrDestroy(struct idr *psIdr)
{
	idr_destroy(psIdr);
	OSFreeMemNoStats(psIdr);
}

void OSIdrPreload(void)
{
	idr_preload(GFP_KERNEL);
}

void OSIdrPreloadEnd(void)
{
	idr_preload_end();
}

int OSIdrAlloc(struct idr *psIdr, void *pv, int iStart, int iEnd, gfp_t gfpFlags)
{
	return idr_alloc(psIdr, pv, iStart, iEnd, gfpFlags);
}

void *OSIdrFind(const struct idr *psIdr, unsigned long ulId)
{
	return idr_find(psIdr, ulId);
}

void *OSIdrRemove(struct idr *psIdr, unsigned long ulId)
{
	return idr_remove(psIdr, ulId);
}

void *OSIdrReplace(struct idr *psIdr, void *pv, unsigned long ulId)
{
	return idr_replace(psIdr, pv, ulId);
}

int OSIdrForEach(const struct idr *psIdr,
		 int (*pFn)(int iId, void *pv, void *pvData),
		 void *pvData)
{
	return idr_for_each(psIdr, pFn, pvData);
}

ssize_t OSKernelWrite(void *pvFile, const char __user *pszBuf, size_t count, loff_t *psPos)
{
	return kernel_write(pvFile, pszBuf, count, psPos);
}

void *OSGetDmaBufPrivateData(struct dma_buf *psDmaBuf)
{
	return psDmaBuf->priv;
}

size_t OSGetDmaBufSize(struct dma_buf *psDmaBuf)
{
	return psDmaBuf->size;
}

const struct dma_buf_ops *OSGetDmaBufOps(struct dma_buf *psDmaBuf)
{
	return psDmaBuf->ops;
}

unsigned int OSGetSgTableNents(struct sg_table *psSgTable)
{
	return psSgTable->nents;
}

struct scatterlist *OSGetSgTableSglist(struct sg_table *psSgTable)
{
	return psSgTable->sgl;
}

struct scatterlist *OSGetSglistNext(struct scatterlist *psSg)
{
	return sg_next(psSg);
}

IMG_UINT64 OSGetSglistDmaAddress(struct scatterlist *psSg)
{
	return sg_dma_address(psSg);
}

void OSSetSglistDmaAddress(struct scatterlist *psSg, IMG_UINT64 ui64DmaAddr)
{
	psSg->dma_address = ui64DmaAddr;
}

IMG_UINT64 OSGetSglistDmaLength(struct scatterlist *psSg)
{
#if defined(PVR_ANDROID_ION_USE_SG_LENGTH)
	return psSg->length;
#else
	return sg_dma_len(psSg);
#endif
}

void OSSetSglistDmaLength(struct scatterlist *psSg, IMG_INT64 ui64DmaLength)
{
#if defined(PVR_ANDROID_ION_USE_SG_LENGTH)
	psSg->length = ui64DmaLength;
#else
	sg_dma_len(psSg) = ui64DmaLength;
#endif
}

int OSSgTableCreate(struct sg_table **ppsSgTable)
{
	*ppsSgTable = OSAllocZMem(sizeof(struct sg_table));
	if (*ppsSgTable)
	{
		return 0;
	}

	return -ENOMEM;
}

int OSSgTableAllocList(struct sg_table *psTable, unsigned int uiNents)
{
	return sg_alloc_table(psTable, uiNents, GFP_KERNEL);
}

void OSSgTableFreeList(struct sg_table *psSgTable)
{
	sg_free_table(psSgTable);
}

void *OSGetDmaBufPrivateDataByAttachment(struct dma_buf_attachment *psAttachment)
{
	return psAttachment->dmabuf->priv;
}

struct device *OSGetDmaBufAttachmentDev(struct dma_buf_attachment *psAttachment)
{
	return psAttachment->dev;
}

static struct sg_table *PVRDmaBufOpsMapWrapper(struct dma_buf_attachment *psAttachment,
					       enum dma_data_direction eDirection)
{
	return PVRDmaBufOpsMap(psAttachment, eDirection);
}

static void PVRDmaBufOpsUnmapWrapper(struct dma_buf_attachment *psAttachment,
				     struct sg_table *psSgTable,
				     enum dma_data_direction eDirection)
{
	PVRDmaBufOpsUnmap(psAttachment, psSgTable, eDirection);
}

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
int PVRDmaBufOpsVMap(struct dma_buf *psDmaBuf, struct iosys_map *psMap)
#else
void *PVRDmaBufOpsVMap(struct dma_buf *psDmaBuf)
#endif
{
	void *pvKernAddr = NULL;

	if (DmaBufOpsVMap(psDmaBuf, pvKernAddr))
	{
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
		return -EINVAL;
#else
		return NULL;
#endif
	}

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	iosys_map_set_vaddr(psMap, pvKernAddr);
	return 0;
#else
	return pvKernAddr;
#endif
}

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
void PVRDmaBufOpsVUnmap(struct dma_buf *psDmaBuf, struct iosys_map *psMap)
#else
void PVRDmaBufOpsVUnmap(struct dma_buf *psDmaBuf, void *pvKernAddr)
#endif
{
	DmaBufOpsVUnmap(psDmaBuf);
}

void *OSGetDmaBufAddrFromPrivdata(void *pvPriv, size_t uiOffset)
{
	PMR_DMA_BUF_DATA *psPrivData = pvPriv;
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	return psPrivData->psMap->vaddr + uiOffset;
#else
	return psPrivData->vaddr + uiOffset;
#endif
}

/*
 * dma_buf_ops
 *
 * support outside drivers to import and use the dma-buf by ops
 */

int PVRDmaBufOpsAttach(struct dma_buf *psDmaBuf,
#if defined(OS_DMA_BUF_OPS_ATTACH_HAS_DEVICE_ARG)
		       struct device *psDev,
#endif
		       struct dma_buf_attachment *psAttachment)
{
	PMR *psPMR = psDmaBuf->priv;

	if (!psPMR)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: PMR is NULL", __func__));
		return -EINVAL;
	}

	if (PMR_IsUser(psPMR))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "%s: PMR for user memory is not supported", __func__));
		return -EINVAL;
	}

	return 0;
}

static const struct dma_buf_ops sPVRDmaBufOps = {
	.attach        = PVRDmaBufOpsAttach,
	.map_dma_buf   = PVRDmaBufOpsMapWrapper,
	.unmap_dma_buf = PVRDmaBufOpsUnmapWrapper,
	.release       = PVRDmaBufOpsRelease,
#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_MAP_ATOMIC)
	.map_atomic    = PVRDmaBufOpsKMap,
#endif
#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_MAP)
	.map           = PVRDmaBufOpsKMap,
#endif
#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_KMAP_ATOMIC)
	.kmap_atomic   = PVRDmaBufOpsKMap,
#endif
#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_KMAP)
	.kmap          = PVRDmaBufOpsKMap,
#endif
	.mmap          = PVRDmaBufOpsMMap,
	.vmap          = PVRDmaBufOpsVMap,
	.vunmap        = PVRDmaBufOpsVUnmap,
};

const struct dma_buf_ops *OSGetPvrDmaBufOps(void)
{
	return &sPVRDmaBufOps;
}

DEFINE_MUTEX(g_sOSHashLock);

#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_MAP_ATOMIC) || \
    defined(OS_STRUCT_DMA_BUF_OPS_HAS_MAP) || \
    defined(OS_STRUCT_DMA_BUF_OPS_HAS_KMAP_ATOMIC) || \
    defined(OS_STRUCT_DMA_BUF_OPS_HAS_KMAP)
void *PVRDmaBufOpsKMap(struct dma_buf *psDmaBuf, unsigned long uiPageNum)
{
	return NULL;
}
#endif

int OSDmaBufBeginCpuAccess(struct dma_buf *psDmBuf, int eDirection)
{
	return dma_buf_begin_cpu_access(psDmBuf, eDirection);
}

int OSDmaBufVmap(struct dma_buf *psDmaBuf, void *pvPriv, void **pvMapOut, void **vaddr)
{
	PMR_DMA_BUF_DATA *psPrivData = pvPriv;

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	void *pvMap;

	if (pvPriv)
	{
		if (dma_buf_vmap(psDmaBuf, psPrivData->psMap) || !psPrivData->psMap->vaddr)
		{
			return -ENOMEM;
		}
	}
	else
	{
		pvMap = OSAllocZMem(sizeof(struct iosys_map));
		if (!pvMap)
		{
			PVR_DPF((PVR_DBG_ERROR, "%s: Couldn't allocate IosysMap", __func__));
			return -ENOMEM;
		}

		if (dma_buf_vmap(psDmaBuf, (struct iosys_map *)pvMap))
		{
			OSFreeMem(pvMap);
			return -ENOMEM;
		}

		*pvMapOut = pvMap;
	}
#else
	if (pvPriv)
	{
		psPrivData->vaddr = dma_buf_vmap(psDmaBuf);
		if (!psPrivData->vaddr)
		{
			return -ENOMEM;
		}
	}
	else
	{
		*vaddr = dma_buf_vmap(psDmaBuf);
		if (!(*vaddr))
		{
			return -ENOMEM;
		}
	}
#endif
	return 0;
}

int OSDmaBufKmap(struct dma_buf *psDmaBuf, int iValue, const char *szFunc)
{
#if defined(OS_FUNC_DMA_BUF_KMAP_EXIST)
	int i;

	for (i = 0; i < OSGetDmaBufSize(psDmaBuf) / gpu_page_size; i++)
	{
		void *pvKernAddr;

		pvKernAddr = dma_buf_kmap(psDmaBuf, i);
		if (IS_ERR_OR_NULL(pvKernAddr))
		{
			PVR_DPF((PVR_DBG_ERROR, "%s: Failed to map page (err=%ld)",
						szFunc,
						pvKernAddr ? PTR_ERR(pvKernAddr) : -(long)ENOMEM));
			return !pvKernAddr ? -ENOMEM : -EINVAL;
		}

		memset(pvKernAddr, iValue, gpu_page_size);
		dma_buf_kunmap(psDmaBuf, i, pvKernAddr);
	}

	return 0;
#else
	PVR_DPF((PVR_DBG_ERROR, "%s: Failed to map page", szFunc));
	return -ENOMEM;
#endif
}

int OSSetDmaBufValue(struct dma_buf *psDmaBuf, int iValue, void *pvMap, void *vaddr)
{
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	memset(((struct iosys_map *)pvMap)->vaddr, iValue, psDmaBuf->size);
	dma_buf_vunmap(psDmaBuf, (struct iosys_map *)pvMap);
#else
	if (!vaddr)
	{
		return -ENOMEM;
	}
	memset(vaddr, iValue, psDmaBuf->size);
	dma_buf_vunmap(psDmaBuf, vaddr);
#endif
	return 0;
}

void OSDmaBufKunmap(struct dma_buf *psDmBuf, unsigned long ulPageNum,
		    void *pvAddr)
{
#if defined(OS_FUNC_DMA_BUF_KMAP_EXIST)
	dma_buf_kunmap(psDmBuf, ulPageNum, pvAddr);
#endif
}

void OSDmaBufVunmap(struct dma_buf *psDmBuf, void *pvPriv)
{
	PMR_DMA_BUF_DATA *psPrivData = pvPriv;
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	dma_buf_vunmap(psDmBuf, psPrivData->psMap);
#else
	dma_buf_vunmap(psDmBuf, psPrivData->vaddr);
#endif
}

int OSIosysMapCreate(void *pvPriv)
{
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	PMR_DMA_BUF_DATA *psPrivData = pvPriv;

	psPrivData->psMap = OSAllocZMem(sizeof(struct iosys_map));
	if (psPrivData->psMap)
	{
		return 0;
	}

	return -ENOMEM;
#endif
	return 0;
}

void OSIosysMapDestroy(void *pvPriv)
{
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	PMR_DMA_BUF_DATA *psPrivData = pvPriv;

	OSFreeMem(psPrivData->psMap);
#endif
}

int OSDmaBufEndCpuAccessDmaToDevice(struct dma_buf *psDmBuf)
{
	return dma_buf_end_cpu_access(psDmBuf, DMA_TO_DEVICE);
}

int OSDmaBufEndCpuAccessDmaBidirectional(struct dma_buf *psDmBuf)
{
	return dma_buf_end_cpu_access(psDmBuf, DMA_BIDIRECTIONAL);
}

void OSDmaBufUnmapAttachment(struct dma_buf_attachment *psAttach,
			     struct sg_table *psSgTable)
{
	dma_buf_unmap_attachment(psAttach, psSgTable, DMA_BIDIRECTIONAL);
}

struct sg_table *OSDmaBufMapAttachment(struct dma_buf_attachment *psAttach)
{
	return dma_buf_map_attachment(psAttach, DMA_BIDIRECTIONAL);
}

int OSDmaBufMMap(struct dma_buf *psDmBuf, struct vm_area_struct *psVma,
		 unsigned long ulPgOff)
{
	return dma_buf_mmap(psDmBuf, psVma, ulPgOff);
}

struct dma_buf_attachment *OSGetAttachmentFromPrivData(struct _PMR_DMA_BUF_DATA_ *psPrivData)
{
	return psPrivData->psAttachment;
}

struct dma_buf *OSGetDmaBufFromAttachment(struct dma_buf_attachment *psAttachment)
{
	return psAttachment->dmabuf;
}

void OSDmaBufDetach(struct dma_buf *psDmBuf, struct dma_buf_attachment *psAttach)
{
	dma_buf_detach(psDmBuf, psAttach);
}

void OSDmaBufPut(struct dma_buf *psDmBuf)
{
	dma_buf_put(psDmBuf);
}

int OSDmaBufExportInfoCreate(struct dma_buf_export_info **ppsDmaBufExportInfo)
{
	*ppsDmaBufExportInfo = OSAllocZMem(sizeof(struct dma_buf_export_info));
	if (*ppsDmaBufExportInfo)
	{
		(*ppsDmaBufExportInfo)->exp_name = KBUILD_MODNAME;
		(*ppsDmaBufExportInfo)->owner = THIS_MODULE;

		return 0;
	}

	return -ENOMEM;
}

void OSDmaBufExportInfoDestroy(struct dma_buf_export_info *psDmaBufExportInfo)
{
	OSFreeMem(psDmaBufExportInfo);
}

void OSSetDmaBufExportInfo(struct dma_buf_export_info *psDmaBufExportInfo, PMR *psPMR,
			   const struct dma_buf_ops *psOps, IMG_DEVMEM_SIZE_T uiPMRSize)
{
	psDmaBufExportInfo->priv = psPMR;
	psDmaBufExportInfo->ops = psOps;
	psDmaBufExportInfo->size = uiPMRSize;
	psDmaBufExportInfo->flags = O_RDWR;
}

struct dma_buf *OSDmaBufExport(const struct dma_buf_export_info *psExpInfo)
{
	return dma_buf_export(psExpInfo);
}

int OSGetDmaBufFd(struct dma_buf *psDmBuf)
{
	return dma_buf_fd(psDmBuf, O_RDWR);
}

struct dma_buf *OSDmaBufGet(int iFd)
{
	return dma_buf_get(iFd);
}

struct dma_buf_attachment *OSDmaBufAttach(struct dma_buf *psDmBuf,
					  struct device *psDev)
{
	return dma_buf_attach(psDmBuf, psDev);
}

bool OSIsErrOrNull(__force const void *pvPtr)
{
	return IS_ERR_OR_NULL(pvPtr);
}

void *OSGetKernelParamArg(const struct kernel_param *psKernelParam)
{
	return psKernelParam->arg;
}

bool OSQueueWork(struct workqueue_struct *psWorkQueue, struct work_struct *psWork)
{
	return queue_work(psWorkQueue, psWork);
}

struct workqueue_struct *OSCreateSingleThreadWorkqueue(const IMG_CHAR *pszFormat)
{
	return create_singlethread_workqueue(pszFormat);
}

struct workqueue_struct *OSAllocWorkqueue(const IMG_CHAR *pszFormat, unsigned int flags, int max_active)
{
	return alloc_workqueue(pszFormat, flags, max_active);
}

void OSFlushWorkqueue(struct workqueue_struct *psWorkQueue)
{
	flush_workqueue(psWorkQueue);
}

void OSDestroyWorkqueue(struct workqueue_struct *psWorkQueue)
{
	destroy_workqueue(psWorkQueue);
}

struct mt_work_struct {
	struct work_struct work;
	void *pvData;
};

void *OSCreateWork(work_func_t pfnFunc)
{
	struct mt_work_struct *work = kzalloc(sizeof(struct mt_work_struct), GFP_KERNEL);
	if (!work)
		return NULL;

	INIT_WORK(&work->work, pfnFunc);

	return work;
}

void OSDestroyWork(struct work_struct *psWork)
{
	kfree(psWork);
}

void OSSetWorkDrvdata(struct work_struct *psWork, void *pvData)
{
	struct mt_work_struct *mt_work = container_of(psWork, struct mt_work_struct, work);

	mt_work->pvData = pvData;
}

void *OSGetWorkDrvdata(struct work_struct *psWork)
{
	struct mt_work_struct *mt_work = container_of(psWork, struct mt_work_struct, work);

	return mt_work->pvData;
}

struct workqueue_struct *OSCreateFreezableWorkqueue(char *name)
{
	return create_freezable_workqueue(name);
}

void *OSCreateDelayedWork(void)
{
	return kzalloc(sizeof(struct delayed_work), GFP_KERNEL);
}

void OSDestroyDelayedWork(struct delayed_work *dwork)
{
	kfree(dwork);
}

void OSInitDelayedWork(struct delayed_work *dwork, work_func_t func)
{
	INIT_DELAYED_WORK(dwork, func);
}

bool OSQueueDelayedWork(struct workqueue_struct *wq,
			struct delayed_work *dwork,
			unsigned long delay)
{
	return queue_delayed_work(wq, dwork, delay);
}

bool OSCancelDelayedWorkSync(struct delayed_work *dwork)
{
	return cancel_delayed_work_sync(dwork);
}

struct delayed_work *OSToDelayedWork(struct work_struct *work)
{
	return to_delayed_work(work);
}

struct mt_dma_fence {
	struct dma_fence dma_fence;
	void *data;
};

void *OSCreateDmaFence(void)
{
	return kzalloc(sizeof(struct mt_dma_fence), GFP_KERNEL);
}

void OSDestroyDmaFence(struct dma_fence *dma_fence)
{
	kfree(dma_fence);
}

static struct mt_dma_fence *OSGetMtDmaFence(struct dma_fence *dma_fence)
{
	return container_of(dma_fence, struct mt_dma_fence, dma_fence);
}

void OSSetDmaFenceDrvdata(struct dma_fence *dma_fence, void *data)
{
	struct mt_dma_fence *mt_dma_fence = OSGetMtDmaFence(dma_fence);

	mt_dma_fence->data = data;
}

void *OSGetDmaFenceDrvdata(struct dma_fence *dma_fence)
{
	struct mt_dma_fence *mt_dma_fence = OSGetMtDmaFence(dma_fence);

	return mt_dma_fence->data;
}

struct mt_dma_fence_cb {
	struct dma_fence_cb dma_fenc_cb;
	void *data;
};

void *OSCreateDmaFenceCB(void)
{
	return kzalloc(sizeof(struct dma_fence_cb), GFP_KERNEL);
}

void OSDestroyDmaFenceCB(struct dma_fence_cb *dma_fence_cb)
{
	kfree(dma_fence_cb);
}

static struct mt_dma_fence_cb *OSGetMtDmaFenceCB(struct dma_fence_cb *dma_fence_cb)
{
	return container_of(dma_fence_cb, struct mt_dma_fence_cb, dma_fenc_cb);
}

void OSSetDmaFenceCBDrvdata(struct dma_fence_cb *dma_fence_cb, void *data)
{
	struct mt_dma_fence_cb *mt_dma_fence_cb = OSGetMtDmaFenceCB(dma_fence_cb);

	mt_dma_fence_cb->data = data;
}

void *OSGetDmaFenceCBDrvdata(struct dma_fence_cb *dma_fence_cb)
{
	struct mt_dma_fence_cb *mt_dma_fence_cb = OSGetMtDmaFenceCB(dma_fence_cb);

	return mt_dma_fence_cb->data;
}

IMG_UINT64 OSDmaFenceOpsInit(struct dma_fence_ops **ops,
                             const struct MTDmaFenceOps *mt_ops)
{
	struct dma_fence_ops *dma_ops;

	dma_ops = kzalloc(sizeof(**ops), GFP_KERNEL);
	if (!dma_ops)
		return -ENOMEM;

	dma_ops->get_driver_name = mt_ops->get_driver_name;
	dma_ops->get_timeline_name = mt_ops->get_timeline_name;
	dma_ops->enable_signaling = mt_ops->enable_signaling;
	dma_ops->signaled = mt_ops->signaled;
	dma_ops->wait = mt_ops->wait;
	dma_ops->release = mt_ops->release;
	dma_ops->fence_value_str = mt_ops->fence_value_str;
	dma_ops->timeline_value_str = mt_ops->timeline_value_str;

	*ops = dma_ops;

	return 0;
}

void OSDmaFenceInit(struct dma_fence *fence,
                    const struct dma_fence_ops *ops,
                    spinlock_t *lock, u64 context, u64 seqno)
{
	dma_fence_init(fence, ops, lock, context, seqno);
}

IMG_UINT64 OSDmaFenceContextAlloc(IMG_UINT32 num)
{
	return dma_fence_context_alloc(num);
}

#if !defined(OS_FUNC_DMA_FENCE_GET_STUB_EXIST)
static const char *os_dma_fence_stub_get_name(struct dma_fence *fence)
{
	return "stub";
}
#endif

struct dma_fence *OSDmaFenceGetStub(void)
{
#if defined(OS_FUNC_DMA_FENCE_GET_STUB_EXIST)
	return dma_fence_get_stub();
#else
	static struct dma_fence dma_fence_stub;
	static DEFINE_SPINLOCK(dma_fence_stub_lock);
	static const struct dma_fence_ops dma_fence_stub_ops = {
                .get_driver_name = os_dma_fence_stub_get_name,
		.get_timeline_name = os_dma_fence_stub_get_name,
	};

	spin_lock(&dma_fence_stub_lock);
	if (!dma_fence_stub.ops) {
		dma_fence_init(&dma_fence_stub,
                               &dma_fence_stub_ops,
                               &dma_fence_stub_lock,
                               0, 0);
		dma_fence_signal_locked(&dma_fence_stub);
	}
	spin_unlock(&dma_fence_stub_lock);

        return dma_fence_get(&dma_fence_stub);
#endif
}

struct dma_fence *OSDmaFenceGet(struct dma_fence *fence)
{
	return dma_fence_get(fence);
}

void OSDmaFenSignal(struct dma_fence *fence)
{
	dma_fence_signal(fence);
}

void OSDmaFencePut(struct dma_fence *fence)
{
	dma_fence_put(fence);
}

bool OSDmaFenceIsSignaled(struct dma_fence *fence)
{
	return dma_fence_is_signaled(fence);
}

int OSDmaFenceAddCallback(struct dma_fence *fence, struct dma_fence_cb *cb,
			      dma_fence_func_t func)
{
	return dma_fence_add_callback(fence, cb, func);
}

struct sync_file *OSSyncFileCreate(struct dma_fence *fence)
{
	return sync_file_create(fence);
}

int OSGetUnusedFDFlags(unsigned flag)
{
	return get_unused_fd_flags(flag);
}

void OSFDInstallSyncFile(int fd, struct sync_file *sync_file)
{
	fd_install(fd, sync_file->file);
}

struct dma_fence *OSSyncFileGetFence(int fd)
{
	return sync_file_get_fence(fd);
}

/* Change the linux kernel's mapping of this address to cpu uncached wc. */
void OSArchLinearMapAttrSetCached(phys_addr_t addr, size_t size)
{
#if defined(OS_FUNC_ARCH_LINEAR_MAP_ATTR_SET_CACHED_EXIST)
	arch_linear_map_attr_set_cached(addr, size);
#endif
}

/* Change the linux kernel's mapping of this address to cpu cached. */
void OSArchLinearMapAttrSetUncached(phys_addr_t addr, size_t size)
{
#if defined(OS_FUNC_ARCH_LINEAR_MAP_ATTR_SET_CACHED_EXIST)
	arch_linear_map_attr_set_uncached(addr, size);
#endif
}
