/*
 * @File
 * @Title       PowerVR DRM driver
 * @Codingstyle LinuxKernel
 * @Copyright   Copyright (c) Imagination Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 *
 * The contents of this file are subject to the MIT license as set out below.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License Version 2 ("GPL") in which case the provisions
 * of GPL are applicable instead of those above.
 *
 * If you wish to allow use of your version of this file only under the terms of
 * GPL, and not to allow others to use your version of this file under the terms
 * of the MIT license, indicate your decision by deleting the provisions above
 * and replace them with the notice and other provisions required by GPL as set
 * out in the file called "GPL-COPYING" included in this distribution. If you do
 * not delete the provisions above, a recipient may use your version of this file
 * under the terms of either the MIT license or GPL.
 *
 * This License is also included in this distribution in the file called
 * "MIT-COPYING".
 *
 * EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <linux/version.h>

#include <drm/drm.h>

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_drv.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_print.h>
#include <linux/dma-mapping.h>
#endif

#include <drm/drm_crtc.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/mutex.h>

#include "module_common.h"
#include "pvr_drm.h"
#include "pvr_drv.h"
#include "pvr_bridge_k.h"
#include "pvrversion.h"
#include "services_kernel_client.h"
#include "pvr_sync_ioctl_drm.h"

#include "mtgpu_drm.h"
#include "mtgpu_vm.h"
#include "mtgpu_bo.h"
#include "mtgpu_dma_next.h"
#include "mtgpu_drv_next.h"
#include "mtgpu_info.h"
#include "mtgpu_semaphore.h"
#include "mtgpu_fence.h"
#include "mtgpu_job.h"
#include "mtgpu_context.h"
#include "mtgpu_cache.h"
#include "mtgpu_object.h"
#include "mtgpu_transport_layer.h"
#include "mtgpu_hwperf.h"
#include "mtgpu_codec.h"

#include "kernel_compatibility.h"

#define PVR_DRM_DRIVER_NAME PVR_DRM_NAME
#define PVR_DRM_DRIVER_DESC "Imagination Technologies DRM Driver"
#define	PVR_DRM_DRIVER_DATE "20170530"

/*
 * Protects global PVRSRV_DATA on a multi device system. i.e. this is used to
 * protect the PVRSRVCommonDeviceXXXX() APIs in the Server common layer which
 * are not re-entrant for device creation and initialisation.
 */
static DEFINE_MUTEX(g_device_mutex);

static int pvr_pm_suspend(struct device *dev)
{
	struct drm_device *ddev = dev_get_drvdata(dev);
	struct pvr_drm_private *priv = ddev->dev_private;
	int ret;

	DRM_DEV_INFO(dev, "gpu device suspend enter\n");

	DRM_DEBUG_DRIVER("device %p\n", dev);

	ret = PVRSRVDeviceSuspend(priv->dev_node);

	DRM_DEV_INFO(dev, "gpu device suspend exit\n");

	return ret;
}

static int pvr_pm_resume(struct device *dev)
{
	struct drm_device *ddev = dev_get_drvdata(dev);
	struct pvr_drm_private *priv = ddev->dev_private;
	int ret;

	DRM_DEV_INFO(dev, "gpu device resume enter\n");

	DRM_DEBUG_DRIVER("device %p\n", dev);

	ret = PVRSRVDeviceResume(priv->dev_node);

	DRM_DEV_INFO(dev, "gpu device resume exit\n");

	return ret;
}

const struct dev_pm_ops pvr_pm_ops = {
	.suspend = pvr_pm_suspend,
	.resume  = pvr_pm_resume,
	.freeze  = pvr_pm_suspend,
	.restore = pvr_pm_resume,
};


