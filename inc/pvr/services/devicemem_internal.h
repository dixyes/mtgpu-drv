/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __DEVICEMEM_INTERNAL_H__
#define __DEVICEMEM_INTERNAL_H__

#include "devicemem_server.h"
#include "device.h"
#include "img_types.h"
#include "mmu_common.h"
#include "pmr.h"
#include "osfunc.h"
#include "lock.h"
#include "rb_tree.h"

typedef enum {
	MEM_CTX_STATE_OK = 0,
	MEM_CTX_STATE_TO_RELEASE,
	MEM_CTX_STATE_FORCE_CLEANUP,
} MEM_CTX_STATE;

struct _DEVMEMINT_CTX_
{
	PVRSRV_DEVICE_NODE *psDevNode;

	/* MMU common code needs to have a context. There's a one-to-one
	 * correspondence between device memory context and MMU context,
	 * but we have the abstraction here so that we don't need to care
	 * what the MMU does with its context, and the MMU code need not
	 * know about us at all.
	 */
	MMU_CONTEXT *psMMUContext;

	ATOMIC_T hRefCount;

	/* This handle is for devices that require notification when a new
	 * memory context is created and they need to store private data that
	 * is associated with the context.
	 */
	IMG_HANDLE hPrivData;

	/* Protects access to sProcessNotifyListHead */
	POSWR_LOCK hListLock;

	/* The following tracks UM applications that need to be notified of a
	 * page fault
	 */
	DLLIST_NODE sProcessNotifyListHead;
	/* The following is a node for the list of registered devmem contexts */
	DLLIST_NODE sPageFaultNotifyListElem;

	/* Device virtual address of a page fault on this context */
	IMG_DEV_VIRTADDR sFaultAddress;

	/* General purpose flags */
	IMG_UINT32 ui32Flags;

	DEVMEMINT_MAPPING *psPbMapping;
	DEVMEMINT_MAPPING *psPH1CdmPatchMapping;
	DEVMEMINT_MAPPING2 *psCscTabMapping;
};

struct _MEM_CTX_
{
	DEVMEMINT_CTX sDevmemCtx;

	DLLIST_NODE sNode;
	struct mtgpu_job_list *psJobList;
	IMG_UINT64 ui64PageTableUid;
	IMG_BOOL bLegacy;

	IMG_UINT32 ui32MemCtxState;
	POS_LOCK hMemCtxLock;

	/* Record the handle and used to release. */
	IMG_HANDLE hJobList;
	DLLIST_NODE sFreeNode;
};

struct _DEVMEMINT_CTX_EXPORT_
{
	DEVMEMINT_CTX *psDevmemCtx;
	PMR *psPMR;
	ATOMIC_T hRefCount;
	DLLIST_NODE sNode;
};

struct _DEVMEMINT_HEAP2_
{
	struct _DEVMEMINT_CTX_ *psDevmemCtx;
	IMG_UINT32 ui32PageSizeBitMask;
	ATOMIC_T uiRefCount;
};

struct _DEVMEMINT_HEAP_
{
	struct _DEVMEMINT_CTX_ *psDevmemCtx;
	IMG_UINT32 uiLog2PageSize;
	ATOMIC_T uiRefCount;
};

struct _DEVMEMINT_RESERVATION_
{
	struct _DEVMEMINT_HEAP_ *psDevmemHeap;
	IMG_DEV_VIRTADDR sBase;
	IMG_DEVMEM_SIZE_T uiLength;
	uint64_t ui64RmGpuVirtMem;
};

struct _DEVMEMINT_RESERVATION2_
{
	struct _DEVMEMINT_HEAP2_ *psDevmemHeap;
	IMG_DEV_VIRTADDR sBase;
	IMG_DEVMEM_SIZE_T uiLength;
	IMG_UINT32 ui32log2PageSize;
	uint64_t ui64RmGpuVirtMem;
};

struct _DEVMEMINT_MAPPING2_
{
	struct _DEVMEMINT_RESERVATION2_ *psReservation;
	PMR *psPMR;
	IMG_UINT32 uiNumPages;
	IMG_PID ui32PID;
};

struct _DEVMEMINT_MAPPING_
{
	struct _DEVMEMINT_RESERVATION_ *psReservation;
	PMR *psPMR;
	IMG_UINT32 uiNumPages;
	IMG_PID ui32PID;
};

struct _DEVMEMINT_PF_NOTIFY_
{
	IMG_UINT32  ui32PID;
	DLLIST_NODE sProcessNotifyListElem;
};

struct _DEVMEMINT_PROCESS_DATA_
{
	IMG_PID ui32PID;
	/* Protects access to psMappingTree */
	POS_LOCK hTreeLock;
	RB_TREE *psMappingTree;
};

#endif

