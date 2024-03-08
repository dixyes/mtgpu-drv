/*************************************************************************/ /*!
@File
@Title          Linux OS PMR functions
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

#include <asm/page.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
#include <linux/pfn_t.h>
#include <linux/pfn.h>
#endif

#include "img_defs.h"
#include "pvr_debug.h"
#include "allocmem.h"
#include "memory_pool.h"
#include "devicemem_server_utils.h"
#include "pmr.h"
#include "pmr_os.h"
#include "cache_km.h"

#include "asm/processor.h"

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
#include "process_stats.h"
#endif

#if defined(PVRSRV_ENABLE_LINUX_MMAP_STATS)
#include "mmap_stats.h"
#endif

#include "kernel_compatibility.h"

/*
 * x86_32:
 * Use vm_insert_page because remap_pfn_range has issues when mapping HIGHMEM
 * pages with default memory attributes; these HIGHMEM pages are skipped in
 * set_pages_array_[uc,wc] during allocation; see reserve_pfn_range().
 * Also vm_insert_page is faster.
 *
 * x86_64:
 * Use vm_insert_page because it is faster.
 *
 * Other platforms:
 * Use remap_pfn_range by default because it does not issue a cache flush.
 * It is known that ARM32 benefits from this. When other platforms become
 * available it has to be investigated if this assumption holds for them as well.
 *
 * Since vm_insert_page does more precise memory accounting we have the build
 * flag PVR_MMAP_USE_VM_INSERT that forces its use. This is useful as a debug
 * feature.
 *
 */
#if defined(CONFIG_X86) || defined(PVR_MMAP_USE_VM_INSERT)
#define PMR_OS_USE_VM_INSERT_PAGE 1
#endif

static void OSSetVMAFlags(struct vm_area_struct *psVma, unsigned long ulFlags)
{
#ifdef OS_VM_FLAGS_IS_NOT_CONST
	psVma->vm_flags |= ulFlags;
#else
	vm_flags_set(psVma, (vm_flags_t)ulFlags);
#endif
}

static void OSClearVMAFlags(struct vm_area_struct *psVma, unsigned long ulFlags)
{
#ifdef OS_VM_FLAGS_IS_NOT_CONST
	psVma->vm_flags &= ~ulFlags;
#else
	vm_flags_clear(psVma, (vm_flags_t)ulFlags);
#endif
}

static void MMapPMROpen(struct vm_area_struct *ps_vma)
{
	PMR *psPMR = ps_vma->vm_private_data;

	/* Our VM flags should ensure this function never gets called */
	PVR_DPF((PVR_DBG_WARNING,
			 "%s: Unexpected mmap open call, this is probably an application bug.",
			 __func__));
	PVR_DPF((PVR_DBG_WARNING,
			 "%s: vma struct: 0x%p, vAddr: %#lX, length: %#lX, PMR pointer: 0x%p",
			 __func__,
			 ps_vma,
			 ps_vma->vm_start,
			 ps_vma->vm_end - ps_vma->vm_start,
			 psPMR));

	/* In case we get called anyway let's do things right by increasing the refcount and
	 * locking down the physical addresses. */
	PMRRefPMR(psPMR);

	if (PMRLockSysPhysAddresses(psPMR) != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Could not lock down physical addresses, aborting.", __func__));
		PMRUnrefPMR(psPMR);
	}
}

static void MMapPMRClose(struct vm_area_struct *ps_vma)
{
	PMR *psPMR = ps_vma->vm_private_data;

#if defined(PVRSRV_ENABLE_PROCESS_STATS)
#if defined(PVRSRV_ENABLE_MEMORY_STATS)
	{
		uintptr_t vAddr = ps_vma->vm_start;

		while (vAddr < ps_vma->vm_end)
		{
			/* USER MAPPING */
			PVRSRVStatsRemoveMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE_MAP_UMA_LMA_PAGES,
							(IMG_UINT64)vAddr,
							OSGetCurrentClientProcessIDKM(),
							PMR_DeviceNode(psPMR));
			vAddr += PAGE_SIZE;
		}
	}
#else
	PVRSRVStatsDecrMemAllocStat(PVRSRV_MEM_ALLOC_TYPE_MAP_UMA_LMA_PAGES,
				    ps_vma->vm_end - ps_vma->vm_start,
				    OSGetCurrentClientProcessIDKM(),
				    PMR_DeviceNode(psPMR));