#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0))
static
#endif
int pvr_drm_load(struct device *dev, struct drm_device *ddev, unsigned long flags)
{
	struct pvr_drm_private *priv = ddev->dev_private;
	enum PVRSRV_ERROR_TAG srv_err;
	int err, deviceId;

	DRM_DEBUG_DRIVER("device %p\n", ddev->dev);

	dev_set_drvdata(dev, ddev);

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

	if (!ddev->dev->dma_parms)
		ddev->dev->dma_parms = &priv->dma_parms;
	dma_set_max_seg_size(ddev->dev, DMA_BIT_MASK(32));

	mutex_lock(&g_device_mutex);

	srv_err = PVRSRVCommonDeviceCreate(dev, deviceId, &priv->dev_node);
	if (srv_err != PVRSRV_OK) {
		DRM_ERROR("failed to create device node for device %p (%s)\n",
			  ddev->dev, PVRSRVGetErrorString(srv_err));
		if (srv_err == PVRSRV_ERROR_PROBE_DEFER)
			err = -EPROBE_DEFER;
		else
			err = -ENODEV;
		goto err_unset_dma_parms;
	}

	err = PVRSRVDeviceInit(priv->dev_node);
	if (err) {
		DRM_ERROR("device %p initialisation failed (err=%d)\n",
			  ddev->dev, err);
		goto err_device_destroy;
	}


#if (PVRSRV_DEVICE_INIT_MODE == PVRSRV_LINUX_DEV_INIT_ON_PROBE)
	srv_err = PVRSRVCommonDeviceInitialise(priv->dev_node);
	if (srv_err != PVRSRV_OK) {
		err = -ENODEV;
		DRM_ERROR("device %p initialisation failed (err=%d)\n",
			  ddev->dev, err);
		goto err_device_deinit;
	}
#endif

	mutex_unlock(&g_device_mutex);

	return 0;

#if (PVRSRV_DEVICE_INIT_MODE == PVRSRV_LINUX_DEV_INIT_ON_PROBE)
err_device_deinit:
	drm_mode_config_cleanup(ddev);
	PVRSRVDeviceDeinit(priv->dev_node);
#endif
err_device_destroy:
	PVRSRVCommonDeviceDestroy(priv->dev_node);
err_unset_dma_parms:
	mutex_unlock(&g_device_mutex);
	if (ddev->dev->dma_parms == &priv->dma_parms)
		ddev->dev->dma_parms = NULL;
	return err;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0))
static
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
int pvr_drm_unload(struct drm_device *ddev)
#else
void pvr_drm_unload(struct drm_device *ddev)
#endif
{
	struct pvr_drm_private *priv = ddev->dev_private;

	DRM_DEBUG_DRIVER("device %p\n", ddev->dev);

	PVRSRVDeviceDeinit(priv->dev_node);

	mutex_lock(&g_device_mutex);
	PVRSRVCommonDeviceDestroy(priv->dev_node);
	mutex_unlock(&g_device_mutex);

	if (ddev->dev->dma_parms == &priv->dma_parms)
		ddev->dev->dma_parms = NULL;

	kfree(priv);
	ddev->dev_private = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	return 0;
#endif
}

int pvr_drm_open(struct drm_device *ddev, struct drm_file *dfile)
{
#if (PVRSRV_DEVICE_INIT_MODE == PVRSRV_LINUX_DEV_INIT_ON_OPEN)
	struct pvr_drm_private *priv = ddev->dev_private;

	return PVRSRVDeviceServicesOpen(priv->dev_node, dfile);
#else
	return 0;
#endif
}

void pvr_drm_release(struct drm_device *ddev, struct drm_file *dfile)
{
	struct pvr_drm_private *priv = ddev->dev_private;

	PVRSRVDeviceRelease(priv->dev_node, dfile);
}

