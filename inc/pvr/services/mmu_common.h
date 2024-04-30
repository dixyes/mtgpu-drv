/*************************************************************************/ /*!
@File
@Title          Common MMU Management
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Implements basic low level control of MMU.
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

#ifndef MMU_COMMON_H
#define MMU_COMMON_H

/*
	The Memory Management Unit (MMU) performs device virtual to physical
	translation.

	Terminology:
	 - page catalogue, PC	(optional, 3 tier MMU)
	 - page directory, PD
	 - page table, PT (can be variable sized)
	 - data page, DP (can be variable sized)
	Note: PD and PC are fixed size and can't be larger than the native
	      physical (CPU) page size
	Shifts and AlignShift variables:
	 - 'xxxShift' represent the number of bits a bitfield is shifted left from bit0
	 - 'xxxAlignShift' is used to convert a bitfield (based at bit0) into byte units
	   by applying a bit shift left by 'xxxAlignShift' bits
*/

/*
	Device Virtual Address Config:

	Incoming Device Virtual Address is deconstructed into up to 4
	fields, where the virtual address is up to 64bits:
	MSB-----------------------------------------------LSB
	| PC Index:   | PD Index:  | PT Index: | DP offset: |
	| d bits      | c bits     | b-v bits  |  a+v bits  |
	-----------------------------------------------------
	where v is the variable page table modifier, e.g.
			v == 0 -> 4KB DP
			v == 2 -> 16KB DP
			v == 4 -> 64KB DP
			v == 6 -> 256KB DP
			v == 8 -> 1MB DP
			v == 10 -> 4MB DP
*/

/* services/server/include/ */
#include "pmr.h"

/* include/ */
#include "img_types.h"
#include "img_defs.h"
#include "pvr_notifier.h"
#include "pvrsrv_error.h"
#include "servicesext.h"

#include "ra.h"
#include "dllist.h"
#include "lock_types.h"
#include "sync_prim_internal.h"
#include "pvrsrv_cleanup.h"
#include "hash.h"

#define UNITS_IN_BITFIELD(Mask, Shift)	((Mask >> Shift) + 1)

/*!
	The level of the MMU
*/
typedef enum
{
	MMU_LEVEL_0 = 0,	/* Level 0 = Page */

	MMU_LEVEL_1,
	MMU_LEVEL_2,
	MMU_LEVEL_3,
	MMU_LEVEL_4,
	MMU_LEVEL_LAST
} MMU_LEVEL;


#define MMU_MAX_LEVEL 4

typedef struct _MMU_LEVEL_DATA_
{
	IMG_UINT32	ui32Index;
	IMG_UINT32	ui32NumOfEntries;
	IMG_CHAR const	*psDebugStr;
	IMG_UINT8	uiBytesPerEntry;
	IMG_UINT64	ui64Address;
} MMU_LEVEL_DATA;

typedef enum _MMU_FAULT_TYPE_
{
	MMU_FAULT_TYPE_UNKNOWN = 0, /* If fault is not analysed by Host */
	MMU_FAULT_TYPE_PM,
	MMU_FAULT_TYPE_NON_PM,
} MMU_FAULT_TYPE;

typedef struct _MMU_FAULT_DATA_
{
	MMU_LEVEL	eTopLevel;
	MMU_FAULT_TYPE	eType;
	MMU_LEVEL_DATA	sLevelData[MMU_LEVEL_LAST];
} MMU_FAULT_DATA;

struct _MMU_DEVVADDR_CONFIG_;

/*!
	MMU virtual address split
*/
typedef struct _MMU_DEVVADDR_CONFIG_
{
	/*! Page catalogue index mask */
	IMG_UINT64	uiPCIndexMask;
	/*! Page catalogue index shift */
	IMG_UINT8	uiPCIndexShift;
	/*! Total number of PC entries */
	IMG_UINT32	uiNumEntriesPC;
	/*! Page directory ptr mask */
	IMG_UINT64	uiPDPtrIndexMask;
	/*! Page directory ptr shift */
	IMG_UINT8	uiPDPtrIndexShift;
	/*! Total number of PDP entries */
	IMG_UINT32	uiNumEntriesPDPtr;
	/*! Page directory mask */
	IMG_UINT64	uiPDIndexMask;
	/*! Page directory shift */
	IMG_UINT8	uiPDIndexShift;
	/*! Total number of PD entries */
	IMG_UINT32	uiNumEntriesPD;
	/*! Page table mask */
	IMG_UINT64	uiPTIndexMask;
	/*! Page index shift */
	IMG_UINT8	uiPTIndexShift;
	/*! Total number of PT entries */
	IMG_UINT32	uiNumEntriesPT;
	/*! Page offset mask */
	IMG_UINT64	uiPageOffsetMask;
	/*! Page offset shift */
	IMG_UINT8	uiPageOffsetShift;
	/*! First virtual address mappable for this config */
	IMG_UINT64	uiOffsetInBytes;

} MMU_DEVVADDR_CONFIG;

