/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/errno.h>

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"

typedef struct _PVRSRV_ERR_MAPPING_ {
	IMG_INT32 i32LinuxErrno;
	PVRSRV_ERROR eError;
} PVRSRV_ERR_MAPPING;

static PVRSRV_ERR_MAPPING asErrMappingTable[] = {
	{ 0, PVRSRV_OK },
	{ -EAGAIN, PVRSRV_ERROR_RETRY },
	{ -EINVAL, PVRSRV_ERROR_INVALID_PARAMS },
	{ -ENOMEM, PVRSRV_ERROR_OUT_OF_MEMORY },
	{ -ETIME, PVRSRV_ERROR_TIMEOUT },
	{ -EINTR, PVRSRV_ERROR_INTERRUPTED},
};

PVRSRV_ERROR LinuxErrnoToPvrError(int errno)
{
	IMG_UINT32 i;
	PVRSRV_ERROR eError = PVRSRV_ERROR_TASK_FAILED;

	for (i = 0; i < ARRAY_SIZE(asErrMappingTable); i++) {
		if (errno == asErrMappingTable[i].i32LinuxErrno) {
			eError = asErrMappingTable[i].eError;
			break;
		}
	}

	return eError;
}
