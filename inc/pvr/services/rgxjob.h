/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _RGXJOB_H_
#define _RGXJOB_H_

#include "sync_server.h"
#include "mtgpu_job.h"

void RGXJobFenceSignal(struct mtgpu_job_item *psJobItem);

PVRSRV_ERROR RGXDrmSchedulerAddJob(PVRSRV_DEVICE_NODE         * psDeviceNode,
				   IMG_DEV_PHYADDR 	        sPCBaseAddr,
				   IMG_UINT32                   ui32Priority,
				   POS_LOCK                     hLock,
				   struct mtgpu_sched_entity  * psEntity,
				   struct SERVER_MMU_CONTEXT_TAG * psMMUContext,
				   IMG_UINT32			ui32SubmissionQueueId,
				   IMG_UINT32     		eSubmissionCmd,
				   IMG_UINT32	     	        eNodeType,
				   IMG_UINT32	     	        eCcbType,
				   /*update sync_prim*/
				   SYNC_PRIMITIVE_BLOCK      ** pauiUpdateSyncPrimBlock,
				   IMG_UINT32                 * paui32UpdateSyncOffset,
				   IMG_UINT64                 * paui64UpdateValue,
				   IMG_UINT32                   ui32ClientUpdateCount,
				   /*check sync_prim*/
				   SYNC_PRIMITIVE_BLOCK      ** pauiCheckSyncPrimBlock,
				   IMG_UINT32                 * paui32CheckSyncOffset,
				   IMG_UINT64                 * paui64CheckValue,
				   IMG_UINT32                   ui32ClientCheckCount,
				   struct mtgpu_syncobj      ** psCheckSyncObj,
				   IMG_UINT32                   psCheckSyncObjCount,
				   struct mtgpu_syncobj       * psUpdateSyncObj,
				   IMG_UINT64 		        ui64SubmissionVa,
				   IMG_UINT32 		        ui32SubmissionSize,
				   IMG_UINT64 		        ui64SubmissionId,
				   IMG_UINT32			ui32SubmissionType);
#endif /* _RGXJOB_H_ */
