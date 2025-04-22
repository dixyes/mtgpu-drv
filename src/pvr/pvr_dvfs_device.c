/*************************************************************************/ /*!
@File
@Title          PowerVR devfreq device implementation
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Linux module setup
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

#if !defined(NO_HARDWARE)

#include <linux/devfreq.h>
#include <linux/thermal.h>
#include <linux/acpi.h>
#include <linux/version.h>
#include <linux/device.h>
#include <drm/drm.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#endif

#include "power.h"
#include "pvrsrv.h"
#include "pvrsrv_device.h"

#include "rgxdevice.h"
#include "rgxinit.h"
#include "sofunc_rgx.h"

#include "syscommon.h"

#include "pvr_dvfs_device.h"

#include "kernel_compatibility.h"

#include "mtgpu_util.h"
#include "mtgpu_module_param.h"

#define THERMAL_COOLING_STATE_MAX 4
#define THERMAL_COOLING_STATE_DEFAULT 0
#define THERMAL_COOLING_DEVICE_NAME "mtgpu"

struct _THERMAL_COOLING_DATA_
{
	struct thermal_cooling_device_ops sCoolingOps;
	PPVRSRV_DEVICE_NODE psDeviceNode;
	IMG_UINT32 ui32CurState;
	IMG_UINT32 ui32MaxState;
};

static int _device_get_devid(struct device *dev)
{
	struct drm_device *ddev = dev_get_drvdata(dev);
	int deviceId;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0))
	/*
	 * Older kernels do not have render drm_minor member in drm_device,
	 * so we fallback to primary node for device identification
	 */
	deviceId = ddev->primary->index;
#else
	if (ddev->render)
		deviceId = ddev->render->index;
	else /* when render node is NULL, fallback to primary node */
		deviceId = ddev->primary->index;
#endif

	return deviceId;
}

static IMG_INT32 devfreq_target(struct device *dev, unsigned long *requested_freq, IMG_UINT32 flags)
{
	int				deviceId = _device_get_devid(dev);
	PVRSRV_DEVICE_NODE		*psDeviceNode = PVRSRVGetDeviceInstanceByOSId(deviceId);
	RGX_DATA			*psRGXData = NULL;
	IMG_DVFS_DEVICE			*psDVFSDevice = NULL;
	IMG_DVFS_DEVICE_CFG		*psDVFSDeviceCfg = NULL;
	RGX_TIMING_INFORMATION		*psRGXTimingInfo = NULL;
	PVRSRV_DEVICE_HEALTH_STATUS	eNewStatus;
	IMG_UINT32			ui32Freq, ui32CurFreq, ui32Volt = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
	struct opp *opp;
#else
	struct dev_pm_opp *opp;
#endif

	/* Check the device is registered */
	if (!psDeviceNode)
	{
		return -ENODEV;
	}

	psRGXData = (RGX_DATA*) psDeviceNode->psDevConfig->hDevData;
	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;

	/* Check the MUSA device is initialised */
	if (!psRGXData)
	{
		return -ENODATA;
	}

	psRGXTimingInfo = psRGXData->psRGXTimingInfo;

	eNewStatus = OSAtomicRead(&psDeviceNode->eHealthStatus);

	if (!psDVFSDevice->bEnabled || eNewStatus != PVRSRV_DEVICE_HEALTH_STATUS_OK)
	{
		/* If DVFS has been suspend or the device status is not OK,
		 * the frequency will not be adjusted and the previous frequency
		 * will be returned.
		 */
		*requested_freq = psRGXTimingInfo->ui32CoreClockSpeed;
		return 0;
	}

	if (psDVFSDevice->bPerfMode)
	{
		/* perf mode, get max frequency */
#if defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
		ui32Freq = psDVFSDevice->psDevFreq->policy.user.max_freq;
#elif defined(OS_STRUCT_DEVFREQ_HAS_MIN_FREQ)
		ui32Freq = psDVFSDevice->psDevFreq->max_freq;
#else
		ui32Freq = psDVFSDevice->psDevFreq->scaling_max_freq;
#endif
		*requested_freq = ui32Freq;
	}

	/* Get recommended frequency */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	rcu_read_lock();
#endif

	opp = devfreq_recommended_opp(dev, requested_freq, flags);
	if (IS_ERR(opp)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
		rcu_read_unlock();
#endif
		PVR_DPF((PVR_DBG_ERROR, "Invalid OPP"));
		return PTR_ERR(opp);
	}

	ui32Freq = dev_pm_opp_get_freq(opp);
	ui32Volt = dev_pm_opp_get_voltage(opp);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	rcu_read_unlock();
#else
	dev_pm_opp_put(opp);
#endif

	ui32CurFreq = psRGXTimingInfo->ui32CoreClockSpeed;

	if (ui32CurFreq == ui32Freq)
	{
		return 0;
	}

	if (PVRSRV_OK != PVRSRVDevicePreClockSpeedChange(psDeviceNode,
													 psDVFSDeviceCfg->bIdleReq,
													 NULL))
	{
		dev_err(dev, "PVRSRVDevicePreClockSpeedChange failed\n");
		return -EPERM;
	}

	/* Increasing frequency, change voltage first */
	if (ui32Freq > ui32CurFreq)
	{
		if (psDVFSDeviceCfg->pfnSetVoltage)
		{
			psDVFSDeviceCfg->pfnSetVoltage(ui32Volt);
		}
	}

	psDVFSDeviceCfg->pfnSetFrequency(psDeviceNode, ui32Freq);

	/* Decreasing frequency, change frequency first */
	if (ui32Freq < ui32CurFreq)
	{
		if (psDVFSDeviceCfg->pfnSetVoltage)
		{
			psDVFSDeviceCfg->pfnSetVoltage(ui32Volt);
		}
	}

	psRGXTimingInfo->ui32CoreClockSpeed = ui32Freq;

	PVRSRVDevicePostClockSpeedChange(psDeviceNode, psDVFSDeviceCfg->bIdleReq,
									 NULL);

	return 0;
}

