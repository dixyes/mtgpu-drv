/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_VM_H_
#define _MTGPU_VM_H_

struct drm_device;
struct drm_file;
struct mtgpu_vm_context;
struct DEVMEM_HEAP_TAG;
struct _DEVMEMINT_MAPPING_;

#define DEVMEM_HEAPNAME_MAXLENGTH 160

struct mtgpu_vm_bo {
	struct _DEVMEMINT_MAPPING_ *mapping;
	struct DEVMEM_HEAP_TAG *heap;
	void *pmr;
	uint64_t dev_vaddr;
	void *cpu_vaddr;
};

int mtgpu_vm_bo_create_and_map(PVRSRV_DEVICE_NODE *dev_node,
			       struct mtgpu_vm_context *vm_ctx,
			       char *heap_name,
			       u64 size,
			       struct mtgpu_vm_bo **buffer_obj);
int mtgpu_vm_bo_unmap_and_destroy(struct mtgpu_vm_bo *bo);

int mtgpu_vm_reserved_heap_create(struct device *dev,
				  struct mtgpu_vm_context *vm_ctx,
				  u64 heap_base,
				  u64 heap_length,
				  u32 log2_page_size,
				  char *heap_name,
				  DEVMEM_HEAP **heap_out);
void mtgpu_vm_reserved_heap_destroy(DEVMEM_HEAP *heap);
int mtgpu_vm_reserved_heap_populate(PVRSRV_DEVICE_NODE *dev_node,
				    struct mtgpu_vm_context *vm_ctx);
void mtgpu_vm_reserved_heap_cleanup(struct mtgpu_vm_context *vm_ctx);

void *mtgpu_vm_context_private_data(struct mtgpu_vm_context *vm_ctx);

int mtgpu_vm_context_create_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_vm_context_destroy_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_vm_map_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_vm_unmap_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_VM_H_ */