#endif
#endif

	PMRUnlockSysPhysAddresses(psPMR);
	PMRUnrefPMR(psPMR);
}

/*
 * This vma operation is used to read data from mmap regions. It is called
 * by access_process_vm, which is called to handle PTRACE_PEEKDATA ptrace
 * requests and reads from /proc/<pid>/mem.
 */
static int MMapVAccess(struct vm_area_struct *ps_vma, unsigned long addr,
		       void *buf, int len, int write)
{
	PMR *psPMR = ps_vma->vm_private_data;
	unsigned long ulOffset = addr - ps_vma->vm_start;
	size_t uiBytesCopied;
	PVRSRV_ERROR eError;
	int iRetVal = -EINVAL;

	if (write)
	{
		eError = PMR_WriteBytes(psPMR,
					(IMG_DEVMEM_OFFSET_T) ulOffset,
					buf,
					len,
					&uiBytesCopied);
	}
	else
	{
		eError = PMR_ReadBytes(psPMR,
				       (IMG_DEVMEM_OFFSET_T) ulOffset,
				       buf,
				       len,
				       &uiBytesCopied);
	}

	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Error from %s (%d)",
			 __func__,
			 write ? "PMR_WriteBytes" : "PMR_ReadBytes",
			 eError));
	}
	else
	{
		iRetVal = uiBytesCopied;
	}

	return iRetVal;
}

static const struct vm_operations_struct gsMMapOps =
{
	.open = &MMapPMROpen,
	.close = &MMapPMRClose,
	.access = MMapVAccess,
};

static INLINE int _OSMMapPMR(PVRSRV_DEVICE_NODE *psDevNode,
							struct vm_area_struct *ps_vma,
							IMG_DEVMEM_OFFSET_T uiOffset,
							IMG_CPU_PHYADDR *psCpuPAddr,
							IMG_UINT32 uiLog2PageSize,
							IMG_BOOL bUseVMInsertPage,
							IMG_BOOL bUseMixedMap)
{
	IMG_INT32 iStatus;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
	pfn_t sPFN;
#else
	unsigned long uiPFN;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
	sPFN = phys_to_pfn_t(psCpuPAddr->uiAddr, 0);
#else
	uiPFN = psCpuPAddr->uiAddr >> PAGE_SHIFT;
	PVR_ASSERT(((IMG_UINT64)uiPFN << PAGE_SHIFT) == psCpuPAddr->uiAddr);
#endif

	/*
	 * vm_insert_page() allows insertion of individual pages into user
	 * VMA space _only_ if page is a order-zero allocated page
	 */
	if (bUseVMInsertPage)
	{
		if (bUseMixedMap)
		{
			/*
			 * This path is just for debugging. It should be
			 * equivalent to the remap_pfn_range() path.
			 */
#if defined(OS_FUNC_VMF_INSERT_MIXED_EXIST)
			vm_fault_t vmf;

			vmf = vmf_insert_mixed(ps_vma,
									ps_vma->vm_start + uiOffset,
									sPFN);
			if (vmf & VM_FAULT_ERROR)
			{
				iStatus = vm_fault_to_errno(vmf, 0);
			}
			else
			{
				iStatus = 0;
			}
#else
			iStatus = vm_insert_mixed(ps_vma,
									  ps_vma->vm_start + uiOffset,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
									  sPFN);
#else
									  uiPFN);
#endif
#endif
		}
		else
		{
			/* Since kernel 3.7 this sets VM_MIXEDMAP internally */
			iStatus = vm_insert_page(ps_vma,
									 ps_vma->vm_start + uiOffset,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
									 pfn_t_to_page(sPFN));
#else
									 pfn_to_page(uiPFN));
#endif
		}
	}
	else
	{
		/*
		   NOTE: Regarding absence of dma_mmap_coherent() in _OSMMapPMR()

		   The current services mmap model maps in a PMR's full-length size
		   into the user VMA & applies any user specified offset to the kernel
		   returned zero-offset based VA in services client; this essentially
		   means services server ignores ps_vma->vm_pgoff (this houses hPMR)
		   during a mmap call.

		   Furthermore, during a DMA/CMA memory allocation, multiple order-n
		   pages are used to satisfy an allocation request due to DMA/CMA
		   framework rounding-up allocation size to next power-of-two which
		   can lead to wasted memory (so we don't allocate using single call).

		   The combination of the above two issues mean that we cannot use the
		   dma_mmap_coherent() for a number of reasons outlined below:

		     - Services mmap semantics does not fit with dma_mmap_coherent()
		       which requires proper ps_vma->vm_pgoff; seeing this houses a
		       hPMR handle value, calls into dma_mmap_coherent() fails. This
		       could be avoided by forcing ps_vma->vm_pgoff to zero but the
		       ps_vma->vm_pgoff is applied to DMA bus address PFN and not
		       user VMA which is always mapped at ps_vma->vm_start.

		     - As multiple order-n pages are used for DMA/CMA allocations, a
		       single dma_mmap_coherent() call with a vma->vm_pgoff set to
		       zero cannot (maybe) be used because there is no guarantee that
		       all of the multiple order-n pages in the PMR are physically
		       contiguous from the first entry to the last. Whilst this is
		       highly likely to be the case, there is no guarantee that it
		       will be so we cannot depend on this being the case.

		   The solution is to manually mmap DMA/CMA pages into user VMA
		   using remap_pfn_range() directly. Furthermore, accounting is
		   always compromised for DMA/CMA allocations.
		*/
		size_t uiNumContiguousBytes = 1ULL << uiLog2PageSize;

		iStatus = remap_pfn_range(ps_vma,
								  ps_vma->vm_start + uiOffset,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
								  pfn_t_to_pfn(sPFN),
#else
								  uiPFN,
#endif
								  uiNumContiguousBytes,
								  ps_vma->vm_page_prot);
	}

	return iStatus;
}