static int devfreq_get_dev_status(struct device *dev, struct devfreq_dev_status *stat)
{
	int                      deviceId = _device_get_devid(dev);
	PVRSRV_DEVICE_NODE      *psDeviceNode = PVRSRVGetDeviceInstanceByOSId(deviceId);
	PVRSRV_RGXDEV_INFO      *psDevInfo = NULL;
	IMG_DVFS_DEVICE         *psDVFSDevice = NULL;
	RGX_DATA                *psRGXData = NULL;
	RGX_TIMING_INFORMATION  *psRGXTimingInfo = NULL;
	RGXFWIF_GPU_UTIL_STATS   asGpuUtilStats[RGXFWIF_GPU_UTIL_ITEM_COUNT];
	PVRSRV_ERROR             eError;

	/* Check the device is registered */
	if (!psDeviceNode)
	{
		return -ENODEV;
	}

	psDevInfo = psDeviceNode->pvDevice;
	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psRGXData = (RGX_DATA*) psDeviceNode->psDevConfig->hDevData;

	/* Check the MUSA device is initialised */
	if (!psDevInfo || !psRGXData)
	{
		return -ENODATA;
	}

	psRGXTimingInfo = psRGXData->psRGXTimingInfo;
	stat->current_frequency = psRGXTimingInfo->ui32CoreClockSpeed;

	if (psDevInfo->pfnGetGpuUtilStats == NULL)
	{
		/* Not yet ready. So set times to something sensible. */
		stat->busy_time = 0;
		stat->total_time = 0;
		return 0;
	}

	eError = psDevInfo->pfnGetGpuUtilStats(psDeviceNode,
					       psDVFSDevice->hGpuUtilUserDVFS,
					       asGpuUtilStats);
	if (eError == PVRSRV_OK)
	{
		stat->busy_time = asGpuUtilStats[RGXFWIF_GPU_OVERALL_UTIL].ui64GpuStatActive;
		stat->total_time = asGpuUtilStats[RGXFWIF_GPU_OVERALL_UTIL].ui64GpuStatCumulative;
		psDVFSDevice->ui64PreBusyTime = asGpuUtilStats[RGXFWIF_GPU_OVERALL_UTIL].ui64GpuStatActive;
		psDVFSDevice->ui64PreTotalTime = asGpuUtilStats[RGXFWIF_GPU_OVERALL_UTIL].ui64GpuStatCumulative;
	}
	else if (eError == PVRSRV_ERROR_RESOURCE_UNAVAILABLE)
	{
		stat->busy_time = psDVFSDevice->ui64PreBusyTime;
		stat->total_time = psDVFSDevice->ui64PreTotalTime;
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR, "failed to Get GPU utilisation statistics(%s)", PVRSRVGETERRORSTRING(eError)));
		return -PvrErrorToLinuxErrno(eError);
	}

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
static IMG_INT32 devfreq_cur_freq(struct device *dev, unsigned long *freq)
{
	int deviceId = _device_get_devid(dev);
	PVRSRV_DEVICE_NODE *psDeviceNode = PVRSRVGetDeviceInstanceByOSId(deviceId);
	RGX_DATA *psRGXData = NULL;

	/* Check the device is registered */
	if (!psDeviceNode)
	{
		return -ENODEV;
	}

	psRGXData = (RGX_DATA*) psDeviceNode->psDevConfig->hDevData;

	/* Check the MUSA device is initialised */
	if (!psRGXData)
	{
		return -ENODATA;
	}

	*freq = psRGXData->psRGXTimingInfo->ui32CoreClockSpeed;

	return 0;
}
#endif

