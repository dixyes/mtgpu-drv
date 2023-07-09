/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <linux/device.h>
#endif
#include <linux/dma-buf.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <drm/drm_crtc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_plane.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_fb_helper.h>

#include "mtgpu_drm_gem.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drv.h"
#include "mtgpu_segment.h"
#include "mtgpu_vpu.h"

void mtgpu_gem_object_free(struct drm_gem_object *obj)
{
	struct mtgpu_gem_object *mtgpu_obj = to_mtgpu_obj(obj);

	DRM_INFO("%s(), comm = %s, pid = %d, dev_addr = 0x%llx, size = 0x%lx\n",
		 __func__, current->comm, current->pid, mtgpu_obj->dev_addr, mtgpu_obj->base.size);

	if (mtgpu_obj->handle)
		mtgpu_vram_free(mtgpu_obj->handle);
	else if (obj->import_attach)
		drm_prime_gem_destroy(obj, mtgpu_obj->sgt);

	drm_gem_object_release(&mtgpu_obj->base);
	kfree(mtgpu_obj);
}

const struct vm_operations_struct mtgpu_gem_vm_ops = {
	.open	= drm_gem_vm_open,
	.close	= drm_gem_vm_close,
};

#if !defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
const struct drm_gem_object_funcs mtgpu_gem_object_funcs = {
	.export		= mtgpu_gem_prime_export,
	.free		= mtgpu_gem_object_free,
	.vmap		= mtgpu_gem_prime_vmap,
	.vunmap		= mtgpu_gem_prime_vunmap,
	.vm_ops		= &mtgpu_gem_vm_ops,
};
#endif

static bool mtgpu_is_fbcon_buffer(struct drm_file *file, struct drm_device *drm)
{
	if (!drm->fb_helper)
		return false;

	return file == drm->fb_helper->client.file;
}

static void mtgpu_gem_buffer_clear(struct drm_gem_object *obj)
{
	void *fb_addr;

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
	struct iosys_map map;

	if (mtgpu_gem_prime_vmap(obj, &map)) {
		DRM_INFO("vmap failed, fb didn't clear\n");
		return;
	}
	fb_addr = map.vaddr;
	memset(fb_addr, 0, obj->size);
	mtgpu_gem_prime_vunmap(obj, &map);
#else
	fb_addr = mtgpu_gem_prime_vmap(obj);
	if (IS_ERR(fb_addr)) {
		DRM_INFO("vmap failed, fb didn't clear\n");
		return;
	}
	memset(fb_addr, 0, obj->size);
	mtgpu_gem_prime_vunmap(obj, fb_addr);
#endif
}

static struct drm_gem_object *mtgpu_gem_object_create(struct drm_device *drm, size_t size, int flags)
{
	struct mtgpu_gem_object *mtgpu_obj;
	int segment_id = 0, err;

	WARN_ON(ALIGN(size, gpu_page_size) != size);
	size = ALIGN(size, gpu_page_size);

	mtgpu_obj = kzalloc(sizeof(*mtgpu_obj), GFP_KERNEL);
	if (!mtgpu_obj)
		return ERR_PTR(-ENOMEM);

#if !defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
	mtgpu_obj->base.funcs = &mtgpu_gem_object_funcs;
#endif

	drm_gem_private_object_init(drm, &mtgpu_obj->base, size);

	segment_id = mtgpu_vram_get_segment_for_display(drm);
	if (segment_id < 0) {
		DRM_ERROR("mtgpu_vram_get_segment_for_display() failed\n");
		err = segment_id;
		goto err_unref;
	}

	if (flags & MTGPU_DUMB_BUFFER_FLAG_USE_SYSTEM) {
		err = mtgpu_system_alloc(drm, size, &mtgpu_obj->cpu_pa_array, &mtgpu_obj->handle);
		if (err) {
			DRM_ERROR("mtgpu_system_alloc() %ld bytes failed\n", size);
			err = -ENOMEM;
			goto err_unref;
		}
		mtgpu_obj->cpu_pa_count = size / OSGetPageSize();
	} else {
		err = mtgpu_vram_alloc(drm, segment_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);
		if (err) {
			DRM_ERROR("mtgpu_vram_alloc() %ld bytes failed on segment %d\n", size, segment_id);
			err = -ENOMEM;
			goto err_unref;
		}
	}

