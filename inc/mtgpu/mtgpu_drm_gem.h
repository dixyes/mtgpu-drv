/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_GEM_H__
#define __MTGPU_DRM_GEM_H__

#include <drm/drm_gem.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include "pvrsrv.h"

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST)
#define iosys_map		dma_buf_map
#define iosys_map_set_vaddr	dma_buf_map_set_vaddr
#endif

#if !defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
extern const struct drm_gem_object_funcs mtgpu_gem_object_funcs;
#endif

#define to_mtgpu_obj(obj) container_of(obj, struct mtgpu_gem_object, base)

struct drm_plane_state;
struct drm_mode_create_dumb;

#define MTGEM_TYPE_GPU_START 0
#define MTGEM_TYPE_VPU_START 100
#define MTGPU_DUMB_BUFFER_FLAG_USE_SYSTEM (1 << 4)

struct mtgpu_gem_object {
	struct drm_gem_object base;
	struct sg_table *sgt;
	phys_addr_t cpu_addr;
	dma_addr_t dev_addr;
	u64 *cpu_pa_array;
	u64 cpu_pa_count;
	void *handle;
	u64 private_data;
	u32 type;
};

extern const struct vm_operations_struct mtgpu_gem_vm_ops;

void mtgpu_gem_object_free(struct drm_gem_object *obj);
int mtgpu_gem_vram_mmap(struct file *filp, struct vm_area_struct *vma);
int mtgpu_gem_dumb_create(struct drm_file *file, struct drm_device *drm,
			  struct drm_mode_create_dumb *args);
#if defined(OS_DRM_GEM_PRIME_EXPORT_HAS_TWO_ARGS)
struct dma_buf *mtgpu_gem_prime_export(struct drm_gem_object *obj, int flags);
#else
struct dma_buf *mtgpu_gem_prime_export(struct drm_device *drm,
				       struct drm_gem_object *obj, int flags);
#endif
struct drm_gem_object *mtgpu_gem_prime_import(struct drm_device *drm,
					      struct dma_buf *dma_buf);
struct drm_gem_object *mtgpu_gem_prime_import_sg_table(struct drm_device *dev,
						       struct dma_buf_attachment *attach,
						       struct sg_table *sgt);
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
int mtgpu_gem_prime_vmap(struct drm_gem_object *obj, struct iosys_map *map);
void mtgpu_gem_prime_vunmap(struct drm_gem_object *obj, struct iosys_map *map);
#else
void *mtgpu_gem_prime_vmap(struct drm_gem_object *obj);
void mtgpu_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr);
#endif

struct mtgpu_gem_object *mtgpu_fb_get_gem_obj(struct drm_framebuffer *fb,
					      u32 plane);
dma_addr_t mtgpu_fb_get_dma_addr(struct drm_framebuffer *fb,
				 struct drm_plane_state *state,
				 u32 plane);

#endif /* __MTGPU_DRM_GEM_H__ */