static struct devfreq_dev_profile img_devfreq_dev_profile =
{
	.target             = devfreq_target,
	.get_dev_status     = devfreq_get_dev_status,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
	.get_cur_freq       = devfreq_cur_freq,
#endif
};

static int FillOPPTable(struct device *dev, PVRSRV_DEVICE_NODE *psDeviceNode)
{
	const IMG_OPP *iopp;
	int i, err = 0;
	IMG_DVFS_DEVICE_CFG *psDVFSDeviceCfg = NULL;

	/* Check the device exists */
	if (!dev || !psDeviceNode)
	{
		return -ENODEV;
	}

	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;

	for (i = 0, iopp = psDVFSDeviceCfg->pasOPPTable;
	     i < psDVFSDeviceCfg->ui32OPPTableSize;
	     i++, iopp++)
	{
		err = dev_pm_opp_add(dev, iopp->ui32Freq, iopp->ui32Volt);
		if (err) {
			dev_err(dev, "Could not add OPP entry, %d\n", err);
			return err;
		}
	}

	return 0;
}

static void ClearOPPTable(struct device *dev, PVRSRV_DEVICE_NODE *psDeviceNode)
{
#if (defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
	const IMG_OPP *iopp;
	int i;
	IMG_DVFS_DEVICE_CFG *psDVFSDeviceCfg = NULL;

	/* Check the device exists */
	if (!dev || !psDeviceNode)
	{
		return;
	}

	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;

	for (i = 0, iopp = psDVFSDeviceCfg->pasOPPTable;
	     i < psDVFSDeviceCfg->ui32OPPTableSize;
	     i++, iopp++)
	{
		dev_pm_opp_remove(dev, iopp->ui32Freq);
	}
#endif
}

static int GetOPPValues(struct device *dev,
                        unsigned long *min_freq,
                        unsigned long *min_volt,
                        unsigned long *max_freq)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
	struct opp *opp;
#else
	struct dev_pm_opp *opp;
#endif
	int count, i, err = 0;
	unsigned long freq;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)) && \
	(!defined(CHROMIUMOS_KERNEL) || (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)))
	unsigned int *freq_table;
#else
	unsigned long *freq_table;
#endif

	count = dev_pm_opp_get_opp_count(dev);
	if (count < 0)
	{
		dev_err(dev, "Could not fetch OPP count, %d\n", count);
		return count;
	}

	dev_info(dev, "Found %d OPP points.\n", count);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
	freq_table = devm_kcalloc(dev, count, sizeof(*freq_table), GFP_ATOMIC);