/*
	P(C/D/T) Entry Config:

	MSB-----------------------------------------------LSB
	| PT Addr:   | variable PT ctrl | protection flags: |
	| bits c+v   | b bits           | a bits            |
	-----------------------------------------------------
	where v is the variable page table modifier and is optional
*/
/*!
	Generic MMU entry description. This is used to describe PC, PD and PT entries.
*/
typedef struct _MMU_PxE_CONFIG_
{
	IMG_UINT8	uiBytesPerEntry; /*! Size of an entry in bytes */

	IMG_UINT64	uiAddrMask;      /*! Physical address mask */
	IMG_UINT8	uiAddrShift;     /*! Physical address shift */
	IMG_UINT8	uiAddrLog2Align; /*! Physical address Log 2 alignment */

	IMG_UINT64	uiVarCtrlMask;   /*! Variable control mask */
	IMG_UINT8	uiVarCtrlShift;  /*! Variable control shift */

	IMG_UINT64	uiProtMask;      /*! Protection flags mask */
	IMG_UINT8	uiProtShift;     /*! Protection flags shift */

	IMG_UINT64	uiValidEnMask;   /*! Entry valid bit mask */
	IMG_UINT8	uiValidEnShift;  /*! Entry valid bit shift */
} MMU_PxE_CONFIG;

/* MMU Protection flags */

/* These are specified generically and in a h/w independent way, and
   are interpreted at each level (PC/PD/PT) separately. */

/* The following flags are for internal use only, and should not
   traverse the API */
#define MMU_PROTFLAGS_VALID					(1U << 0)
#define MMU_PROTFLAGS_READABLE					(1U << 1)
#define MMU_PROTFLAGS_WRITEABLE					(1U << 2)
#define MMU_PROTFLAGS_CACHE_COHERENT				(1U << 3)
#define MMU_PROTFLAGS_CACHED					(1U << 4)
#define MMU_PROTFLAGS_UNMAP					(1U << 5)
#define MMU_PORTFLAGS_ATOMIC					(1U << 6)

typedef IMG_UINT32 MMU_PROTFLAGS_T;

/* Device specific flags*/
#define MMU_PROTFLAGS_DEVICE_OFFSET		16
#define MMU_PROTFLAGS_DEVICE_MASK		0x000f0000UL
#define MMU_PROTFLAGS_DEVICE(n)	\
			(((n) << MMU_PROTFLAGS_DEVICE_OFFSET) & \
			MMU_PROTFLAGS_DEVICE_MASK)

typedef struct _MMU_CONTEXT_ MMU_CONTEXT;

struct _PVRSRV_DEVICE_NODE_;

struct _CONNECTION_DATA_;

typedef struct _MMU_PAGESIZECONFIG_
{
	const MMU_PxE_CONFIG *psPDPtrEConfig;
	const MMU_PxE_CONFIG *psPDEConfig;
	const MMU_PxE_CONFIG *psPTEConfig;
	const MMU_DEVVADDR_CONFIG *psDevVAddrConfig;
	IMG_UINT32 uiRefCount;
	IMG_UINT32 uiMaxRefCount;
} MMU_PAGESIZECONFIG;

/*!
 * Refcounted structure that is shared between the context and
 * the cleanup thread items.
 * It is used to keep track of all cleanup items and whether the creating
 * MMU context has been destroyed and therefore is not allowed to be
 * accessed any more.
 *
 * The cleanup thread is used to defer the freeing of the page tables
 * because we have to make sure that the MMU cache has been invalidated.
 * If we don't take care of this the MMU might partially access cached
 * and uncached tables which might lead to inconsistencies and in the
 * worst case to MMU pending faults on random memory.
 */
typedef struct _MMU_CTX_CLEANUP_DATA_
{
	/* Refcount to know when this structure can be destroyed */
	ATOMIC_T iRef;
	/* Protect items in this structure, especially the refcount */
	POS_LOCK hCleanupLock;
	/* List of all cleanup items currently in flight */
	DLLIST_NODE sMMUCtxCleanupItemsHead;
	/* Was the MMU context destroyed and should not be accessed any more? */
	IMG_BOOL bMMUContextExists;
#if defined(SUPPORT_GPUVIRT_VALIDATION)
	/* Associated OSid for this context */
	IMG_UINT32 ui32OSid;
#endif	/* defined(SUPPORT_GPUVIRT_VALIDATION) */
} MMU_CTX_CLEANUP_DATA;

/*!
 * All physical allocations and frees are relative to this context, so
 * we would get all the allocations of PCs, PDs, and PTs from the same
 * RA.
 *
 * We have one per MMU context in case we have mixed UMA/LMA devices
 * within the same system.
 */
