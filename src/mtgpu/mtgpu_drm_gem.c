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
#include "mtgpu_drm_internal.h"
#include "mtgpu_drv.h"
#include "mtgpu_vpu.h"
#include "os-interface-drm.h"

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

bool mtgpu_is_fbcon_buffer(struct drm_file *file, struct drm_device *drm)
{
	if (!drm->fb_helper)
		return false;

	return file == drm->fb_helper->client.file;
}

void mtgpu_gem_buffer_clear(struct drm_gem_object *obj)
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

static struct sg_table *mtgpu_gem_dmabuf_map(struct dma_buf_attachment *attach,
					     enum dma_data_direction dir)
{
	int int_dir = dir;

	return mtgpu_gem_dmabuf_map_internal(attach, int_dir);
}

static void mtgpu_gem_dmabuf_unmap(struct dma_buf_attachment *attach,
				   struct sg_table *sgt,
				   enum dma_data_direction dir)
{
	int int_dir = dir;

	return mtgpu_gem_dmabuf_unmap_internal(attach, sgt, int_dir);
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
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

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
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

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
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

	err = mtgpu_vram_vmap(mtgpu_obj->handle, obj->size,
			      &private_data, &kaddr);
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
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

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

const struct dma_buf_ops mtgpu_gem_dmabuf_ops = {
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

const struct dma_buf_ops *mtgpu_get_gem_dmabuf_ops(void)
{
	return &mtgpu_gem_dmabuf_ops;
}

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

dma_addr_t mtgpu_fb_get_dma_addr(struct drm_framebuffer *fb,
				 struct drm_plane_state *state,
				 u32 plane)
{
	struct mtgpu_gem_object *mtgpu_obj;
	struct drm_gem_object *obj;
	dma_addr_t dev_addr;

	obj = drm_gem_fb_get_obj(fb, plane);
	if (!obj)
		return 0;

	mtgpu_obj = os_get_drm_gem_object_drvdata(obj);
	if (!mtgpu_obj)
		return 0;

	dev_addr = mtgpu_obj->dev_addr + fb->offsets[plane];
	dev_addr += fb->format->cpp[plane] * (state->src_x >> 16);
	dev_addr += fb->pitches[plane] * (state->src_y >> 16);

	return dev_addr;
}
