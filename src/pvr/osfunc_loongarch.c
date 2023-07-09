/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/version.h>
#include <linux/cpumask.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <linux/uaccess.h>

#include "pvrsrv_error.h"
#include "img_types.h"
#include "img_defs.h"
#include "osfunc.h"
#include "pvr_debug.h"

void OSCPUCacheFlushRangeKM(PVRSRV_DEVICE_NODE *psDevNode,
			      void *pvVirtStart,
			      void *pvVirtEnd,
			      IMG_CPU_PHYADDR sCPUPhysStart,
			      IMG_CPU_PHYADDR sCPUPhysEnd)
{
	struct device *dev;

	if (pvVirtStart)
	{
		return;
	}
	/* dev must be pcie device */
	dev = psDevNode->psDevConfig->pvOSDevice;
	dev = dev->parent->parent;
	if (dev)
	{
		dma_sync_single_for_device(dev, sCPUPhysStart.uiAddr,
					   sCPUPhysEnd.uiAddr - sCPUPhysStart.uiAddr,
					   DMA_TO_DEVICE);
		dma_sync_single_for_cpu(dev, sCPUPhysStart.uiAddr,
					sCPUPhysEnd.uiAddr - sCPUPhysStart.uiAddr,
					DMA_FROM_DEVICE);
	}
}

void OSCPUCacheCleanRangeKM(PVRSRV_DEVICE_NODE *psDevNode,
			      void *pvVirtStart,
			      void *pvVirtEnd,
			      IMG_CPU_PHYADDR sCPUPhysStart,
			      IMG_CPU_PHYADDR sCPUPhysEnd)
{
	struct device *dev;

	if (pvVirtStart)
	{
		return;
	}
	/* dev must be pcie device */
	dev = psDevNode->psDevConfig->pvOSDevice;
	dev = dev->parent->parent;
	if (dev)
	{
		dma_sync_single_for_device(dev, sCPUPhysStart.uiAddr,
					   sCPUPhysEnd.uiAddr - sCPUPhysStart.uiAddr,
					   DMA_TO_DEVICE);
	}
}

void OSCPUCacheInvalidateRangeKM(PVRSRV_DEVICE_NODE *psDevNode,
				   void *pvVirtStart,
				   void *pvVirtEnd,
				   IMG_CPU_PHYADDR sCPUPhysStart,
				   IMG_CPU_PHYADDR sCPUPhysEnd)
{
	struct device *dev;

	if (pvVirtStart)
	{
		return;
	}
	/* dev must be pcie device */
	dev = psDevNode->psDevConfig->pvOSDevice;
	dev = dev->parent->parent;
	if (dev)
	{
		dma_sync_single_for_cpu(dev, sCPUPhysStart.uiAddr,
					sCPUPhysEnd.uiAddr - sCPUPhysStart.uiAddr,
					DMA_FROM_DEVICE);
	}
}

OS_CACHE_OP_ADDR_TYPE OSCPUCacheOpAddressType(void)
{
	return OS_CACHE_OP_ADDR_TYPE_PHYSICAL;
}

void OSUserModeAccessToPerfCountersEn(void)
{
}

IMG_BOOL OSIsWriteCombineUnalignedSafe(void)
{
	return IMG_TRUE;
}