#else
	freq_table = kcalloc(count, sizeof(*freq_table), GFP_ATOMIC);
#endif
	if (! freq_table)
	{
		return -ENOMEM;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	/* Start RCU read-side critical section to map frequency to OPP */
	rcu_read_lock();
#endif

	/* Iterate over OPP table; Iteration 0 finds "opp w/ freq >= 0 Hz".	 */
	freq = 0;
	opp = dev_pm_opp_find_freq_ceil(dev, &freq);
	if (IS_ERR(opp))
	{
		err = PTR_ERR(opp);
		dev_err(dev, "Couldn't find lowest frequency, %d\n", err);
		goto exit;
	}

	*min_volt = dev_pm_opp_get_voltage(opp);
	*max_freq = *min_freq = freq_table[0] = freq;
	dev_dbg(dev, "opp[%d/%d]: (%lu Hz, %lu uV)\n", 1, count, freq, *min_volt);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
	dev_pm_opp_put(opp);
#endif

	/* Iteration i > 0 finds "opp w/ freq >= (opp[i-1].freq + 1)". */
	for (i = 1; i < count; i++)
	{
		freq++;
		opp = dev_pm_opp_find_freq_ceil(dev, &freq);
		if (IS_ERR(opp))
		{
			err = PTR_ERR(opp);
			dev_err(dev, "Couldn't find %dth frequency, %d\n", i, err);
			goto exit;
		}

		freq_table[i] = freq;
		*max_freq = freq;
		dev_dbg(dev,
			"opp[%d/%d]: (%lu Hz, %lu uV)\n",
			i + 1,
			count,
			freq,
			dev_pm_opp_get_voltage(opp));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
		dev_pm_opp_put(opp);
#endif
	}

exit:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	rcu_read_unlock();
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
	if (!err)
	{
		img_devfreq_dev_profile.freq_table = freq_table;
		img_devfreq_dev_profile.max_state = count;
	}
	else
#endif
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
		devm_kfree(dev, freq_table);
#else
		kfree(freq_table);
#endif
	}

	return err;
}

static int cooling_ops_get_max_state(struct thermal_cooling_device *cdev, unsigned long *max_state)
{
	struct _THERMAL_COOLING_DATA_ *psCoolingData = (struct _THERMAL_COOLING_DATA_ *)cdev->ops;

	if (!psCoolingData)
		return -EINVAL;

	*max_state = psCoolingData->ui32MaxState - 1;

	return 0;
}

static int cooling_ops_get_cur_state(struct thermal_cooling_device *cdev, unsigned long *cur_state)
{
	struct _THERMAL_COOLING_DATA_ *psCoolingData = (struct _THERMAL_COOLING_DATA_ *)cdev->ops;

	if (!psCoolingData)
		return -EINVAL;

	*cur_state = psCoolingData->ui32CurState;

	return 0;
}

static int cooling_ops_set_cur_state(struct thermal_cooling_device *cdev, unsigned long state)
{
	IMG_UINT64 ui64CappingMaxFreq;
	PVRSRV_DEVICE_NODE *psDeviceNode;
	IMG_DVFS_DEVICE *psDVFSDevice;
	IMG_DVFS_DEVICE_CFG *psDVFSDeviceCfg;
	struct _THERMAL_COOLING_DATA_ *psCoolingData = (struct _THERMAL_COOLING_DATA_ *)cdev->ops;

	if (!psCoolingData || !psCoolingData->psDeviceNode)
		return -EINVAL;

	if (state >= psCoolingData->ui32MaxState)
		return -ERANGE;

	if (state  == psCoolingData->ui32CurState)
		return 0;

	psDeviceNode = psCoolingData->psDeviceNode;
	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;

	ui64CappingMaxFreq = psDVFSDeviceCfg->ui64MaxFreq -
			     (psDVFSDeviceCfg->ui64MaxFreq - psDVFSDeviceCfg->ui64MinFreq) /
			     psCoolingData->ui32MaxState * state;
	ui64CappingMaxFreq = ui64CappingMaxFreq / 1000 * 1000;

	/* Modify the max frequency of devfreq */
#if defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	psDVFSDevice->psDevFreq->policy.user.max_freq = ui64CappingMaxFreq;
#elif defined(OS_STRUCT_DEVFREQ_HAS_MIN_FREQ)
	psDVFSDevice->psDevFreq->max_freq = ui64CappingMaxFreq;
#else
	psDVFSDevice->psDevFreq->scaling_max_freq = ui64CappingMaxFreq;
#endif

	psCoolingData->ui32CurState = state;

	return 0;
}

