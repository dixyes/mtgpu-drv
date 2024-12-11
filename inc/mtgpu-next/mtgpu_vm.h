/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_VM_H_
#define _MTGPU_VM_H_

struct drm_device;
struct drm_file;
struct DEVMEM_HEAP_TAG;
struct _DEVMEMINT_MAPPING_;
struct mutex;

#define DEVMEM_HEAPNAME_MAXLENGTH 160

struct mtgpu_vm_bo {
	struct _DEVMEMINT_MAPPING_ *mapping;
	struct DEVMEM_HEAP_TAG *heap;
	void *pmr;
	uint64_t dev_vaddr;
	void *cpu_vaddr;
	void *map_data;
};

struct mtgpu_vm_context {
	struct _DEVMEMINT_CTX_ *devmem_int_ctx;
	struct _DEVMEMINT_HEAP_ *reserved_int_heap;
	struct _DEVMEMINT_HEAP_ **app_int_heaps;
	u32 app_heap_count;
	struct rb_root_cached *root;
	struct mutex *tree_lock;	/* Mutex lock for tree access */
	u32 reserved_heap_count;
	struct DEVMEM_HEAP_TAG **reserved_heap_array;
	struct mtgpu_vm_bo *yuv_csc_buffer;
	struct mtgpu_vm_bo *pb_bo;

	struct mtgpu_job_item_list *job_list;
};

int mtgpu_vm_bo_create_and_map(PVRSRV_DEVICE_NODE *dev_node,
			       struct mtgpu_vm_context *vm_ctx,
			       char *heap_name,
			       u64 size,
			       u64 flags,
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
