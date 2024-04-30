/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRM_INTERNAL_H__
#define __MTGPU_DRM_INTERNAL_H__

struct dma_buf_attachment;
struct drm_gem_object;
struct drm_mode_create_dumb;
struct drm_device;
struct drm_file;
struct dma_buf_attachment;
struct sg_table;
struct dma_buf;
struct vm_area_struct;
struct drm_plane_state;
struct drm_framebuffer;
struct file;

#define MTGEM_TYPE_GPU_START 0
#define MTGEM_TYPE_VPU_START 100
#define MTGPU_DUMB_BUFFER_FLAG_USE_SYSTEM (1 << 4)

struct mtgpu_gem_object {
	struct drm_gem_object *obj;
	struct sg_table *sgt;
	phys_addr_t cpu_addr;
	dma_addr_t dev_addr;
	u64 *cpu_pa_array;
	u64 cpu_pa_count;
	void *handle;
	u64 private_data;
	u32 type;
	u64 global_handle;
};

/* The following functions are defined in open source file */
bool mtgpu_is_fbcon_buffer(struct drm_file *file, struct drm_device *drm);
void mtgpu_gem_buffer_clear(struct drm_gem_object *obj);
const struct dma_buf_ops *mtgpu_get_gem_dmabuf_ops(void);

/* The following functions are defined in close source file */
void mtgpu_gem_object_free(struct drm_gem_object *obj);
struct drm_gem_object *mtgpu_gem_object_create(struct drm_device *drm, size_t size, int flags);
int mtgpu_gem_dumb_create(struct drm_file *file, struct drm_device *drm,
                          struct drm_mode_create_dumb *args);
int mtgpu_gem_dmabuf_attach(struct dma_buf *dma_buf,
			    struct dma_buf_attachment *attach);
struct sg_table *mtgpu_gem_dmabuf_map_internal(struct dma_buf_attachment *attach,
					 int dir);
void mtgpu_gem_dmabuf_unmap_internal(struct dma_buf_attachment *attach,
			       struct sg_table *sgt, int dir);
int mtgpu_gem_dmabuf_mmap(struct dma_buf *dma_buf,
			  struct vm_area_struct *vma);
struct drm_gem_object *mtgpu_gem_prime_import(struct drm_device *drm,
                                              struct dma_buf *dma_buf);
struct drm_gem_object *mtgpu_gem_prime_import_sg_table(struct drm_device *drm,
                                                       struct dma_buf_attachment *attach,
                                                       struct sg_table *sgt);
int mtgpu_mmap(struct file *filp, struct vm_area_struct *vma);

#endif /* __MTGPU_DRM_INTERNAL_H__ */