PVRSRV_ERROR
OSMMapPMRGeneric(PMR *psPMR, PMR_MMAP_DATA pOSMMapData)
{
	struct vm_area_struct *ps_vma = pOSMMapData;
	PVRSRV_DEVICE_NODE *psDevNode = PMR_DeviceNode(psPMR);
	PVRSRV_ERROR eError;
	size_t uiLength;
	IMG_INT32 iStatus;
	IMG_DEVMEM_OFFSET_T ui64OffsetByGPUPage;
	IMG_UINT32 ui32CPUCacheFlags;
	pgprot_t sPageProt;
	IMG_CPU_PHYADDR asCpuPAddr[PMR_MAX_TRANSLATION_STACK_ALLOC];
	IMG_BOOL abValid[PMR_MAX_TRANSLATION_STACK_ALLOC];
	IMG_UINT32 uiOffsetIdx;
	IMG_UINT32 uiNumOfPFNs;
	IMG_UINT32 uiLog2PageSize;
	IMG_CPU_PHYADDR *psCpuPAddr;
	IMG_BOOL *pbValid;
	IMG_BOOL bUseMixedMap = IMG_FALSE;
	IMG_BOOL bUseVMInsertPage = IMG_FALSE;
	IMG_UINT32 i;
	IMG_UINT32 ui32LoopCount = 1;
	IMG_UINT64 ui64VmOffset;
	IMG_CPU_PHYADDR sCpuPAddr;

	eError = PMRLockSysPhysAddresses(psPMR);
	if (eError != PVRSRV_OK)
	{
		goto e0;
	}

	if (((ps_vma->vm_flags & VM_WRITE) != 0) &&
		((ps_vma->vm_flags & VM_SHARED) == 0))
	{
		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto e1;
	}

	sPageProt = vm_get_page_prot(ps_vma->vm_flags);

	eError = DevmemCPUCacheMode(psDevNode,
	                            PMR_Flags(psPMR),
	                            &ui32CPUCacheFlags);
	if (eError != PVRSRV_OK)
	{
		goto e0;
	}

	eError = CpuCacheFlag2Pgprot(&sPageProt, ui32CPUCacheFlags,
				     PMR_IsUser(psPMR) || PMR_IsSystem(psPMR));
	if (eError != PVRSRV_OK)
		goto e1;

	ps_vma->vm_page_prot = sPageProt;

	OSSetVMAFlags(ps_vma, VM_IO);

	/* Don't include the mapping in core dumps */
	OSSetVMAFlags(ps_vma, VM_DONTDUMP);

	/*
	 * Disable mremap because our nopage handler assumes all
	 * page requests have already been validated.
	 */
	OSSetVMAFlags(ps_vma, VM_DONTEXPAND);

	/* Don't allow mapping to be inherited across a process fork */
	OSSetVMAFlags(ps_vma, VM_DONTCOPY);

	uiLength = ps_vma->vm_end - ps_vma->vm_start;

	/* Is this mmap targeting non order-zero pages or does it use pfn mappings?
	 * If yes, don't use vm_insert_page */
	uiLog2PageSize = PMR_GetLog2Contiguity(psPMR);

#if defined(PMR_OS_USE_VM_INSERT_PAGE)
	/*
	 * user memory is an anonymous page, and cannot be remapped with insert page,
	 * and remap_pfn_range must be used.
	 */
	if (PMR_IsVram(psPMR))
	{
		bUseVMInsertPage = (uiLog2PageSize == PAGE_SHIFT) &&
				   (PMR_GetType(psPMR) != PMR_TYPE_EXTMEM);
	}
	else if (PMR_IsSystem(psPMR))
	{
		bUseVMInsertPage = (PMR_GetType(psPMR) != PMR_TYPE_EXTMEM);
	}
#endif

	/* Can we use stack allocations */
	uiNumOfPFNs = uiLength >> uiLog2PageSize;
	if (uiNumOfPFNs > PMR_MAX_TRANSLATION_STACK_ALLOC)
	{
		psCpuPAddr = MemoryPoolAllocDefault(uiNumOfPFNs * sizeof(*psCpuPAddr));
		if (psCpuPAddr == NULL)
		{
			eError = PVRSRV_ERROR_OUT_OF_MEMORY;
			goto e1;
		}

		/* Should allocation fail, clean-up here before exiting */
		pbValid = MemoryPoolAllocDefault(uiNumOfPFNs * sizeof(*pbValid));
		if (pbValid == NULL)
		{
			eError = PVRSRV_ERROR_OUT_OF_MEMORY;
			MemoryPoolFreeDefault(psCpuPAddr);
			goto e2;
		}
	}
	else
	{
		psCpuPAddr = asCpuPAddr;
		pbValid = abValid;
	}

	/* Obtain map range pfns */
	eError = PMR_CpuPhysAddr(psPMR,
				 uiLog2PageSize,
				 uiNumOfPFNs,
				 0,
				 psCpuPAddr,
				 pbValid);
	if (eError != PVRSRV_OK)
	{
		goto e3;
	}

	/*
	 * Scan the map range for pfns without struct page* handling. If
	 * we find one, this is a mixed map, and we can't use vm_insert_page()
	 * NOTE: vm_insert_page() allows insertion of individual pages into user
	 * VMA space _only_ if said page is an order-zero allocated page.
	 */
	if (bUseVMInsertPage)
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
		pfn_t sPFN;
#else
		unsigned long uiPFN;
#endif

		for (uiOffsetIdx = 0; uiOffsetIdx < uiNumOfPFNs; ++uiOffsetIdx)
		{
			if (pbValid[uiOffsetIdx])
			{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
				sPFN = phys_to_pfn_t(psCpuPAddr[uiOffsetIdx].uiAddr, 0);

				if (!pfn_t_valid(sPFN) || page_count(pfn_t_to_page(sPFN)) == 0)
#else
				uiPFN = psCpuPAddr[uiOffsetIdx].uiAddr >> PAGE_SHIFT;
				PVR_ASSERT(((IMG_UINT64)uiPFN << PAGE_SHIFT) == psCpuPAddr[uiOffsetIdx].uiAddr);

				if (!pfn_valid(uiPFN) || page_count(pfn_to_page(uiPFN)) == 0)
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) */
				{
					bUseMixedMap = IMG_TRUE;
					break;
				}
			}
		}

		if (bUseMixedMap)
		{
			OSSetVMAFlags(ps_vma, VM_MIXEDMAP);
		}
	}
	else
	{
		OSSetVMAFlags(ps_vma, VM_PFNMAP);
	}

	/*
	 * For SYSTEM PMR, VM_PFNMAP should not be included,
	 * ui32LoopCount could be bigger than 1.
	 */
	if (PMR_IsSystem(psPMR))
	{
		OSClearVMAFlags(ps_vma, VM_PFNMAP);
		ui32LoopCount = (1 << (uiLog2PageSize - PAGE_SHIFT));
	}

	/* For each PMR page-size contiguous bytes, map page(s) into user VMA */
	for (ui64OffsetByGPUPage = 0; ui64OffsetByGPUPage < uiLength; ui64OffsetByGPUPage += (1ULL << uiLog2PageSize))
	{
		PVR_ASSERT(uiLog2PageSize >= PAGE_SHIFT);
		uiOffsetIdx = ui64OffsetByGPUPage >> uiLog2PageSize;
		ui64VmOffset = ui64OffsetByGPUPage;
		sCpuPAddr.uiAddr = psCpuPAddr[uiOffsetIdx].uiAddr;

		/*
		 * Only map in pages that are valid, any that aren't will be
		 * picked up by the nopage handler which will return a zeroed
		 * page for us.
		 */
		if (pbValid[uiOffsetIdx])
		{
			for (i = 0; i < ui32LoopCount; i++)
			{
				iStatus = _OSMMapPMR(psDevNode,
						     ps_vma,
						     ui64VmOffset,
						     &sCpuPAddr,
						     uiLog2PageSize,
						     bUseVMInsertPage,
						     bUseMixedMap);
				ui64VmOffset += PAGE_SIZE;
				sCpuPAddr.uiAddr += PAGE_SIZE;
				if (iStatus)
				{
					/* Failure error code doesn't get propagated */
					eError = PVRSRV_ERROR_PMR_CPU_PAGE_MAP_FAILED;
					PVR_ASSERT(0);
					goto e3;
				}
			}
		}
#if defined(PVRSRV_ENABLE_PROCESS_STATS) && defined(PVRSRV_ENABLE_MEMORY_STATS)
#define PMR_OS_BAD_CPUADDR 0x0BAD0BAD
		{
			IMG_CPU_PHYADDR sPAddr;
			sPAddr.uiAddr = pbValid[uiOffsetIdx] ?
					psCpuPAddr[uiOffsetIdx].uiAddr :
					IMG_CAST_TO_CPUPHYADDR_UINT(PMR_OS_BAD_CPUADDR);

			PVRSRVStatsAddMemAllocRecord(PVRSRV_MEM_ALLOC_TYPE_MAP_UMA_LMA_PAGES,
						     (void *)(uintptr_t)(ps_vma->vm_start + ui64OffsetByGPUPage),
						     sPAddr,
						     1 << uiLog2PageSize,
						     NULL,
						     OSGetCurrentClientProcessIDKM(),
						     psDevNode
						     DEBUG_MEMSTATS_VALUES);
		}
#undef PMR_OS_BAD_CPUADDR
#endif
	}