typedef struct _MMU_PHYSMEM_CONTEXT_
{
	/* Associated MMU_CONTEXT */
	struct _MMU_CONTEXT_ *psMMUContext;
	/* Parent device node */
	struct _PVRSRV_DEVICE_NODE_ *psDevNode;
	/* Refcount so we know when to free up the arena */
	IMG_UINT32 uiNumAllocations;
	/* Arena from which physical memory is derived */
	RA_ARENA *psPhysMemRA;
	/* Arena name */
	IMG_CHAR *pszPhysMemRAName;
	/* Size of arena name string */
	size_t uiPhysMemRANameAllocSize;
	/* Meta data for deferred cleanup */
	MMU_CTX_CLEANUP_DATA *psCleanupData;
	/* Temporary list of all deferred MMU_MEMORY_MAPPINGs. */
	DLLIST_NODE sTmpMMUMappingHead;
#if defined(SUPPORT_GPUVIRT_VALIDATION)
	IMG_UINT32 ui32OSid;
	IMG_UINT32 ui32OSidReg;
	IMG_BOOL   bOSidAxiProt;
#endif
} MMU_PHYSMEM_CONTEXT;

/*
 * Mapping structure for MMU memory allocation
 */
typedef struct _MMU_MEMORY_MAPPING_
{
	/* Physmem context to allocate from */
	MMU_PHYSMEM_CONTEXT *psContext;
	/* OS/system Handle for this allocation */
	PG_HANDLE sMemHandle;
	/* CPU virtual address of this allocation */
	void *pvCpuVAddr;
	/* Device physical address of this allocation */
	IMG_DEV_PHYADDR	sDevPAddr;
	/* Size of this allocation */
	size_t uiSize;
	/* Number of current mappings of this allocation */
	IMG_UINT32 uiCpuVAddrRefCount;
	/* Node for the defer free list */
	DLLIST_NODE sMMUMappingItem;
} MMU_MEMORY_MAPPING;

/*
 * Memory descriptor for MMU objects. There can be more than one memory
 * descriptor per MMU memory allocation.
 */
typedef struct _MMU_MEMORY_DESC_
{
	/* NB: bValid is set if this descriptor describes physical
	 * memory.  This allows "empty" descriptors to exist, such that we
	 * can allocate them in batches.
	 */
	/* Does this MMU object have physical backing */
	IMG_BOOL bValid;
	/* Device Physical address of physical backing */
	IMG_DEV_PHYADDR sDevPAddr;
	/* CPU virtual address of physical backing */
	void *pvCpuVAddr;
	/* Mapping data for this MMU object */
	MMU_MEMORY_MAPPING *psMapping;
	/* Memdesc offset into the psMapping */
	IMG_UINT32 uiOffset;
	/* Size of the Memdesc */
	IMG_UINT32 uiSize;
} MMU_MEMORY_DESC;

/*
 * MMU levelx structure. This is generic and is used
 * for all levels (PC, PD, PT).
 */
typedef struct _MMU_Levelx_INFO_
{
	/* The Number of entries in this level */
	IMG_UINT32 ui32NumOfEntries;
	/* Number of times this level has been reference. Note: For Level1 (PTE)
	 * we still take/drop the reference when setting up the page tables rather
	 * then at map/unmap time as this simplifies things
	 */
	IMG_UINT32 ui32RefCount;
	/* MemDesc for this level */
	MMU_MEMORY_DESC sMemDesc;
	/* backing of page table device memory in system memory */
	void *pvPageTableBacking;
	/* MMU Entry description of current level */
	const MMU_PxE_CONFIG *psConfig;
	/* MMU Level */
	MMU_LEVEL eMMULevel;
	/* Previous level info */
	struct _MMU_Levelx_INFO_ *psPrevLevel;
	/* Index in previous level */
	IMG_UINT32 ui32PrevPxEIndex;
	/* The Number of entries with unmap bit flag in this level */
	IMG_UINT32 ui32NumOfUnmapEntries;
	/* Array of infos for the next level. Must be last member in structure */
	struct _MMU_Levelx_INFO_ *apsNextLevel[1];
} MMU_Levelx_INFO;

/*!
 * MMU device attributes. This structure is the interface between the generic
 * MMU code and the device specific MMU code.
 */