struct drm_ioctl_desc pvr_drm_ioctls[128] = {
	DRM_IOCTL_DEF_DRV(PVR_SRVKM_CMD, PVRSRV_BridgeDispatchKM,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(PVR_SRVKM_INIT, drm_pvr_srvkm_init,
			  DRM_RENDER_ALLOW),
#if defined(SUPPORT_NATIVE_FENCE_SYNC) && !defined(USE_PVRSYNC_DEVNODE)
	DRM_IOCTL_DEF_DRV(PVR_SYNC_RENAME_CMD, pvr_sync_rename_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(PVR_SYNC_FORCE_SW_ONLY_CMD, pvr_sync_force_sw_only_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(PVR_SW_SYNC_CREATE_FENCE_CMD, pvr_sw_sync_create_fence_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(PVR_SW_SYNC_INC_CMD, pvr_sw_sync_inc_ioctl,
			  DRM_RENDER_ALLOW),
#endif
#if !defined(NO_HARDWARE)
	DRM_IOCTL_DEF_DRV(MTGPU_DEVICE_INIT, mtgpu_device_init_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_QUERY_INFO, mtgpu_query_info_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_BO_CREATE, mtgpu_bo_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_BO_FROM_USERPTR, mtgpu_bo_from_userptr_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_BO_GET_MMAP_OFFSET, mtgpu_bo_get_mmap_offset_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_BO_EXPORT_GLOBAL_HANDLE, mtgpu_bo_export_global_handle_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_BO_IMPORT_GLOBAL_HANDLE, mtgpu_bo_import_global_handle_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_VM_CONTEXT_CREATE, mtgpu_vm_context_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_VM_CONTEXT_DESTROY, mtgpu_vm_context_destroy_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_VM_MAP, mtgpu_vm_map_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_VM_UNMAP, mtgpu_vm_unmap_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_TIMELINE_CREATE, mtgpu_timeline_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_TIMELINE_DESTROY, mtgpu_timeline_destroy_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_TIMELINE_READ, mtgpu_timeline_read_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_FENCE_WAIT, mtgpu_fence_wait_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_FENCE_TO_FD, mtgpu_fence_to_fd_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_CONTEXT_CREATE, mtgpu_context_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_CONTEXT_DESTROY, mtgpu_context_destroy_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_JOB_SUBMIT, mtgpu_job_submit_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_SUBMIT, mtgpu_semaphore_submit_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_CREATE, mtgpu_semaphore_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_DESTROY, mtgpu_semaphore_destroy_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_CPU_SIGNAL, mtgpu_semaphore_cpu_signal_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_EXPORT_FD, mtgpu_semaphore_export_fd_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_CACHE_OP, mtgpu_cache_op_ioctl,
			  DRM_RENDER_ALLOW),
#if defined SUPPORT_DMA_TRANSFER
	DRM_IOCTL_DEF_DRV(MTGPU_DMA_TRANSFER, mtgpu_dma_transfer_ioctl,
			  DRM_RENDER_ALLOW),
#endif /* SUPPORT_DMA_TRANSFER */
	DRM_IOCTL_DEF_DRV(MTGPU_OBJECT_CREATE, mtgpu_object_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_OBJECT_DESTROY, mtgpu_object_destroy_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_TRANSPORT_LAYER, mtgpu_transport_layer_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_HWPERF, mtgpu_hwperf_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_NOTIFY_QUEUE_UPDATE, mtgpu_notify_queue_update_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_ALIGN_CHECK, mtgpu_align_check_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_VERSION_CHECK, mtgpu_version_check_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_JOB_CONTEXT_CREATE, mtgpu_job_context_create_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_JOB_CONTEXT_DESTROY, mtgpu_job_context_destroy_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_JOB_SUBMIT_V3, mtgpu_job_submit_ioctl_v3,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_JOB_APPEND, mtgpu_job_append_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_CODEC_WAIT, mtgpu_codec_wait_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_IMPORT_FD, mtgpu_semaphore_import_fd_ioctl,
			  DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTGPU_SEMAPHORE_WAIT, mtgpu_semaphore_wait_ioctl,
			  DRM_RENDER_ALLOW),
#endif
};

#if defined(CONFIG_COMPAT)
static long pvr_compat_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	unsigned int nr = DRM_IOCTL_NR(cmd);

	if (nr < DRM_COMMAND_BASE)
		return drm_compat_ioctl(file, cmd, arg);

	return drm_ioctl(file, cmd, arg);
}
#endif /* defined(CONFIG_COMPAT) */

const struct file_operations pvr_drm_fops = {
	.owner			= THIS_MODULE,
	.open			= drm_open,
	.release		= drm_release,
	.unlocked_ioctl		= drm_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl		= pvr_compat_ioctl,
#endif
	.mmap			= PVRSRV_MMap,
	.poll			= drm_poll,
	.read			= drm_read,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0))
	.fasync			= drm_fasync,
#endif
};

struct drm_driver pvr_drm_generic_driver = {
	.driver_features	= DRIVER_RENDER,

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	.load			= NULL,
	.unload			= NULL,
#else
	.load			= pvr_drm_load,
	.unload			= pvr_drm_unload,
#endif
	.open			= pvr_drm_open,
	.postclose		= pvr_drm_release,

	.ioctls			= pvr_drm_ioctls,
	.num_ioctls		= ARRAY_SIZE(pvr_drm_ioctls),
	.fops			= &pvr_drm_fops,

	.name			= PVR_DRM_DRIVER_NAME,
	.desc			= PVR_DRM_DRIVER_DESC,
	.date			= PVR_DRM_DRIVER_DATE,
	.major			= PVRVERSION_MAJ,
	.minor			= PVRVERSION_MIN,
	.patchlevel		= PVRVERSION_BUILD,
};