static int RegisterCoolingDevice(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	int err = 0;
	struct device *psOSDevice;
	struct device *psPcieDevice;
	struct acpi_device *psAcpiDevice;
	IMG_DVFS_DEVICE *psDVFSDevice;
	IMG_DVFS_DEVICE_CFG *psDVFSDeviceCfg;
	struct _THERMAL_COOLING_DATA_ *psCoolingData;

	PVRSRV_VZ_RET_IF_MODE(GUEST, err);

	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;

	psOSDevice = OSGetOSDeviceFromDeviceNode(psDeviceNode);
	psPcieDevice = OSGetPcieDeviceFromOSDevice(psOSDevice);
	psAcpiDevice = ACPI_COMPANION(psPcieDevice);

	psCoolingData = kzalloc(sizeof(*psDVFSDeviceCfg->psCoolingData), GFP_KERNEL);
	if (!psCoolingData)
	{
		return -ENOMEM;
	}

	psCoolingData->ui32CurState = THERMAL_COOLING_STATE_DEFAULT;
	psCoolingData->ui32MaxState = THERMAL_COOLING_STATE_MAX;
	psCoolingData->psDeviceNode = psDeviceNode;
	psCoolingData->sCoolingOps.get_max_state = &cooling_ops_get_max_state;
	psCoolingData->sCoolingOps.get_cur_state = &cooling_ops_get_cur_state;
	psCoolingData->sCoolingOps.set_cur_state = &cooling_ops_set_cur_state;

	psDVFSDevice->psCoolingDevice = thermal_cooling_device_register(
			THERMAL_COOLING_DEVICE_NAME, psAcpiDevice,
			(struct thermal_cooling_device_ops *)psCoolingData);
	if (IS_ERR(psDVFSDevice->psCoolingDevice))
	{
		err = PTR_ERR(psDVFSDevice->psCoolingDevice);
		dev_err(psOSDevice, "Failed to register cooling device %d", err);
		kfree(psCoolingData);
		return err;
	}

	psDVFSDeviceCfg->psCoolingData = psCoolingData;

	return 0;
}

#define TO_IMG_ERR(err) ((err == -EPROBE_DEFER) ? PVRSRV_ERROR_PROBE_DEFER : PVRSRV_ERROR_INIT_FAILURE)

PVRSRV_ERROR InitDVFS(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	IMG_DVFS_DEVICE        *psDVFSDevice = NULL;
	IMG_DVFS_DEVICE_CFG    *psDVFSDeviceCfg = NULL;
	struct device          *psDev;
	PVRSRV_ERROR            eError;
	int                     err;

	PVRSRV_VZ_RET_IF_MODE(GUEST, PVRSRV_OK);

	if (!psDeviceNode)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	if (!psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg.bSupportDVFS)
	{
		return PVRSRV_OK;
	}

#if !defined(CONFIG_PM_OPP)
	return PVRSRV_ERROR_NOT_SUPPORTED;
#endif

	if (psDeviceNode->psDevConfig->sDVFS.sDVFSDevice.bInitPending)
	{
		PVR_DPF((PVR_DBG_ERROR,
				 "DVFS initialise pending for device node %p",
				 psDeviceNode));
		return PVRSRV_ERROR_INIT_FAILURE;
	}

	psDev = psDeviceNode->psDevConfig->pvOSDevice;
	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;
	psDeviceNode->psDevConfig->sDVFS.sDVFSDevice.bInitPending = IMG_TRUE;

	if (mtgpu_drm_major == 1)
	{
		eError = SORgxGpuUtilStatsRegister(&psDVFSDevice->hGpuUtilUserDVFS);
	}
	else
	{
		err = mtgpu_util_stats_register(&psDVFSDevice->hGpuUtilUserDVFS);
		eError = LinuxErrnoToPvrError(err);
	}
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to register to the GPU utilisation stats, %d", eError));
		return eError;
	}