typedef struct _MMU_DEVICEATTRIBS_
{
	PDUMP_MMU_TYPE eMMUType;
	IMG_CHAR *pszMMUPxPDumpMemSpaceName;
	/* The type of the top level object */
	MMU_LEVEL eTopLevel;
	/* Alignment requirement of the base object */
	IMG_UINT32 ui32BaseAlign;
	/* HW config of the base object */
	const struct _MMU_PxE_CONFIG_ *psBaseConfig;
	/* Address split for the base object */
	const struct _MMU_DEVVADDR_CONFIG_ *psTopLevelDevVAddrConfig;
	/* Callback for creating protection bits for the page catalogue entry with 8 byte entry */
	IMG_UINT64 (*pfnDerivePCEProt8)(IMG_UINT32 uiProtFlags, IMG_UINT32 uiLog2DataPageSize);
	/* Callback for creating protection bits for the page catalogue entry with 4 byte entry */
	IMG_UINT32 (*pfnDerivePCEProt4)(IMG_UINT32 uiProtFlags);
	/* Callback for creating protection bits for the page directory ptr entry with 8 byte entry */
	IMG_UINT64 (*pfnDerivePDPtrEProt8)(IMG_UINT32 uiProtFlags, IMG_UINT32 uiLog2DataPageSize);
	/* Callback for creating protection bits for the page directory entry with 8 byte entry */
	IMG_UINT64 (*pfnDerivePDEProt8)(IMG_UINT32 uiProtFlags, IMG_UINT32 uiLog2DataPageSize);
	/* Callback for creating protection bits for the page directory entry with 4 byte entry */
	IMG_UINT32 (*pfnDerivePDEProt4)(IMG_UINT32 uiProtFlags);
	/* Callback for creating protection bits for the page table entry with 8 byte entry */
	IMG_UINT64 (*pfnDerivePTEProt8)(IMG_UINT32 uiProtFlags, IMG_UINT32 uiLog2DataPageSize);
	/* Callback for creating protection bits for the page table entry with 4 byte entry */
	IMG_UINT32 (*pfnDerivePTEProt4)(IMG_UINT32 uiProtFlags);
	/* Callback for getting the MMU configuration based on the specified page size */
	PVRSRV_ERROR (*pfnGetPageSizeConfiguration)(IMG_UINT32 ui32DataPageSize,
						    const struct _MMU_PxE_CONFIG_ **ppsMMUPDPtrEConfig,
						    const struct _MMU_PxE_CONFIG_ **ppsMMUPDEConfig,
						    const struct _MMU_PxE_CONFIG_ **ppsMMUPTEConfig,
						    const struct _MMU_DEVVADDR_CONFIG_ **ppsMMUDevVAddrConfig,
						    IMG_HANDLE *phPriv2);
	/* Callback for putting the MMU configuration obtained from pfnGetPageSizeConfiguration */
	PVRSRV_ERROR (*pfnPutPageSizeConfiguration)(IMG_HANDLE hPriv);

	/* Callback for getting the page size from the PDE for the page table entry with 4 byte entry */
	PVRSRV_ERROR (*pfnGetPageSizeFromPDE4)(IMG_UINT32, IMG_UINT32 *);
	/* Callback for getting the page size from the PDE for the page table entry with 8 byte entry */
	PVRSRV_ERROR (*pfnGetPageSizeFromPDE8)(IMG_UINT64, IMG_UINT32 *);
	/* Callback for getting the page size directly from the address. Supported on MMU4 */
	PVRSRV_ERROR (*pfnGetPageSizeFromVirtAddr)(struct _PVRSRV_DEVICE_NODE_ *psDevNode, IMG_DEV_VIRTADDR, IMG_UINT32 *);
	PVRSRV_ERROR (*pfnGetDerivePxEProtPFN)(MMU_LEVEL eMMULevel,
					       IMG_UINT32 (**ppfnDerivePxEProt4)(IMG_UINT32),
					       IMG_UINT64 (**ppfnDerivePxEProt8)(IMG_UINT32, IMG_UINT32));
	IMG_UINT32 (*pfnGetHeapPageShift)(IMG_UINT32 uiLog2OSPageSize);
	IMG_UINT64 (*pfnGetPageFlagByPageShift)(IMG_UINT32 uiLog2DataPageSize);
	void (*pfnGetLevelData)(IMG_DEV_VIRTADDR sDevVAddrStart,
				IMG_DEV_VIRTADDR sDevVAddrEnd,
				IMG_UINT32 uiLog2DataPageSize,
				IMG_UINT32 auiStartArray[],
				IMG_UINT32 auiEndArray[],
				IMG_UINT32 auiEntriesPerPx[],
				const MMU_PxE_CONFIG *apsConfig[],
				MMU_LEVEL aeMMULevel[],
				const MMU_DEVVADDR_CONFIG **ppsMMUDevVAddrConfig,
				IMG_HANDLE *phPriv);
	void (*pfnGetPTInfo)(MMU_CONTEXT        *psMMUContext,
			     IMG_DEV_VIRTADDR            sDevVAddr,
			     IMG_UINT32                  ui32Log2PageSize,
			     const MMU_DEVVADDR_CONFIG  *psDevVAddrConfig,
			     MMU_Levelx_INFO           **ppsLevel,
			     IMG_UINT32                 *pui32PTEIndex);
	IMG_UINT32 (*pfnCalPTContinous)(IMG_DEV_VIRTADDR *psDevVAddr, IMG_UINT64 uiPageSize,
                                        IMG_UINT32 uiLeftLength, IMG_UINT64 *ui64Port);
	IMG_UINT64 (*pfnGetConfigRangeValue)(IMG_UINT32 ui32DataPageSize, IMG_UINT64 ui64BaseAddress, IMG_UINT64 ui64RangeSize);
	/* Private data handle */
	IMG_HANDLE hGetPageSizeFnPriv;
} MMU_DEVICEATTRIBS;

