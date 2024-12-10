/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_print.h>
#include <drm/drm_gem.h>
#include <drm/drm_prime.h>
#include <drm/drm_drv.h>
#if defined(OS_DRM_DRM_DP_HELPER_H_EXIST)
#include <drm/drm_dp_helper.h>
#elif defined(OS_DRM_DP_DRM_DP_HELPER_H_EXIST)
#include <drm/dp/drm_dp_helper.h>
#elif defined(OS_DRM_DISPLAY_DRM_DP_HELPER_H_EXIST)
#include <drm/display/drm_dp_helper.h>
#endif
#include <drm/drm_crtc.h>
#include <drm/drm_encoder.h>
#include <drm/drm_crtc_helper.h>
#include <uapi/drm/drm.h>
#include <sound/hdmi-codec.h>
#include <video/videomode.h>
#if defined(OS_DRM_DRM_PROBE_HELPER_H_EXIST)
#include <drm/drm_probe_helper.h>
#endif

#include "mtgpu_drm_gem.h"
#include "mtgpu_phy_dp.h"
#include "os-interface-drm.h"

IMPLEMENT_OS_STRUCT_COMMON_FUNCS(drm_gem_object);

struct mutex *os_get_drm_device_mutex(struct drm_device *drm)
{
	return &drm->struct_mutex;
}

struct device *os_get_drm_device_base(struct drm_device *drm)
{
	return drm->dev;
}

bool os_drm_is_registered(struct drm_device *dev)
{
	return dev->registered;
}

int os_drm_get_card_index(struct drm_device *dev)
{
	return dev->primary->index;
}

int os_drm_get_render_index(struct drm_device *dev)
{
	return dev->render->index;
}

int os_drm_gem_handle_create(struct drm_file *file_priv,
			     struct drm_gem_object *obj,
			     u32 *handlep)
{
	return drm_gem_handle_create(file_priv, obj, handlep);
}

int os_drm_gem_handle_delete(struct drm_file *filp, u32 handle)
{
	return drm_gem_handle_delete(filp, handle);
}

/* get drm_gem_object members */
IMPLEMENT_GET_OS_MEMBER_FUNC(drm_gem_object, dev);
IMPLEMENT_GET_OS_MEMBER_FUNC(drm_gem_object, size);
IMPLEMENT_GET_OS_MEMBER_FUNC(drm_gem_object, dma_buf);
IMPLEMENT_GET_OS_MEMBER_FUNC(drm_gem_object, import_attach);

struct drm_gem_object *os_drm_gem_object_lookup(struct drm_file *filp, u32 handle)
{
	return drm_gem_object_lookup(filp, handle);
}

void os_drm_gem_object_get(struct drm_gem_object *obj)
{
	drm_gem_object_get(obj);
}

void os_drm_gem_object_put(struct drm_gem_object *obj)
{
#if defined(OS_FUNC_DRM_GEM_OBJECT_PUT_UNLOCKED_EXIST)
	drm_gem_object_put_unlocked(obj);
#else
	drm_gem_object_put(obj);
#endif
}

void os_drm_gem_object_release(struct drm_gem_object *obj)
{
	drm_gem_object_release(obj);
}

void os_set_drm_gem_object_funcs(struct drm_gem_object *obj)
{
#if !defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
	obj->funcs = &mtgpu_gem_object_funcs;
#endif
}

int os_drm_gem_dumb_map_offset(struct drm_file *file,
			       struct drm_device *dev,
			       u32 handle,
			       u64 *offset)
{
	return drm_gem_dumb_map_offset(file, dev, handle, offset);
}

void os_drm_gem_free_mmap_offset(struct drm_gem_object *obj)
{
	drm_gem_free_mmap_offset(obj);
}

int os_drm_gem_create_mmap_offset(struct drm_gem_object *obj)
{
	return drm_gem_create_mmap_offset(obj);
}

__u64 os_drm_vma_node_offset_addr(struct drm_gem_object *obj)
{
	return drm_vma_node_offset_addr(&obj->vma_node);
}

void os_drm_prime_gem_destroy(struct drm_gem_object *obj, struct sg_table *sg)
{
	drm_prime_gem_destroy(obj, sg);
}

void os_drm_gem_private_object_init(struct drm_device *dev,
				    struct drm_gem_object *obj,
				    size_t size)
{
	drm_gem_private_object_init(dev, obj, size);
}

struct drm_gem_object *os_drm_gem_prime_import(struct drm_device *drm,
					       struct dma_buf *dma_buf)
{
	return drm_gem_prime_import(drm, dma_buf);
}

int os_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return drm_gem_mmap(filp, vma);
}

