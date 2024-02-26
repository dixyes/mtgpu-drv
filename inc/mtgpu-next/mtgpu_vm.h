/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_VM_H_
#define _MTGPU_VM_H_

struct drm_device;
struct drm_file;
struct mtgpu_vm_context;

void *mtgpu_vm_context_private_data(struct mtgpu_vm_context *vm_ctx);

int mtgpu_vm_context_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_vm_context_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_vm_map_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_vm_unmap_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_VM_H_ */