#if defined(PVRSRV_ENABLE_PROCESS_STATS) && !defined(PVRSRV_ENABLE_MEMORY_STATS)
	PVRSRVStatsIncrMemAllocStat(PVRSRV_MEM_ALLOC_TYPE_MAP_UMA_LMA_PAGES,
				    uiNumOfPFNs * PAGE_SIZE,
				    OSGetCurrentClientProcessIDKM(),
				    psDevNode);
#endif

	if (psCpuPAddr != asCpuPAddr)
	{
		MemoryPoolFreeDefault(psCpuPAddr);
		MemoryPoolFreeDefault(pbValid);
	}

	/* let us see the PMR so we can unlock it later */
	ps_vma->vm_private_data = psPMR;

	/* Install open and close handlers for ref-counting */
	ps_vma->vm_ops = &gsMMapOps;

	/*
	 * Take a reference on the PMR so that it can't be freed while mapped
	 * into the user process.
	 */
	PMRRefPMR(psPMR);

#if defined(PVRSRV_ENABLE_LINUX_MMAP_STATS)
	/* record the stats */
	MMapStatsAddOrUpdatePMR(psPMR, uiLength);
#endif

	return PVRSRV_OK;

	/* Error exit paths follow */
e3:
	if (pbValid != abValid)
	{
		MemoryPoolFreeDefault(pbValid);
	}
e2:
	if (psCpuPAddr != asCpuPAddr)
	{
		MemoryPoolFreeDefault(psCpuPAddr);
	}
e1:
	PMRUnlockSysPhysAddresses(psPMR);
e0:
	return eError;
}
