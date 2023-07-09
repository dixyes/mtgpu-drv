/*************************************************************************/ /*!
@File
@Title          Functions for recording ION memory stats.
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

#ifndef PVR_ION_STATS_H
#define PVR_ION_STATS_H

#include <linux/rbtree.h>
#include "pvrsrv_error.h"
#include "img_defs.h"

struct dma_buf;

#if defined(PVRSRV_ENABLE_PVR_ION_STATS)
#include "di_common.h"
#include "di_server.h"
#include "device.h"

#define MAX_NUM_HEAPS (5)

struct _pvr_ion_stats_state_;

typedef struct _pvr_ion_stats_heap_ {
	/* Name of Heap */
	char szName[32];
	/* Pointer to its related _pvr_ion_stats_state_ */
	struct _pvr_ion_stats_state_ *stats_state;
	/* An unique key which refers to a specific heap */
	IMG_UINT32 ui32HashKey;
} PVR_ION_STATS_HEAP;

typedef IMG_UINT32 (*HashKeypfn)(uintptr_t input);

typedef struct _pvr_ion_stats_state_ {
	/* Supported heaps on this platform */
	PVR_ION_STATS_HEAP sHeapData[MAX_NUM_HEAPS];
	IMG_UINT32 ui32NumHeaps;

	/* Nodes of debugfs */
	DI_GROUP *debugfs_ion;
	DI_GROUP *debugfs_heaps;
	DI_ENTRY *debugfs_heaps_entry[MAX_NUM_HEAPS];

	/* A record of buffers */
	struct rb_root buffers;

	/* Buffer lock that need to be held to edit tree */
	POS_LOCK hBuffersLock;

	/* Hash function for heap hash key */
	HashKeypfn pfnHashKey;
} PVR_ION_STATS_STATE;

PVRSRV_ERROR PVRSRVIonStatsInitialise(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);

void PVRSRVIonStatsDestroy(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);

void PVRSRVIonAddMemAllocRecord(struct dma_buf *psDmaBuf,
				struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);

void PVRSRVIonRemoveMemAllocRecord(struct dma_buf *psDmaBuf,
				   struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);
#else
static INLINE PVRSRV_ERROR PVRSRVIonStatsInitialise(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode)
{
	return PVRSRV_OK;
}

static INLINE void PVRSRVIonStatsDestroy(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode)
{
}

static INLINE void PVRSRVIonAddMemAllocRecord(struct dma_buf *psDmaBuf,
					      struct _PVRSRV_DEVICE_NODE_ *psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psDmaBuf);
}

static INLINE void PVRSRVIonRemoveMemAllocRecord(struct dma_buf *psDmaBuf,
						 struct _PVRSRV_DEVICE_NODE_ *psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psDmaBuf);
}
#endif /* defined(PVRSRV_ENABLE_PVR_ION_STATS) */

#endif /* PVR_ION_STATS_H */
