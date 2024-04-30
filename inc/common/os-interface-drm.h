/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __OS_INTERFACE_DRM_H__
#define __OS_INTERFACE_DRM_H__

#include "os-interface.h"
#include "mtgpu_dpcd_defs.h"

struct drm_crtc;
struct drm_encoder;
struct drm_connector;
struct drm_dp_aux;
struct drm_modeset_acquire_ctx;
struct drm_device;
struct drm_gem_object;
struct drm_file;
struct drm_mode_map_dumb;
struct drm_mode_destroy_dumb;
struct phy;
union phy_configure_opts;
struct videomode;
struct hdmi_codec_params;
struct sg_table;
struct dma_buf_attachment;
struct drm_mode_create_dumb;
struct edid;

DECLARE_OS_STRUCT_COMMON_FUNCS(drm_gem_object);

struct mutex *os_get_drm_device_mutex(struct drm_device *dev);
struct device *os_get_drm_device_base(struct drm_device *drm);
int os_drm_get_card_index(struct drm_device *dev);
int os_drm_get_render_index(struct drm_device *dev);
int os_drm_gem_handle_create(struct drm_file *file_priv,
			     struct drm_gem_object *obj,
			     u32 *handlep);
int os_drm_gem_handle_delete(struct drm_file *filp, u32 handle);
struct drm_gem_object *os_drm_gem_object_lookup(struct drm_file *filp, u32 handle);
void os_drm_gem_object_get(struct drm_gem_object *obj);
void os_drm_gem_object_put(struct drm_gem_object *obj);
void os_drm_gem_object_release(struct drm_gem_object *obj);
void os_set_drm_gem_object_funcs(struct drm_gem_object *obj);
int os_drm_gem_dumb_map_offset(struct drm_file *file,
			       struct drm_device *dev,
			       u32 handle,
			       u64 *offset);
void os_drm_gem_free_mmap_offset(struct drm_gem_object *obj);
int os_drm_gem_create_mmap_offset(struct drm_gem_object *obj);
__u64 os_drm_vma_node_offset_addr(struct drm_gem_object *obj);
void os_drm_prime_gem_destroy(struct drm_gem_object *obj, struct sg_table *sg);
void os_drm_gem_private_object_init(struct drm_device *dev,
				    struct drm_gem_object *obj,
				    size_t size);
struct drm_gem_object *os_drm_gem_prime_import(struct drm_device *dev,
					       struct dma_buf *dma_buf);
int os_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma);
int os_drm_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma);
void *os_get_vma_private_data(struct vm_area_struct *vma);
u32 os_get_drm_mode_destroy_dumb_handle(struct drm_mode_destroy_dumb *args);
u32 os_get_drm_mode_map_dumb_handle(struct drm_mode_map_dumb *args);
u64 *os_get_drm_mode_map_dumb_offset(struct drm_mode_map_dumb *args);
void os_get_drm_mode_create_dumb_args(struct drm_mode_create_dumb *args,
				      u32 *dumb_width,
				      u32 *dumb_height,
				      u32 *dumb_bpp,
				      u32 *dumb_flags);
void os_set_drm_mode_create_dumb_args(struct drm_mode_create_dumb *args,
				      u32 handle, u32 pitch, u64 size);
struct file *os_get_drm_file_filp(struct drm_file *file);