	DRM_INFO("%s(), comm = %s, pid = %d, dev_addr = 0x%llx, size = 0x%lx\n",
		 __func__, current->comm, current->pid, mtgpu_obj->dev_addr, mtgpu_obj->base.size);
	return &mtgpu_obj->base;

err_unref:
	drm_gem_object_release(&mtgpu_obj->base);
	kfree(mtgpu_obj);

	return ERR_PTR(err);
}

int mtgpu_gem_dumb_create(struct drm_file *file, struct drm_device *drm,
			  struct drm_mode_create_dumb *args)
{
	struct drm_gem_object *obj;
	u32 handle, pitch;
	size_t size;
	int err;

	pitch = args->width * (ALIGN(args->bpp, 8) >> 3);
	size = ALIGN(pitch * args->height, gpu_page_size);

#if defined(CONFIG_QUYUAN2_HAPS)
	if (args->flags == 1) {
		/* Temporary for test : PVRIC need header space */
		u32 header_len = ((args->width  + 15) / 16) *
				 ((args->height + 3) / 4);
		header_len = ALIGN(header_len, 256);
		size = size + header_len;
		size = ALIGN(size, gpu_page_size);
	}
#endif

	obj = mtgpu_gem_object_create(drm, size, args->flags);
	if (IS_ERR(obj))
		return PTR_ERR(obj);

	/*
	 * Clear the fb, otherwise garbage maybe observed when switch to fbcon.
	 * drm->fb_helper->client.file will be set as a new drm_file which allocated
	 * in fbcon setup. The dumb_create func also has a drm_file as a input param.
	 * During dumb_create, if these two drm_file matched, which indicates the
	 * dumb_create is triggered by fbcon task, we should clear fb at this time.
	 */
	if (mtgpu_is_fbcon_buffer(file, drm))
		mtgpu_gem_buffer_clear(obj);

	err = drm_gem_handle_create(file, obj, &handle);
	if (err) {
		DRM_ERROR("failed to create gem handle\n");
		goto exit;
	}

	args->handle = handle;
	args->pitch = pitch;
	args->size = size;

	DRM_DEBUG("%s(), pitch = %d, size = %zu, handle = %u\n",
		  __func__, pitch, size, handle);
exit:
#if defined(OS_FUNC_DRM_GEM_OBJECT_PUT_UNLOCKED_EXIST)
	drm_gem_object_put_unlocked(obj);
#else
	drm_gem_object_put(obj);
#endif
	return err;
}

static int mtgpu_gem_dmabuf_attach(struct dma_buf *dma_buf,
				   struct dma_buf_attachment *attach)
{
	return 0;
}

static struct sg_table *
mtgpu_gem_dmabuf_map(struct dma_buf_attachment *attach,
		     enum dma_data_direction dir)
{
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct mtgpu_gem_object *mtgpu_obj = to_mtgpu_obj(obj);
	struct sg_table *sgt;
	struct scatterlist *sgl;
	u64 sg_idx = 0;

	sgt = kmalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	if ((obj->dev->dev == attach->dev->parent->parent) && mtgpu_obj->dev_addr) {
		/* This gem object was exported to our own gpu driver,
		 * and it was a vram backed object. */
		if (sg_alloc_table(sgt, 1, GFP_KERNEL)) {
			DRM_ERROR("sgtable alloc single sglist failed\n");
			goto err_free_sgt;
		}
		sg_dma_address(sgt->sgl) = mtgpu_obj->dev_addr;
		sg_dma_len(sgt->sgl) = obj->size;
	} else {
		/* This gem object was exported to other drm driver,
		 * it must be a system backed object. */
		if (sg_alloc_table(sgt, mtgpu_obj->cpu_pa_count, GFP_KERNEL)) {
			DRM_ERROR("sgtable alloc multi sglist failed\n");
			goto err_free_sgt;
		}
		for_each_sg(sgt->sgl, sgl, sgt->nents, sg_idx) {
			sg_dma_address(sgl) = mtgpu_obj->cpu_pa_array[sg_idx];
			sg_dma_len(sgl) = OSGetPageSize();
		}
	}

	return sgt;

err_free_sgt:
	kfree(sgt);
	return NULL;
}

static void mtgpu_gem_dmabuf_unmap(struct dma_buf_attachment *attach,
				   struct sg_table *sgt,
				   enum dma_data_direction dir)
{
	sg_free_table(sgt);
	kfree(sgt);
}

static int mtgpu_gem_dmabuf_mmap(struct dma_buf *dma_buf,
				 struct vm_area_struct *vma)
{
	struct mtgpu_gem_object *mtgpu_obj;
	struct drm_gem_object *obj = dma_buf->priv;

