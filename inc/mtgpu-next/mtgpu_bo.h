/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_BO_H_
#define _MTGPU_BO_H_

struct drm_device;
struct drm_file;

u64 mtgpu_bo_flag_from_pmr_flag(u64 pmr_flags);
u32 mtgpu_bo_domain_from_pmr_flag(u64 pmr_flags);

int mtgpu_bo_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_bo_from_userptr_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_bo_get_mmap_offset_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_bo_export_global_handle_ioctl(struct drm_device *drm, void *data,
					struct drm_file *file_priv);
int mtgpu_bo_import_global_handle_ioctl(struct drm_device *drm, void *data,
					struct drm_file *file_priv);
void mtgpu_bo_delete_global_handle(u64 global_handle);

#endif /* _MTGPU_BO_H_ */