#if defined(CONFIG_OF)
	err = dev_pm_opp_of_add_table(psDev);
	if (err)
	{
		/*
		 * If there are no device tree or system layer provided operating points
		 * then return an error
		 */
		if (err != -ENODEV || !psDVFSDeviceCfg->pasOPPTable)
		{
			PVR_DPF((PVR_DBG_ERROR, "Failed to init opp table from devicetree, %d", err));
			eError = TO_IMG_ERR(err);
			goto err_exit;
		}
	}
#endif

	if (psDVFSDeviceCfg->pasOPPTable)
	{
		err = FillOPPTable(psDev, psDeviceNode);
		if (err)
		{
			PVR_DPF((PVR_DBG_ERROR, "Failed to fill OPP table with data, %d", err));
			eError = TO_IMG_ERR(err);
			goto err_exit;
		}
	}

	PVR_TRACE(("MTGPU DVFS init pending: dev = %p, MTGPU device = %p",
			   psDev, psDeviceNode));

	return PVRSRV_OK;

err_exit:
	DeinitDVFS(psDeviceNode);
	return eError;
}

PVRSRV_ERROR RegisterDVFSDevice(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	IMG_DVFS_DEVICE        *psDVFSDevice = NULL;
	IMG_DVFS_DEVICE_CFG    *psDVFSDeviceCfg = NULL;
	IMG_DVFS_GOVERNOR_CFG  *psDVFSGovernorCfg = NULL;
	RGX_TIMING_INFORMATION *psRGXTimingInfo = NULL;
	struct device          *psDev;
	unsigned long           min_freq = 0, max_freq = 0, min_volt = 0;
	PVRSRV_ERROR            eError;
	int                     err;

	if (!psDeviceNode)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	if (!psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg.bSupportDVFS)
	{
		return PVRSRV_OK;
	}

	if (!psDeviceNode->psDevConfig->sDVFS.sDVFSDevice.bInitPending)
	{
		PVR_DPF((PVR_DBG_ERROR,
				 "DVFS initialise not yet pending for device node %p",
				 psDeviceNode));
		return PVRSRV_ERROR_INIT_FAILURE;
	}

	psDev = psDeviceNode->psDevConfig->pvOSDevice;
	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;
	psDVFSGovernorCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSGovernorCfg;
	psRGXTimingInfo = ((RGX_DATA *)psDeviceNode->psDevConfig->hDevData)->psRGXTimingInfo;
	psDeviceNode->psDevConfig->sDVFS.sDVFSDevice.bInitPending = IMG_FALSE;
	psDeviceNode->psDevConfig->sDVFS.sDVFSDevice.bReady = IMG_TRUE;

	err = GetOPPValues(psDev, &min_freq, &min_volt, &max_freq);
	if (err)
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to read OPP points, %d", err));
		eError = TO_IMG_ERR(err);
		goto err_exit;
	}

	psDVFSDeviceCfg->ui64MaxFreq = max_freq;
	psDVFSDeviceCfg->ui64MinFreq = min_freq;

	img_devfreq_dev_profile.initial_freq = min_freq;
	img_devfreq_dev_profile.polling_ms = psDVFSDeviceCfg->ui32PollMs;

	psRGXTimingInfo->ui32CoreClockSpeed = min_freq;

	if (psDVFSDeviceCfg->pfnSetFrequency)
	{
		psDVFSDeviceCfg->pfnSetFrequency(psDeviceNode, min_freq);
	}

	if (psDVFSDeviceCfg->pfnSetVoltage)
	{
		psDVFSDeviceCfg->pfnSetVoltage(min_volt);
	}