	mutex_lock(&obj->dev->struct_mutex);
	drm_gem_mmap_obj(obj, obj->size, vma);
	mutex_unlock(&obj->dev->struct_mutex);

	mtgpu_obj = to_mtgpu_obj(obj);

#if defined(OS_FUNC_DRM_GEM_OBJECT_PUT_UNLOCKED_EXIST)
	drm_gem_object_put_unlocked(obj);
#else
	drm_gem_object_put(obj);
#endif
	return mtgpu_vram_mmap(mtgpu_obj->handle, vma);
}

#if defined(OS_STRUCT_DMA_BUF_MAP_EXIST) || defined(OS_STRUCT_IOSYS_MAP_EXIST)
static int mtgpu_gem_dmabuf_vmap(struct dma_buf *dma_buf, struct iosys_map *map)
{
	struct drm_gem_object *obj = dma_buf->priv;

	return mtgpu_gem_prime_vmap(obj, map);
}

static void mtgpu_gem_dmabuf_vunmap(struct dma_buf *dma_buf, struct iosys_map *map)
{
	struct drm_gem_object *obj = dma_buf->priv;

	mtgpu_gem_prime_vunmap(obj, map);
}

int mtgpu_gem_prime_vmap(struct drm_gem_object *obj, struct iosys_map *map)
{
	int err;
	void *kaddr = NULL;
	u64 private_data = 0;
	struct mtgpu_gem_object *mtgpu_obj = to_mtgpu_obj(obj);

	err = mtgpu_vram_vmap(mtgpu_obj->handle, obj->size, &private_data, &kaddr);
	if (err) {
		DRM_ERROR("failed to acquire cpu kernel address for dev_addr 0x%llx\n",
			  mtgpu_obj->dev_addr);
		return err;
	}
	mtgpu_obj->private_data = private_data;

	iosys_map_set_vaddr(map, kaddr);

	return 0;
}

void mtgpu_gem_prime_vunmap(struct drm_gem_object *obj, struct iosys_map *map)
{
	struct mtgpu_gem_object *mtgpu_obj = to_mtgpu_obj(obj);

	mtgpu_vram_vunmap(mtgpu_obj->handle, mtgpu_obj->private_data);
}
#else
static void *mtgpu_gem_dmabuf_vmap(struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;
	return mtgpu_gem_prime_vmap(obj);
}

static void mtgpu_gem_dmabuf_vunmap(struct dma_buf *dma_buf, void *vaddr)
{
	struct drm_gem_object *obj = dma_buf->priv;
	mtgpu_gem_prime_vunmap(obj, vaddr);
}

void *mtgpu_gem_prime_vmap(struct drm_gem_object *obj)
{
	int err;
	void *kaddr = NULL;
	u64 private_data = 0;
	struct mtgpu_gem_object *mtgpu_obj = to_mtgpu_obj(obj);

	err = mtgpu_vram_vmap(mtgpu_obj->handle, obj->size, &private_data, &kaddr);
	if(err) {
		DRM_ERROR("failed to acquire cpu kernel address for dev_addr 0x%llx\n",
			  mtgpu_obj->dev_addr);
		return NULL;
	}
	mtgpu_obj->private_data = private_data;

	return kaddr;
}

void mtgpu_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	struct mtgpu_gem_object *mtgpu_obj = to_mtgpu_obj(obj);
	mtgpu_vram_vunmap(mtgpu_obj->handle, mtgpu_obj->private_data);
}
#endif

#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_MAP)
static void *mtgpu_gem_dmabuf_kmap(struct dma_buf *dma_buf,
				  unsigned long page_num)
{
	return NULL;
}
#endif

static const struct dma_buf_ops mtgpu_gem_dmabuf_ops = {
	.attach		= mtgpu_gem_dmabuf_attach,
	.map_dma_buf	= mtgpu_gem_dmabuf_map,
	.unmap_dma_buf	= mtgpu_gem_dmabuf_unmap,
	.release	= drm_gem_dmabuf_release,
#if defined(OS_STRUCT_DMA_BUF_OPS_HAS_MAP)
	.map		= mtgpu_gem_dmabuf_kmap,
#endif
	.mmap		= mtgpu_gem_dmabuf_mmap,
	.vmap		= mtgpu_gem_dmabuf_vmap,
	.vunmap		= mtgpu_gem_dmabuf_vunmap
};