/*
 * MMU context structure
 */
struct _MMU_CONTEXT_
{
	/* Originating Connection */
	struct _CONNECTION_DATA_ *psConnection;
	MMU_DEVICEATTRIBS *psDevAttrs;
	/*
	 * For allocation and deallocation of the physical memory where
	 * the pagetables live
	 */
	struct _MMU_PHYSMEM_CONTEXT_ *psPhysMemCtx;
#if defined(PDUMP)
	/* PDump context ID (required for PDump commands with virtual addresses) */
	IMG_UINT32 uiPDumpContextID;
	/* The refcount of the PDump context ID */
	IMG_UINT32 ui32PDumpContextIDRefCount;
#endif
	/* MMU cache invalidation flags (only used on Volcanic driver) */
	ATOMIC_T sCacheFlags;
	/*
	 * Lock to ensure exclusive access when manipulating the MMU context or
	 * reading and using its content
	 */
	POS_LOCK hLock;

	/* dev virtual address and phys address mapping */
	POS_LOCK hHashLock;
	HASH_TABLE *psHashTab;

	IMG_UINT64 ui64PageSizeFlag;

	/* Base level info structure. Must be last member in structure */
	MMU_Levelx_INFO sBaseLevelInfo;
};

/*************************************************************************/ /*!
@Function       MMU_ContextCreate

@Description    Create a new MMU context

@Input          psConnection            Connection requesting the MMU context
                                        creation. Can be NULL for kernel/FW
                                        memory context.
@Input          psDevNode               Device node of the device to create the
                                        MMU context for
@Output         ppsMMUContext           The created MMU context

@Return         PVRSRV_OK if the MMU context was successfully created
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_ContextCreate(struct _CONNECTION_DATA_ *psConnection,
		  struct _PVRSRV_DEVICE_NODE_ *psDevNode,
		  MMU_CONTEXT **ppsMMUContext,
		  IMG_BOOL bKernelMemoryCtx,
		  MMU_DEVICEATTRIBS *psDevAttrs);


/*************************************************************************/ /*!
@Function       MMU_ContextDestroy

@Description    Destroy a MMU context

@Input          psMMUContext            MMU context to destroy

@Return         None
*/
/*****************************************************************************/
void
MMU_ContextDestroy(MMU_CONTEXT *psMMUContext);

/*************************************************************************/ /*!
@Function       MMU_Alloc

@Description    Allocate the page tables required for the specified virtual range

@Input          psMMUContext            MMU context to operate on

@Input          uSize                   The size of the allocation

@Output         puActualSize            Actual size of allocation

@Input          uiProtFlags             Generic MMU protection flags

@Input          uDevVAddrAlignment      Alignment requirement of the virtual
                                        allocation

@Input          psDevVAddr              Virtual address to start the allocation
                                        from

@Return         PVRSRV_OK if the allocation of the page tables was successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_Alloc(MMU_CONTEXT *psMMUContext,
          IMG_DEVMEM_SIZE_T uSize,
          IMG_DEVMEM_SIZE_T *puActualSize,
          IMG_UINT32 uiProtFlags,
          IMG_DEVMEM_SIZE_T uDevVAddrAlignment,
          IMG_DEV_VIRTADDR *psDevVAddr,
          IMG_UINT32 uiLog2PageSize);


/*************************************************************************/ /*!
@Function       MMU_Free

@Description    Free the page tables of the specified virtual range

@Input          psMMUContext            MMU context to operate on

@Input          sDevVAddr               Virtual address to start the free
                                        from

@Input          uiSize                  The size of the allocation

@Return         None
*/
/*****************************************************************************/
void
MMU_Free(MMU_CONTEXT *psMMUContext,
         IMG_DEV_VIRTADDR sDevVAddr,
         IMG_DEVMEM_SIZE_T uiSize,
         IMG_UINT32 uiLog2DataPageSize);