/* drm dp helper interface */
bool os_drm_dp_channel_eq_ok(const u8 link_status[DP_LINK_STATUS_SIZE], int lane_count);
bool os_drm_dp_clock_recovery_ok(const u8 link_status[DP_LINK_STATUS_SIZE], int lane_count);
u8 os_drm_dp_get_adjust_request_voltage(const u8 link_status[DP_LINK_STATUS_SIZE], int lane);
u8 os_drm_dp_get_adjust_request_pre_emphasis(const u8 link_status[DP_LINK_STATUS_SIZE], int lane);
void os_drm_dp_link_train_clock_recovery_delay(const struct drm_dp_aux *aux,
					       const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
void os_drm_dp_link_train_channel_eq_delay(const struct drm_dp_aux *aux,
					   const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
int os_drm_dp_bw_code_to_link_rate(u8 link_bw);
int os_drm_dp_psr_setup_time(const u8 psr_cap[EDP_PSR_RECEIVER_CAP_SIZE]);
int os_drm_dp_max_link_rate(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
u8 os_drm_dp_max_lane_count(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
bool os_drm_dp_enhanced_frame_cap(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
bool os_drm_dp_tps3_supported(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
bool os_drm_dp_tps4_supported(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
ssize_t os_drm_dp_dpcd_read(struct drm_dp_aux *aux, unsigned int offset, void *buffer, size_t size);
ssize_t os_drm_dp_dpcd_write(struct drm_dp_aux *aux, unsigned int offset, void *buffer,
			     size_t size);
ssize_t os_drm_dp_dpcd_readb(struct drm_dp_aux *aux, unsigned int offset, u8 *valuep);
ssize_t os_drm_dp_dpcd_writeb(struct drm_dp_aux *aux, unsigned int offset, u8 value);
int os_drm_dp_dpcd_read_link_status(struct drm_dp_aux *aux, u8 status[DP_LINK_STATUS_SIZE]);
int  os_drm_dp_aux_register(struct drm_dp_aux *aux);
void os_drm_dp_aux_unregister(struct drm_dp_aux *aux);

void os_drm_connector_set_status(struct drm_connector *connector, int status);
int os_drm_connector_get_status(struct drm_connector *connector);
struct drm_device *os_drm_connector_get_dev(struct drm_connector *connector);
void os_get_edid_vendor(const struct edid *edid, char *edid_vendor);
u32 os_get_edid_productid(const struct edid *edid);
void os_drm_edid_get_monitor_name(struct edid *edid, char *name, int bufsize);

/**
 * Interface for get members of the structure.
 */
/* members of videomode */
unsigned long os_get_videomode_pixelclock(struct videomode *vm);
u32 os_get_videomode_hactive(struct videomode *vm);
u32 os_get_videomode_hfront_porch(struct videomode *vm);
u32 os_get_videomode_hback_porch(struct videomode *vm);
u32 os_get_videomode_hsync_len(struct videomode *vm);
u32 os_get_videomode_vactive(struct videomode *vm);
u32 os_get_videomode_vfront_porch(struct videomode *vm);
u32 os_get_videomode_vback_porch(struct videomode *vm);
u32 os_get_videomode_vsync_len(struct videomode *vm);
u32 os_get_videomode_flags(struct videomode *vm);

#define OS_VIDEOMODE_MEMBER(ptr, member)	(os_get_videomode_##member(ptr))

/* members of hdmi_codec_params */
int os_get_hdmi_codec_params_sample_rate(struct hdmi_codec_params *param);
int os_get_hdmi_codec_params_sample_width(struct hdmi_codec_params *param);
int os_get_hdmi_codec_params_channels(struct hdmi_codec_params *param);

#define OS_HDMI_CODEC_PARAMS_MEMBER(ptr, member)	(os_get_hdmi_codec_params_##member(ptr))

/* members of drm_gem_object */
struct drm_device *os_get_drm_gem_object_dev(struct drm_gem_object *obj);
size_t os_get_drm_gem_object_size(struct drm_gem_object *obj);
struct dma_buf *os_get_drm_gem_object_dma_buf(struct drm_gem_object *obj);
struct dma_buf_attachment *os_get_drm_gem_object_import_attach(struct drm_gem_object *obj);

#define OS_DRM_GEM_OBJECT_MEMBER(ptr, member)	(os_get_drm_gem_object_##member(ptr))

int os_drm_helper_probe_detect(struct drm_connector *connector, struct drm_modeset_acquire_ctx *ctx,
			       bool force);
void os_drm_kms_helper_hotplug_event(struct drm_device *dev);

DECLARE_OS_STRUCT_COMMON_FUNCS(drm_crtc);
DECLARE_OS_STRUCT_COMMON_FUNCS(drm_encoder);
DECLARE_OS_STRUCT_COMMON_FUNCS(drm_connector);
DECLARE_OS_STRUCT_COMMON_FUNCS(videomode);
DECLARE_OS_STRUCT_COMMON_FUNCS(drm_dp_aux);

/* drm debug interface */
void os_drm_dev_printk(const struct device *dev, const char *level, const char *format, ...);
void os_drm_dev_dbg(const struct device *dev, unsigned int category, const char *format, ...);

#define OS_DRM_DEV_DEBUG(dev, fmt, ...)					\
	os_drm_dev_dbg(dev, OS_VAL(DRM_UT_CORE), fmt, ##__VA_ARGS__)
#define OS_DRM_DEV_INFO(dev, fmt, ...)					\
	os_drm_dev_printk(dev, KERN_INFO, fmt, ##__VA_ARGS__)
#define OS_DRM_DEV_ERROR(dev, fmt, ...)					\
	os_drm_dev_printk(dev, KERN_ERR, "*ERROR* " fmt, ##__VA_ARGS__)

void os_drm_printk(const char *level, const char *format, ...);
void os_drm_dbg(unsigned int category, const char *format, ...);
#define OS_DRM_DEBUG(fmt, ...)							\
	os_drm_dbg(OS_VAL(DRM_UT_CORE), fmt, ##__VA_ARGS__)
#define OS_DRM_INFO(fmt, ...)							\
	os_drm_printk(KERN_INFO, fmt, ##__VA_ARGS__)
#define OS_DRM_WARN(fmt, ...)							\
	os_drm_printk(KERN_WARNING, fmt, ##__VA_ARGS__)
#define OS_DRM_ERROR(fmt, ...)							\
	os_drm_printk(KERN_ERR, "*ERROR* " fmt, ##__VA_ARGS__)

#endif /* __OS_INTERFACE_DRM_H__ */
