/*************************************************************************/ /*!
@File           module_common.h
@Title          Common linux module setup header
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

#ifndef MODULE_COMMON_H
#define MODULE_COMMON_H

#include "pvr_drm.h"
#include "img_types.h"
#include "pvrsrv_error.h"
#include "pvrsrv.h"

/* DRVNAME is the name we use to register our driver. */
#define DRVNAME PVR_LDM_DRIVER_REGISTRATION_NAME

/* Get the specific firmware path */
#define FIRMWARE_LOAD_PATH(binary_name) "mthreads/" binary_name

#define ENABLE_ASYNC_JOB_SUBMIT_TQ       (enable_async_job_submit & 0x1)
#define ENABLE_ASYNC_JOB_SUBMIT_CE       (enable_async_job_submit & (0x1 << 1))
#define ENABLE_ASYNC_JOB_SUBMIT_COMPUTE  (enable_async_job_submit & (0x1 << 2))
#define ENABLE_ASYNC_JOB_SUBMIT_RENDER   (enable_async_job_submit & (0x1 << 3))

#define CACHE_SNOOPING_BIT_MASK			(0x3)
#define ENABLE_CACHE_SNOOPING_REQUEST_MMU	(enable_cache_snooping & 0x1)
#define ENABLE_CACHE_SNOOPING_REQUEST_NON_MMU	(enable_cache_snooping & (0x1 << 1))
#define ENABLE_CACHE_SNOOPING_REQUEST		(enable_cache_snooping & CACHE_SNOOPING_BIT_MASK)

struct _PVRSRV_DEVICE_NODE_;
struct drm_file;
struct drm_device;

/* psDRMFile->driver_priv will point to a PVRSV_CONNECTION_PRIV
 * struct, which will contain a ptr to the CONNECTION_DATA and
 * a pfn to the release function (which will differ depending
 * on whether the connection is to Sync or Services).
 */
typedef void (*PFN_PVRSRV_DEV_RELEASE)(void *pvData);
typedef struct
{
	/* pvConnectionData is used to hold Services connection data
	 * for all PVRSRV_DEVICE_INIT_MODE options.
	 */
	void *pvConnectionData;

	/* pfDeviceRelease is used to indicate the release function
	 * to be called when PVRSRV_DEVICE_INIT_MODE is PVRSRV_LINUX_DEV_INIT_ON_CONNECT,
	 * as we can then have one connections made (either for Services or Sync) per
	 * psDRMFile, and need to know which type of connection is being released
	 * (as the ioctl release call is common for both).
	 */
	PFN_PVRSRV_DEV_RELEASE pfDeviceRelease;

	/* pvSyncConnectionData is used to hold Sync connection data
	 * when PVRSRV_DEVICE_INIT_MODE is not PVRSRV_LINUX_DEV_INIT_ON_CONNECT,
	 * as we can then have two connections made (for Services and Sync) to
	 * the same psDRMFile.
	 */
	void *pvSyncConnectionData;
} PVRSRV_CONNECTION_PRIV;

extern long vram_lower_limit;
extern bool disable_gpu;
extern bool disable_hwr;
extern bool enable_rdma;
extern int enable_large_mem_mode;
extern bool enable_mmu_persistence;
extern unsigned int enable_async_job_submit;
extern bool mss_mmu_bypass;
extern bool enable_pfm_mss;
extern unsigned int enable_cache_snooping;
#if defined(DEBUG)
extern IMG_UINT32 gPMRAllocFail;
#endif /* DEBUG */

#if defined(NO_HARDWARE)
#define gpu_page_size OSGetPageSize()
#define gpu_page_shift OSGetPageShift()
#endif

bool mtgpu_queue_work(struct work_struct *work);

int PVRSRVDriverInit(void);
void PVRSRVDriverDeinit(void);

int PVRSRVDeviceInit(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);
void PVRSRVDeviceDeinit(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);

void PVRSRVDeviceShutdown(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);
int PVRSRVDeviceSuspend(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);
int PVRSRVDeviceResume(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode);

int PVRSRVDeviceServicesOpen(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode,
                             struct drm_file *psDRMFile);
void PVRSRVDeviceRelease(struct _PVRSRV_DEVICE_NODE_ *psDeviceNode,
                         struct drm_file *psDRMFile);
int drm_pvr_srvkm_init(struct drm_device *dev,
                       void *arg, struct drm_file *psDRMFile);
PVRSRV_ERROR PVRSRVAcquireBackupAreasFromDevice(struct drm_device *ddev,
						MEM_REGION_INFO **psAreas,
						IMG_INT32 *psAreasCnt);
void PVRSRVReleaseBackupAreasFromDevice(struct drm_device *ddev,
					MEM_REGION_INFO *psAreas);
#endif /* MODULE_COMMON_H */
