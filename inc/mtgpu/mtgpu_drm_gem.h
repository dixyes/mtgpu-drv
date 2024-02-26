/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_GEM_H__
#define __MTGPU_DRM_GEM_H__

#include <drm/drm_gem.h>
#include <drm/drm_gem_framebuffer_helper.h>

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST)
#define iosys_map		dma_buf_map
#define iosys_map_set_vaddr	dma_buf_map_set_vaddr
#endif

#if !defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
extern const struct drm_gem_object_funcs mtgpu_gem_object_funcs;
#endif

struct drm_plane_state;

extern const struct vm_operations_struct mtgpu_gem_vm_ops;

#if defined(OS_DRM_GEM_PRIME_EXPORT_HAS_TWO_ARGS)
struct dma_buf *mtgpu_gem_prime_export(struct drm_gem_object *obj, int flags);
#else
struct dma_buf *mtgpu_gem_prime_export(struct drm_device *drm,
				       struct drm_gem_object *obj, int flags);
#endif
#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
int mtgpu_gem_prime_vmap(struct drm_gem_object *obj, struct iosys_map *map);
void mtgpu_gem_prime_vunmap(struct drm_gem_object *obj, struct iosys_map *map);
#else
void *mtgpu_gem_prime_vmap(struct drm_gem_object *obj);
void mtgpu_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr);
#endif

dma_addr_t mtgpu_fb_get_dma_addr(struct drm_framebuffer *fb,
				 struct drm_plane_state *state,
				 u32 plane);

#endif /* __MTGPU_DRM_GEM_H__ */