/*************************************************************************/ /*!
@Function       MMU_MapPages

@Description    Map pages to the MMU.
                Two modes of operation: One requires a list of physical page
                indices that are going to be mapped, the other just takes
                the PMR and a possible offset to map parts of it.

@Input          psMMUContext            MMU context to operate on

@Input          uiMappingFlags          Memalloc flags for the mapping

@Input          sDevVAddrBase           Device virtual address of the 1st page

@Input          psPMR                   PMR to map

@Input          ui32PhysPgOffset        Physical offset into the PMR

@Input          ui32MapPageCount        Number of pages to map

@Input          paui32MapIndices        List of page indices to map,
                                         can be NULL

@Input          uiLog2PageSize          Log2 page size of the pages to map

@Return         PVRSRV_OK if the mapping was successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_MapPages(MMU_CONTEXT *psMMUContext,
	     PVRSRV_MEMALLOCFLAGS_T uiMappingFlags,
	     IMG_DEV_VIRTADDR sDevVAddrBase,
	     PMR *psPMR,
	     IMG_UINT32 ui32PhysPgOffset,
	     IMG_UINT32 ui32MapPageCount,
	     IMG_UINT32 *paui32MapIndices,
	     IMG_UINT32 uiLog2PageSize);

/*************************************************************************/ /*!
@Function       MMU_UnmapPages

@Description    Unmap pages from the MMU.

@Input          psMMUContext            MMU context to operate on

@Input          uiMappingFlags          Memalloc flags for the mapping

@Input          sDevVAddr               Device virtual address of the 1st page

@Input          ui32PageCount           Number of pages to unmap

@Input          pai32UnmapIndicies      Array of page indices to be unmapped

@Input          uiLog2PageSize          log2 size of the page


@Input          uiMemAllocFlags         Indicates if the unmapped regions need
                                        to be backed by dummy or zero page

@Return         None
*/
/*****************************************************************************/
void
MMU_UnmapPages(MMU_CONTEXT *psMMUContext,
               PVRSRV_MEMALLOCFLAGS_T uiMappingFlags,
               IMG_DEV_VIRTADDR sDevVAddr,
               IMG_UINT32 ui32PageCount,
               IMG_UINT32 *pai32UnmapIndicies,
               IMG_UINT32 uiLog2PageSize,
               PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags);

/*************************************************************************/ /*!
@Function       MMU_MapPMRFast

@Description    Map a PMR into the MMU. Must be not sparse.
                This is supposed to cover most mappings and, as the name suggests,
                should be as fast as possible.

@Input          psMMUContext            MMU context to operate on

@Input          sDevVAddr               Device virtual address to map the PMR
                                        into

@Input          psPMR                   PMR to map

@Input          uiPageNums              page number

@Input          uiMappingFlags          Memalloc flags for the mapping

@Return         PVRSRV_OK if the PMR was successfully mapped
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_MapPMRFast(MMU_CONTEXT *psMMUContext,
	       IMG_DEV_VIRTADDR sDevVAddr,
	       const PMR *psPMR,
	       IMG_UINT32 uiPageNums,
	       PVRSRV_MEMALLOCFLAGS_T uiMappingFlags,
	       IMG_UINT32 uiLog2PageSize,
	       DEVMEM_INTERLEAVE_RATIO *psInterleaveRatio);

/*************************************************************************/ /*!
@Function       MMU_UnmapPMRFast

@Description    Unmap pages from the MMU as fast as possible.
                PMR must be non-sparse!

@Input          psMMUContext            MMU context to operate on

@Input          sDevVAddrBase           Device virtual address of the 1st page

@Input          ui32PageCount           Number of pages to unmap

@Input          uiLog2PageSize          log2 size of the page

@Return         None
*/
/*****************************************************************************/
void
MMU_UnmapPMRFast(MMU_CONTEXT *psMMUContext,
                 IMG_DEV_VIRTADDR sDevVAddrBase,
                 IMG_UINT32 ui32PageCount,
                 IMG_UINT32 uiLog2PageSize);

/*************************************************************************/ /*!
@Function       MMU_ChangeValidity

@Description    Sets or unsets the valid bit of page table entries for a given
                address range.

@Input          psMMUContext            MMU context to operate on

@Input          sDevVAddr               The device virtual base address of
                                        the range we want to modify

@Input          uiSizeBytes             The size of the range in bytes

@Input          uiLog2PageSize          Log2 of the used page size

@Input          bMakeValid              Choose to set or unset the valid bit.
                                        (bMakeValid == IMG_TRUE ) -> SET
                                        (bMakeValid == IMG_FALSE) -> UNSET

@Input          psPMR                   The PMR backing the allocation.
                                        Needed in case we have sparse memory
                                        where we have to check whether a physical
                                        address actually backs the virtual.

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_ChangeValidity(MMU_CONTEXT *psMMUContext,
                   IMG_DEV_VIRTADDR sDevVAddr,
                   IMG_DEVMEM_SIZE_T uiSizeBytes,
                   IMG_UINT32 uiLog2PageSize,
                   IMG_BOOL bMakeValid,
                   PMR *psPMR);

/*************************************************************************/ /*!
@Function       MMU_AcquireBaseAddr

@Description    Acquire the device physical address of the base level MMU object

@Input          psMMUContext            MMU context to operate on

@Output         psPhysAddr              Device physical address of the base level
                                        MMU object

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_AcquireBaseAddr(MMU_CONTEXT *psMMUContext, IMG_DEV_PHYADDR *psPhysAddr);

/*************************************************************************/ /*!
@Function       MMU_AcquireCPUBaseAddr

@Description    Acquire the CPU Virtual Address of the base level MMU object

@Input          psMMUContext            MMU context to operate on

@Output         ppvCPUVAddr             CPU Virtual Address of the base level
                                        MMU object

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_AcquireCPUBaseAddr(MMU_CONTEXT *psMMUContext, void **ppvCPUVAddr);

/*************************************************************************/ /*!
@Function       MMU_ReleaseBaseAddr

@Description    Release the device physical address of the base level MMU object

@Input          psMMUContext            MMU context to operate on

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
void
MMU_ReleaseBaseAddr(MMU_CONTEXT *psMMUContext);

#if defined(SUPPORT_GPUVIRT_VALIDATION)
/***********************************************************************************/ /*!
@Function       MMU_SetOSid

@Description    Set the OSid associated with the application (and the MMU Context)

@Input          psMMUContext            MMU context to store the OSid on

@Input          ui32OSid                the OSid in question

@Input          ui32OSidReg             The value that the firmware will assign to the
                                        registers.

@Input          bOSidAxiProt            Toggles whether the AXI prot bit will be set or
                                        not.
@Return None
*/
/***********************************************************************************/