#if defined(OS_DRM_GEM_PRIME_EXPORT_HAS_TWO_ARGS)
struct dma_buf *mtgpu_gem_prime_export(struct drm_gem_object *obj, int flags)
{
	DEFINE_DMA_BUF_EXPORT_INFO(export_info);

	export_info.ops = &mtgpu_gem_dmabuf_ops;
	export_info.size = obj->size;
	export_info.flags = flags;
	export_info.priv = obj;

	return drm_gem_dmabuf_export(obj->dev, &export_info);
}
#else
struct dma_buf *mtgpu_gem_prime_export(struct drm_device *drm,
				       struct drm_gem_object *obj, int flags)
{
	DEFINE_DMA_BUF_EXPORT_INFO(export_info);

	export_info.ops = &mtgpu_gem_dmabuf_ops;
	export_info.size = obj->size;
	export_info.flags = flags;
	export_info.priv = obj;

	return drm_gem_dmabuf_export(drm, &export_info);
}
#endif

struct drm_gem_object *mtgpu_gem_prime_import(struct drm_device *drm,
					      struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;

	if (obj->dev == drm) {
		WARN_ON(dma_buf->ops != &mtgpu_gem_dmabuf_ops);

		/*
		 * The dmabuf is one of ours, so return the associated
		 * MTGPU GEM object, rather than create a new one.
		 */
		drm_gem_object_get(obj);

		return obj;
	}

	return drm_gem_prime_import(drm, dma_buf);
}

struct drm_gem_object *mtgpu_gem_prime_import_sg_table(struct drm_device *drm,
						       struct dma_buf_attachment *attach,
						       struct sg_table *sgt)
{
	struct mtgpu_gem_object *mtgpu_obj;
	int err;

	mtgpu_obj = kzalloc(sizeof(*mtgpu_obj), GFP_KERNEL);
	if (!mtgpu_obj)
		return ERR_PTR(-ENOMEM);

	drm_gem_private_object_init(drm, &mtgpu_obj->base,
				    attach->dmabuf->size);

	mtgpu_obj->sgt = sgt;

	/* We only expect a single entry for card memory */
	if (mtgpu_obj->sgt->nents != 1) {
		err = -EINVAL;
		goto err_obj_unref;
	}

	mtgpu_obj->dev_addr = sg_dma_address(mtgpu_obj->sgt->sgl);

	return &mtgpu_obj->base;

err_obj_unref:
	drm_gem_object_put(&mtgpu_obj->base);

	return ERR_PTR(err);
}

int mtgpu_gem_vram_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct mtgpu_gem_object *mtgpu_obj;
	struct drm_gem_object *obj;
	int ret;

	/*
	 * NOTE:
	 * If drm_gem_mmap() was failed, maybe this buffer was alloacted
	 * through bridge call other than dumb create. Use mtgpu_mmap instead.
	 */
	ret = drm_gem_mmap(filp, vma);
	if (ret)
		return mtgpu_mmap(filp, vma);

	obj = vma->vm_private_data;
	mtgpu_obj = to_mtgpu_obj(obj);

	if (mtgpu_obj->type >= MTGEM_TYPE_VPU_START)
		return mtvpu_gem_mmap_obj(vma->vm_private_data, vma);

#if defined(OS_FUNC_DRM_GEM_OBJECT_PUT_UNLOCKED_EXIST)
	drm_gem_object_put_unlocked(obj);
#else
	drm_gem_object_put(obj);
#endif
	return mtgpu_vram_mmap(mtgpu_obj->handle, vma);
}

struct mtgpu_gem_object *mtgpu_fb_get_gem_obj(struct drm_framebuffer *fb,
					      u32 plane)
{
	struct drm_gem_object *obj;

	obj = drm_gem_fb_get_obj(fb, plane);
	if (!obj)
		return NULL;

	return to_mtgpu_obj(obj);
}

dma_addr_t mtgpu_fb_get_dma_addr(struct drm_framebuffer *fb,
				 struct drm_plane_state *state,
				 u32 plane)
{
	struct mtgpu_gem_object *mtgpu_obj;
	dma_addr_t dev_addr;

	mtgpu_obj = mtgpu_fb_get_gem_obj(fb, plane);
	if (!mtgpu_obj)
		return 0;

	dev_addr = mtgpu_obj->dev_addr + fb->offsets[plane];
	dev_addr += fb->format->cpp[plane] * (state->src_x >> 16);
	dev_addr += fb->pitches[plane] * (state->src_y >> 16);

	return dev_addr;
}