int os_drm_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	return drm_gem_mmap_obj(obj, obj->size, vma);
}

void *os_get_vma_private_data(struct vm_area_struct *vma)
{
	return vma->vm_private_data;
}

u32 os_get_drm_mode_destroy_dumb_handle(struct drm_mode_destroy_dumb *args)
{
	return args->handle;
}

u32 os_get_drm_mode_map_dumb_handle(struct drm_mode_map_dumb *args)
{
	return args->handle;
}

u64 *os_get_drm_mode_map_dumb_offset(struct drm_mode_map_dumb *args)
{
	return &args->offset;
}

void os_get_drm_mode_create_dumb_args(struct drm_mode_create_dumb *args,
				      u32 *dumb_width,
				      u32 *dumb_height,
				      u32 *dumb_bpp,
				      u32 *dumb_flags)
{
	*dumb_width = args->width;
	*dumb_height = args->height;
	*dumb_bpp = args->bpp;
	*dumb_flags = args->flags;
}

void os_set_drm_mode_create_dumb_args(struct drm_mode_create_dumb *args,
				      u32 handle, u32 pitch, u64 size)
{
	args->handle = handle;
	args->pitch = pitch;
	args->size = size;
}

struct file *os_get_drm_file_filp(struct drm_file *file)
{
	return file->filp;
}

/* drm dp helper interface */
bool os_drm_dp_channel_eq_ok(const u8 link_status[DP_LINK_STATUS_SIZE], int lane_count)
{
	return drm_dp_channel_eq_ok(link_status, lane_count);
}

bool os_drm_dp_clock_recovery_ok(const u8 link_status[DP_LINK_STATUS_SIZE], int lane_count)
{
	return drm_dp_clock_recovery_ok(link_status, lane_count);
}

u8 os_drm_dp_get_adjust_request_voltage(const u8 link_status[DP_LINK_STATUS_SIZE], int lane)
{
	return drm_dp_get_adjust_request_voltage(link_status, lane);
}

u8 os_drm_dp_get_adjust_request_pre_emphasis(const u8 link_status[DP_LINK_STATUS_SIZE], int lane)
{
	return drm_dp_get_adjust_request_pre_emphasis(link_status, lane);
}

void os_drm_dp_link_train_clock_recovery_delay(const struct drm_dp_aux *aux,
					       const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
#if defined(OS_DRM_DP_LINK_TRAIN_CLOCK_RECOVERY_DELAY_HAS_TWO_ARGS)
	drm_dp_link_train_clock_recovery_delay(aux, dpcd);
#else
	drm_dp_link_train_clock_recovery_delay(dpcd);
#endif
}

void os_drm_dp_link_train_channel_eq_delay(const struct drm_dp_aux *aux,
					   const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
#if defined(OS_DRM_DP_LINK_TRAIN_CHANNEL_EQ_DELAY_HAS_TWO_ARGS)
	drm_dp_link_train_channel_eq_delay(aux, dpcd);
#else
	drm_dp_link_train_channel_eq_delay(dpcd);
#endif
}

int os_drm_dp_bw_code_to_link_rate(u8 link_bw)
{
	return drm_dp_bw_code_to_link_rate(link_bw);
}

int os_drm_dp_psr_setup_time(const u8 psr_cap[EDP_PSR_RECEIVER_CAP_SIZE])
{
	return drm_dp_psr_setup_time(psr_cap);
}

int os_drm_dp_max_link_rate(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return drm_dp_max_link_rate(dpcd);
}

u8 os_drm_dp_max_lane_count(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return drm_dp_max_lane_count(dpcd);
}

bool os_drm_dp_enhanced_frame_cap(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return drm_dp_enhanced_frame_cap(dpcd);
}

bool os_drm_dp_tps3_supported(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return drm_dp_tps3_supported(dpcd);
}

bool os_drm_dp_tps4_supported(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return drm_dp_tps4_supported(dpcd);
}

ssize_t os_drm_dp_dpcd_read(struct drm_dp_aux *aux, unsigned int offset, void *buffer, size_t size)
{
	return drm_dp_dpcd_read(aux, offset, buffer, size);
}

ssize_t os_drm_dp_dpcd_write(struct drm_dp_aux *aux, unsigned int offset, void *buffer, size_t size)
{
	return drm_dp_dpcd_write(aux, offset, buffer, size);
}

ssize_t os_drm_dp_dpcd_readb(struct drm_dp_aux *aux, unsigned int offset, u8 *valuep)
{
	return drm_dp_dpcd_readb(aux, offset, valuep);
}

