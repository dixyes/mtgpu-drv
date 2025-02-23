/*************************************************************************/ /*!
@File           dma_km.h
@Title          DMA transfer module header
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

#ifndef DMA_KM_H
#define DMA_KM_H

#if !defined(__linux__)
#define KERNEL_VERSION
#endif

#include "pvrsrv_error.h"
#include "img_types.h"
#include "cache_ops.h"
#include "device.h"
#include "pmr.h"
#include "pvrsrv_sync_km.h"
#include "connection_server.h"
#include "mtgpu_dma.h"

struct PageArrayPAInfo {
	IMG_UINT64 *pui64PageDevPAddr;
	IMG_BOOL *pbPagesValid;
	IMG_UINT32 ui32OffsetInPage;
	IMG_UINT32 ui32PageNum;
};

PVRSRV_ERROR DmaDeviceParams(CONNECTION_DATA *psConnection,
							 PVRSRV_DEVICE_NODE *psDevNode,
							 IMG_UINT32 *ui32DmaBuffAlign,
							 IMG_UINT32 *ui32DmaTransferMult);

PVRSRV_ERROR DmaSparseMappingTable(PMR *psPMR,
								   IMG_DEVMEM_OFFSET_T uiOffset,
								   IMG_UINT32 ui32SizeInPages,
								   IMG_BOOL *pbTable);

PVRSRV_ERROR DmaTransfer(CONNECTION_DATA *psConnection,
		    PVRSRV_DEVICE_NODE *psDevNode,
			IMG_UINT32 uiNumDMAs,
			PMR** ppsPMR,
			IMG_UINT64 *puiAddress,
			IMG_DEVMEM_OFFSET_T *puiOffset,
			IMG_DEVMEM_SIZE_T *puiSize,
			IMG_UINT32 uiFlags,
			PVRSRV_TIMELINE iUpdateTimeline);

PVRSRV_ERROR MTDmaTransfer(CONNECTION_DATA *psConnection,
			PVRSRV_DEVICE_NODE *psDevNode,
			IMG_UINT32 uiNumDMAs,
			PMR** ppsPMR,
			IMG_UINT64 *puiAddress,
			IMG_DEVMEM_OFFSET_T *puiOffset,
			IMG_DEVMEM_SIZE_T *puiSize,
			IMG_UINT32 uiFlags,
			IMG_UINT32 uiExtJobRef,
			PVRSRV_TIMELINE iUpdateTimeline);

PVRSRV_ERROR MTDmaTransferP2P(CONNECTION_DATA *psConnection,
			      PVRSRV_DEVICE_NODE *psDevNode,
			      IMG_UINT32 uiNumDMAs,
			      PMR **ppsLocalPMR,
			      IMG_DEVMEM_OFFSET_T *puiLocalOffset,
			      PMR **ppsPeerPMR,
			      IMG_DEVMEM_OFFSET_T *puiPeerOffset,
			      IMG_DEVMEM_SIZE_T *puiSize,
			      IMG_UINT32 uiFlags,
			      IMG_UINT32 uiExtJobRef,
			      PVRSRV_TIMELINE iUpdateFenceTimeline);

PVRSRV_ERROR PVRSRVInitialiseDMA(PVRSRV_DEVICE_NODE *psDeviceNode);
void PVRSRVDeInitialiseDMA(PVRSRV_DEVICE_NODE *psDeviceNode);
IMG_BOOL DmaXferBlockCanMerge(struct dma_xfer_block *psPreBlock,
			      struct dma_xfer_block *psBlock);

#endif /* DMA_KM_H */