void MMU_SetOSids(MMU_CONTEXT *psMMUContext, IMG_UINT32 ui32OSid,
                  IMG_UINT32 ui32OSidReg, IMG_BOOL bOSidAxiProt);

/***********************************************************************************/ /*!
@Function       MMU_GetOSid

@Description    Retrieve the OSid associated with the MMU context.

@Input          psMMUContext            MMU context in which the OSid is stored

@Output         pui32OSid               The OSid in question

@Output         pui32OSidReg            The OSid that the firmware will assign to the
                                        registers.

@Output         pbOSidAxiProt           Toggles whether the AXI prot bit will be set or
                                        not.
@Return None
*/
/***********************************************************************************/

void MMU_GetOSids(MMU_CONTEXT *psMMUContext, IMG_UINT32 * pui32OSid,
                  IMG_UINT32 * pui32OSidReg, IMG_BOOL *pbOSidAxiProt);
#endif

/*************************************************************************/ /*!
@Function       MMU_AppendCacheFlags

@Description    Set the cache flags to the bitwise or of themselves and the
                specified input flags, i.e. ui32CacheFlags |= ui32NewCacheFlags,
                atomically.

@Input          psMMUContext            MMU context

@Input          ui32NewCacheFlags       Cache flags to append.

@Return         None
*/
/*****************************************************************************/
void MMU_AppendCacheFlags(MMU_CONTEXT *psMMUContext, IMG_UINT32 ui32NewCacheFlags);

/*************************************************************************/ /*!
@Function       MMU_ExchangeCacheFlags

@Description    Exchange MMU context flags with specified value, atomically.

@Input          psMMUContext            MMU context

@Input          ui32CacheFlags          Cache flags to set.

@Return         Previous MMU context cache flags.
*/
/*****************************************************************************/
IMG_UINT32 MMU_ExchangeCacheFlags(MMU_CONTEXT *psMMUContext, IMG_UINT32 ui32NewCacheFlags);

/*************************************************************************/ /*!
@Function       MMU_CheckFaultAddress

@Description    Check the specified MMU context to see if the provided address
                should be valid

@Input          psMMUContext            MMU context to store the data on

@Input          psDevVAddr              Address to check

@Output          psOutFaultData          To store fault details after checking

@Return         None
*/
/*****************************************************************************/
void MMU_CheckFaultAddress(MMU_CONTEXT *psMMUContext,
                           IMG_DEV_VIRTADDR *psDevVAddr,
                           MMU_FAULT_DATA *psOutFaultData);

/*************************************************************************/ /*!
@Function       MMU_IsVDevAddrValid
@Description    Checks if given address is valid.
@Input          psMMUContext MMU context to store the data on
@Input          uiLog2PageSize page size
@Input          sDevVAddr Address to check
@Return         IMG_TRUE of address is valid
*/ /**************************************************************************/
IMG_BOOL MMU_IsVDevAddrValid(MMU_CONTEXT *psMMUContext,
                             IMG_UINT32 uiLog2PageSize,
                             IMG_DEV_VIRTADDR sDevVAddr);

#if defined(PDUMP)