ssize_t os_drm_dp_dpcd_writeb(struct drm_dp_aux *aux, unsigned int offset, u8 value)
{
	return drm_dp_dpcd_writeb(aux, offset, value);
}

int os_drm_dp_dpcd_read_link_status(struct drm_dp_aux *aux, u8 status[DP_LINK_STATUS_SIZE])
{
	return drm_dp_dpcd_read_link_status(aux, status);
}

int  os_drm_dp_aux_register(struct drm_dp_aux *aux)
{
	return drm_dp_aux_register(aux);
}

void os_drm_dp_aux_unregister(struct drm_dp_aux *aux)
{
	drm_dp_aux_unregister(aux);
}

IMPLEMENT_OS_STRUCT_COMMON_FUNCS(drm_crtc);
IMPLEMENT_OS_STRUCT_COMMON_FUNCS(drm_encoder);
IMPLEMENT_OS_STRUCT_COMMON_FUNCS(drm_connector);
IMPLEMENT_OS_STRUCT_COMMON_FUNCS(videomode);
IMPLEMENT_OS_STRUCT_COMMON_FUNCS(drm_dp_aux);

int os_drm_helper_probe_detect(struct drm_connector *connector, struct drm_modeset_acquire_ctx *ctx,
			       bool force)
{
	return drm_helper_probe_detect(connector, ctx, force);
}

void os_drm_kms_helper_hotplug_event(struct drm_device *dev)
{
	drm_kms_helper_hotplug_event(dev);
}

void os_drm_connector_set_status(struct drm_connector *connector, int status)
{
	connector->status = status;
}

int os_drm_connector_get_status(struct drm_connector *connector)
{
	return connector->status;
}

struct drm_device *os_drm_connector_get_dev(struct drm_connector *connector)
{
	return connector->dev;
}

void os_drm_edid_get_monitor_name(struct edid *edid, char *name, int bufsize)
{
	drm_edid_get_monitor_name(edid, name, bufsize);
}

/* get videomode member */
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, pixelclock);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, hactive);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, hfront_porch);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, hback_porch);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, hsync_len);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, vactive);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, vfront_porch);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, vback_porch);
IMPLEMENT_GET_OS_MEMBER_FUNC(videomode, vsync_len);

/* Special member 'flags': need to be forced to convert to u32 */
u32 os_get_videomode_flags(struct videomode *vm)
{
	return (u32)(vm->flags);
}

/* get hdmi_codec_params member */
IMPLEMENT_GET_OS_MEMBER_FUNC(hdmi_codec_params, sample_rate);
IMPLEMENT_GET_OS_MEMBER_FUNC(hdmi_codec_params, sample_width);
IMPLEMENT_GET_OS_MEMBER_FUNC(hdmi_codec_params, channels);

/* drm debug interface */
void os_drm_dev_printk(const struct device *dev, const char *level, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;

	if (dev)
		dev_printk(level, dev, "[" DRM_NAME ":%ps] %pV",
			   __builtin_return_address(0), &vaf);
	else
		printk("%s" "[" DRM_NAME ":%ps] %pV",
		       level, __builtin_return_address(0), &vaf);

	va_end(args);
}

#if defined(__drm_debug_enabled)
	#define OS_DEBUG_CONDITION(x)	(!__drm_debug_enabled(x))
#elif defined(OS_FUNC_DRM_DEBUG_ENABLED_EXIST)
	#define OS_DEBUG_CONDITION(x)	(!drm_debug_enabled(x))
#elif defined(OS_GLOBAL_VARIABLE_DRM_DEBUG_EXIST)
	#define OS_DEBUG_CONDITION(x)	(!(drm_debug & (x)))
#else
	#define OS_DEBUG_CONDITION(x)	false
#endif

void os_drm_dev_dbg(const struct device *dev, unsigned int category, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	if (OS_DEBUG_CONDITION(category))
		return;

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;

	if (dev)
		dev_printk(KERN_DEBUG, dev, "[" DRM_NAME ":%ps] %pV",
			   __builtin_return_address(0), &vaf);
	else
		printk(KERN_DEBUG "[" DRM_NAME ":%ps] %pV",
		       __builtin_return_address(0), &vaf);

	va_end(args);
}

void os_drm_dbg(unsigned int category, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	if (OS_DEBUG_CONDITION(category))
		return;

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;

	printk(KERN_DEBUG "[" DRM_NAME ":%ps] %pV",
	       __builtin_return_address(0), &vaf);

	va_end(args);
}

void os_drm_printk(const char *level, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;

	printk("%s" "[" DRM_NAME ":%ps] %pV",
	       level, __builtin_return_address(0), &vaf);

	va_end(args);
}