#if defined(CONFIG_DEVFREQ_GOV_SIMPLE_ONDEMAND)
	psDVFSDevice->psOndemandData = kzalloc(sizeof(*psDVFSDevice->psOndemandData), GFP_KERNEL);
	PVR_GOTO_IF_NOMEM(psDVFSDevice->psOndemandData, eError, err_exit);

	psDVFSDevice->psOndemandData->upthreshold = psDVFSGovernorCfg->ui32UpThreshold;
	psDVFSDevice->psOndemandData->downdifferential = psDVFSGovernorCfg->ui32DownDifferential;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	psDVFSDevice->psDevFreq = devm_devfreq_add_device(psDev,
													  &img_devfreq_dev_profile,
													  "simple_ondemand",
													  psDVFSDevice->psOndemandData);
#else
	psDVFSDevice->psDevFreq = devfreq_add_device(psDev,
												 &img_devfreq_dev_profile,
												 "simple_ondemand",
												 psDVFSDevice->psOndemandData);
#endif

	if (IS_ERR(psDVFSDevice->psDevFreq))
	{
		PVR_DPF((PVR_DBG_ERROR,
				 "Failed to add as devfreq device %p, %ld",
				 psDVFSDevice->psDevFreq,
				 PTR_ERR(psDVFSDevice->psDevFreq)));
		eError = TO_IMG_ERR(PTR_ERR(psDVFSDevice->psDevFreq));
		goto err_exit;
	}

	eError = SuspendDVFS(psDeviceNode);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "PVRSRVInit: Failed to suspend DVFS"));
		goto err_exit;
	}

#if defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	psDVFSDevice->psDevFreq->policy.user.min_freq = min_freq;
	psDVFSDevice->psDevFreq->policy.user.max_freq = max_freq;
#elif defined(OS_STRUCT_DEVFREQ_HAS_MIN_FREQ)
	psDVFSDevice->psDevFreq->min_freq = min_freq;
	psDVFSDevice->psDevFreq->max_freq = max_freq;
#else
	psDVFSDevice->psDevFreq->scaling_min_freq = min_freq;
	psDVFSDevice->psDevFreq->scaling_max_freq = max_freq;
#endif

	err = devfreq_register_opp_notifier(psDev, psDVFSDevice->psDevFreq);
	if (err)
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to register opp notifier, %d", err));
		eError = TO_IMG_ERR(err);
		goto err_exit;
	}

	if (!acpi_disabled)
	{
		err = RegisterCoolingDevice(psDeviceNode);
		if (err)
		{
			eError = TO_IMG_ERR(err);
			goto err_exit;
		}
	}

	PVR_TRACE(("MTGPU DVFS activated: %lu-%lu Hz, Period: %ums",
			   min_freq,
			   max_freq,
			   psDVFSDeviceCfg->ui32PollMs));

	return PVRSRV_OK;

err_exit:
	UnregisterDVFSDevice(psDeviceNode);
	return eError;
}

void UnregisterDVFSDevice(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	IMG_DVFS_DEVICE_CFG *psDVFSDeviceCfg = NULL;
	IMG_DVFS_DEVICE *psDVFSDevice = NULL;
	struct device *psDev = NULL;
	IMG_INT32 i32Error;

	/* Check the device exists */
	if (!psDeviceNode || !psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg.bSupportDVFS)
	{
		return;
	}

	PVRSRV_VZ_RETN_IF_MODE(GUEST);

	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;
	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDev = psDeviceNode->psDevConfig->pvOSDevice;

	if (! psDVFSDevice)
	{
		return;
	}

	if (!IS_ERR_OR_NULL(psDVFSDevice->psCoolingDevice))
	{
		thermal_cooling_device_unregister(psDVFSDevice->psCoolingDevice);
		psDVFSDevice->psCoolingDevice = NULL;

		kfree(psDVFSDeviceCfg->psCoolingData);
		psDVFSDeviceCfg->psCoolingData = NULL;
	}

	if (psDVFSDevice->psDevFreq)
	{
		i32Error = devfreq_unregister_opp_notifier(psDev, psDVFSDevice->psDevFreq);
		if (i32Error < 0)
		{
			PVR_DPF((PVR_DBG_ERROR, "Failed to unregister OPP notifier"));
		}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
		devfreq_remove_device(psDVFSDevice->psDevFreq);
#else
		devm_devfreq_remove_device(psDev, psDVFSDevice->psDevFreq);
#endif

		if (psDVFSDevice->psOndemandData)
		{
			kfree(psDVFSDevice->psOndemandData);
		}

		psDVFSDevice->psDevFreq = NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0) && \
     LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
	kfree(img_devfreq_dev_profile.freq_table);
#endif

	psDVFSDevice->bInitPending = IMG_FALSE;
	psDVFSDevice->bReady = IMG_FALSE;
}