/*************************************************************************/ /*!
@Function       MMU_ContextDerivePCPDumpSymAddr

@Description    Derives a PDump Symbolic address for the top level MMU object

@Input          psMMUContext                    MMU context to operate on

@Input          pszPDumpSymbolicNameBuffer      Buffer to write the PDump symbolic
                                                address to

@Input          uiPDumpSymbolicNameBufferSize   Size of the buffer

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_ContextDerivePCPDumpSymAddr(MMU_CONTEXT *psMMUContext,
                                IMG_CHAR *pszPDumpSymbolicNameBuffer,
                                size_t uiPDumpSymbolicNameBufferSize);

PVRSRV_ERROR
PH1MMU_PDumpWritePageCatBase(MMU_CONTEXT *psMMUContext,
			     const IMG_CHAR *pszSpaceName,
			     IMG_DEVMEM_OFFSET_T uiOffset,
			     IMG_UINT32 ui32AlignShift,
			     IMG_UINT64 ui64PageSizeFlag);

/*************************************************************************/ /*!
@Function       MMU_PDumpWritePageCatBase

@Description    PDump write of the top level MMU object to a device register

@Input          psMMUContext        MMU context to operate on

@Input          pszSpaceName        PDump name of the mem/reg space

@Input          uiOffset            Offset to write the address to

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR MMU_PDumpWritePageCatBase(MMU_CONTEXT *psMMUContext,
                                       const IMG_CHAR *pszSpaceName,
                                       IMG_DEVMEM_OFFSET_T uiOffset,
                                       IMG_UINT32 ui32WordSize,
                                       IMG_UINT32 ui32AlignShift,
                                       IMG_UINT32 ui32Shift,
                                       PDUMP_FLAGS_T uiPdumpFlags);

/*************************************************************************/ /*!
@Function       MMU_AcquirePDumpMMUContext

@Description    Acquire a reference to the PDump MMU context for this MMU
                context

@Input          psMMUContext            MMU context to operate on

@Output         pui32PDumpMMUContextID  PDump MMU context ID

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_AcquirePDumpMMUContext(MMU_CONTEXT *psMMUContext,
                           IMG_UINT32 *pui32PDumpMMUContextID,
                           IMG_UINT32 ui32PDumpFlags);

/*************************************************************************/ /*!
@Function       MMU_ReleasePDumpMMUContext

@Description    Release a reference to the PDump MMU context for this MMU context

@Input          psMMUContext            MMU context to operate on

@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
PVRSRV_ERROR
MMU_ReleasePDumpMMUContext(MMU_CONTEXT *psMMUContext,
                           IMG_UINT32 ui32PDumpFlags);
#else /* PDUMP */

#ifdef INLINE_IS_PRAGMA
#pragma inline(MMU_PDumpWritePageCatBase)
#endif

static INLINE void
PH1MMU_PDumpWritePageCatBase(MMU_CONTEXT *psMMUContext,
			     const IMG_CHAR *pszSpaceName,
			     IMG_DEVMEM_OFFSET_T uiOffset,
			     IMG_UINT32 ui32AlignShift,
			     IMG_UINT64 ui64PageSizeFlag)
{
	PVR_UNREFERENCED_PARAMETER(psMMUContext);
	PVR_UNREFERENCED_PARAMETER(pszSpaceName);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(ui32AlignShift);
	PVR_UNREFERENCED_PARAMETER(ui64PageSizeFlag);
}
static INLINE void
MMU_PDumpWritePageCatBase(MMU_CONTEXT *psMMUContext,
                          const IMG_CHAR *pszSpaceName,
                          IMG_DEVMEM_OFFSET_T uiOffset,
                          IMG_UINT32 ui32WordSize,
                          IMG_UINT32 ui32AlignShift,
                          IMG_UINT32 ui32Shift,
                          PDUMP_FLAGS_T uiPdumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psMMUContext);
	PVR_UNREFERENCED_PARAMETER(pszSpaceName);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(ui32WordSize);
	PVR_UNREFERENCED_PARAMETER(ui32AlignShift);
	PVR_UNREFERENCED_PARAMETER(ui32Shift);
	PVR_UNREFERENCED_PARAMETER(uiPdumpFlags);
}
#endif /* PDUMP */

void RGXMapBRN71422TargetPhysicalAddress(MMU_CONTEXT *psMMUContext);

IMG_UINT32 CalcPCEIdx(IMG_DEV_VIRTADDR sDevVAddr,
		      const MMU_DEVVADDR_CONFIG *psDevVAddrConfig,
		      IMG_BOOL bRoundUp);
IMG_UINT32 CalcPDPtrEIdx(IMG_DEV_VIRTADDR sDevVAddr,
			 const MMU_DEVVADDR_CONFIG *psDevVAddrConfig,
			 IMG_BOOL bRoundUp);
IMG_UINT32 CalcPDEIdx(IMG_DEV_VIRTADDR sDevVAddr,
		      const MMU_DEVVADDR_CONFIG *psDevVAddrConfig,
		      IMG_BOOL bRoundUp);
IMG_UINT32 CalcPTEIdx(IMG_DEV_VIRTADDR sDevVAddr,
		      const MMU_DEVVADDR_CONFIG *psDevVAddrConfig,
		      IMG_BOOL bRoundUp);

#endif /* #ifdef MMU_COMMON_H */