void DeinitDVFS(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	IMG_DVFS_DEVICE *psDVFSDevice = NULL;
	struct device *psDev = NULL;

	/* Check the device exists */
	if (!psDeviceNode || !psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg.bSupportDVFS)
	{
		return;
	}

	PVRSRV_VZ_RETN_IF_MODE(GUEST);

	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDev = psDeviceNode->psDevConfig->pvOSDevice;

	/* Remove OPP entries for this device */
	ClearOPPTable(psDev, psDeviceNode);

#if defined(CONFIG_OF)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
	dev_pm_opp_of_remove_table(psDev);
#endif
#endif

	if (mtgpu_drm_major == 1)
	{
		SORgxGpuUtilStatsUnregister(psDVFSDevice->hGpuUtilUserDVFS);
	}
	else
	{
		mtgpu_util_stats_unregister(psDVFSDevice->hGpuUtilUserDVFS);
	}
	psDVFSDevice->hGpuUtilUserDVFS = NULL;
	psDVFSDevice->bInitPending = IMG_FALSE;
	psDVFSDevice->bReady = IMG_FALSE;
}

PVRSRV_ERROR SuspendDVFS(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	IMG_DVFS_DEVICE	*psDVFSDevice = NULL;

	/* Check the device is registered */
	if (!psDeviceNode)
	{
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	if (!psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg.bSupportDVFS)
	{
		return PVRSRV_OK;
	}

	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDevice->bEnabled = IMG_FALSE;

	return PVRSRV_OK;
}

PVRSRV_ERROR ResumeDVFS(PPVRSRV_DEVICE_NODE psDeviceNode)
{
	IMG_DVFS_DEVICE	*psDVFSDevice = NULL;

	/* Check the device is registered */
	if (!psDeviceNode)
	{
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	if (!psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg.bSupportDVFS)
	{
		return PVRSRV_OK;
	}

	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;

	/* Not supported in GuestOS drivers */
	psDVFSDevice->bEnabled = !PVRSRV_VZ_MODE_IS(GUEST);

	return PVRSRV_OK;
}

PVRSRV_ERROR PVRDVFSPerfModeSet(PPVRSRV_DEVICE_NODE psDeviceNode, IMG_BOOL bEnable)
{
	IMG_DVFS_DEVICE	*psDVFSDevice = NULL;
	IMG_DVFS_DEVICE_CFG *psDVFSDeviceCfg = NULL;

	/* Check the device is registered */
	if (!psDeviceNode)
	{
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	psDVFSDevice = &psDeviceNode->psDevConfig->sDVFS.sDVFSDevice;
	psDVFSDeviceCfg = &psDeviceNode->psDevConfig->sDVFS.sDVFSDeviceCfg;

	if (!psDVFSDeviceCfg->bSupportDVFS || psDVFSDevice->bPerfMode == bEnable)
	{
		return PVRSRV_OK;
	}

	/* Not supported in GuestOS drivers */
	psDVFSDevice->bPerfMode = PVRSRV_VZ_MODE_IS(GUEST) ? IMG_FALSE : bEnable;

	return PVRSRV_OK;
}
#endif /* !NO_HARDWARE */
