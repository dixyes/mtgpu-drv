/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/timer.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <drm/drm_device.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#endif
#ifdef SUPPORT_ION
#include "ion/ion.h"
#endif

#include "pvrsrv.h"
#include "mtgpu_mdev.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drm_gem.h"
#include "mtvpu_api.h"
#include "mtvpu_pool.h"
#include "mtvpu_drv.h"
#include "misc.h"
#include "vdi.h"
#include "helper.h"

bool is_guest_cmds = false;

struct mt_timer_struct {
       struct timer_list timer;
       void *data;
};

struct file_operations vinfo_fops = {
	.owner = THIS_MODULE,
	.read = vpu_info_read,
};

struct file_operations fwinfo_fops = {
	.owner = THIS_MODULE,
	.read = fw_info_read,
};

struct file_operations *get_vinfo_fops(void)
{
	return &vinfo_fops;
}

struct file_operations *get_fwinfo_fops(void)
{
	return &fwinfo_fops;
}

void VLOG(int level, const char *fmt, ...)
{
	va_list args;
	int r;

	// Eliminate log level bigger than warning.
	// For debug, you can change 'WARN' to 'MAX_LOG_LEVEL'
	if (level > WARN)
		return;

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	return;
}

Uint64 osal_gettime(void)
{
	u64 tv = ktime_get_ns();

	return ((Uint64)tv / 1000000);
}

void *osal_malloc(int size)
{
	return kmalloc(size, GFP_KERNEL);
}

void osal_free(void *p)
{
	kfree(p);
}

void osal_memcpy(void *dst, const void *src, int count)
{
	memcpy(dst, src, count);
}

void *osal_memset(void *dst, int val, int count)
{
	return memset(dst, val, count);
}

struct timer_list *malloc_mt_timer(void)
{
	struct mt_timer_struct *mt_timer;

	return kzalloc(sizeof(*mt_timer), GFP_KERNEL);
}

void set_mt_timer_data(struct timer_list *timer, void *data)
{
	struct mt_timer_struct *mt_timer = (struct mt_timer_struct *)timer;

	if (mt_timer)
		mt_timer->data = data;
}

void *get_mt_timer_data(struct timer_list *timer)
{
	struct mt_timer_struct *mt_timer = (struct mt_timer_struct *)timer;

	if (!mt_timer)
		return NULL;

	return mt_timer->data;
}

struct mt_chip *to_chip(struct drm_device *drm)
{
	struct mtgpu_drm_private *drm_private = drm->dev_private;

	if (!drm_private)
		return NULL;

	return drm_private->chip;
}

struct mtgpu_gem_object *alloc_mtgpu_obj(void)
{
	struct mtgpu_gem_object *mtgpu_obj;

	mtgpu_obj = kzalloc(sizeof(struct mtgpu_gem_object), GFP_KERNEL);
	if (!mtgpu_obj)
		return NULL;

	return mtgpu_obj;
}

void vpu_set_drm_gem_object_funcs(struct drm_gem_object *obj)
{
	if (!obj)
		return;

#if !defined(OS_STRUCT_DRM_DRIVER_HAS_GEM_VM_OPS)
	obj->funcs = &mtgpu_gem_object_funcs;
#endif
}

struct mtgpu_gem_object *get_mtgpu_obj(struct drm_gem_object *obj)
{
	if (!obj)
		return NULL;

	return to_mtgpu_obj(obj);
}

struct drm_gem_object *get_mtgpu_base_obj(struct mtgpu_gem_object *mtgpu_obj)
{
	if (!mtgpu_obj)
		return NULL;

	return &mtgpu_obj->base;
}

size_t get_mtgpu_obj_size(struct mtgpu_gem_object *mtgpu_obj)
{
	return mtgpu_obj->base.size;
}

u64 get_mtgpu_obj_addr(struct mtgpu_gem_object *mtgpu_obj)
{
	return mtgpu_obj->dev_addr;
}

struct mt_fbc *get_mtgpu_obj_fbc_info(struct mtgpu_gem_object *mtgpu_obj)
{
	return (struct mt_fbc *)mtgpu_obj->private_data;
}

void *get_mtgpu_obj_handle(struct mtgpu_gem_object *mtgpu_obj)
{
	return mtgpu_obj->handle;
}

struct sg_table *get_mtgpu_obj_sgt(struct mtgpu_gem_object *mtgpu_obj)
{
	return mtgpu_obj->sgt;
}

size_t get_mt_node_size(struct mt_node *node)
{
	return node->obj->size;
}

u64 get_mt_node_addr(struct mt_node *node)
{
	return node->dev_addr;
}

void set_mtgem_obj_addr(struct mtgpu_gem_object *mtgem_obj, u64 base, u64 dev_addr)
{
	mtgem_obj->dev_addr = dev_addr;
	mtgem_obj->cpu_addr = base + dev_addr;
}

void set_mtgem_obj_type(struct mtgpu_gem_object *mtgem_obj, u32 offset)
{
	mtgem_obj->type = MTGEM_TYPE_VPU_START + offset;
}

u32 get_mtgem_obj_pool_id(struct mtgpu_gem_object *mtgem_obj)
{
	if (mtgem_obj->type > MTGEM_TYPE_VPU_START)
		return mtgem_obj->type - MTGEM_TYPE_VPU_START;
	else
		return 0;
}

struct mt_file *os_get_drm_file_private_data(struct drm_file *file)
{
	struct mtgpu_drm_file *drv_priv = file->driver_priv;

	return drv_priv->vpu_priv;
}

void os_set_drm_file_private_data(struct drm_file *file, struct mt_file *priv)
{
	struct mtgpu_drm_file *drv_priv = file->driver_priv;

	if (!drv_priv)
		return;

	drv_priv->vpu_priv = priv;
}

int vpu_vram_alloc_pool(struct drm_device *drm, int group_id, size_t size, struct mtgpu_gem_object *mtgpu_obj)
{
	return mtgpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);
}

size_t vpu_vram_alloc(struct drm_device *drm, int group_id, u32 pool_id, u32 width, u32 height, u32 bpp,
					u32 mem_type, struct mtgpu_gem_object *mtgpu_obj)
{
	struct mt_chip *chip = to_chip(drm);
	size_t size = 0;
	int err = 0;
	u32 pitch = 0;
	struct drm_gem_object *obj;

	if (mem_type == FB_COMPRESSED) {
		struct mt_fbc *fbc_info = (struct mt_fbc *)kzalloc(sizeof(struct mt_fbc), GFP_KERNEL);
		mtgpu_obj->private_data = (u64)fbc_info;
		if (mtgpu_obj->private_data == 0)
			return -1;
		get_fbc_info(bpp, width, height, &fbc_info->fbc_type, fbc_info->header_size, fbc_info->data_size);
		size = fbc_info->header_size[0] + fbc_info->header_size[1] + fbc_info->data_size[0] + fbc_info->data_size[1];
		size = ALIGN(size, gpu_page_size);
	}
	else {
		pitch = width * (ALIGN(bpp, 8) >> 3);
		size = ALIGN(pitch * height, gpu_page_size);
	}
	obj = get_mtgpu_base_obj(mtgpu_obj);
	drm_gem_private_object_init(drm, obj, size);

	if (VPU_IS_POOL_ID_VALID(pool_id) && vpu_fixed_128M_mem(mem_type)) {
		err = vpu_alloc_mem_pool(chip, pool_id, size, &mtgpu_obj->dev_addr);
		set_mtgem_obj_type(mtgpu_obj, pool_id);
		set_mtgem_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	} else
		err = mtgpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);

	if (err) {
		if (mtgpu_obj->private_data) {
			kfree((void *)mtgpu_obj->private_data);
			mtgpu_obj->private_data = 0;
		}
		size = 0;
	}
	return size;
}

void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj)
{
	if (mtgpu_obj) {
		if (mtgpu_obj->private_data)
			kfree((void *)mtgpu_obj->private_data);

		if (mtgpu_obj->handle)
			mtgpu_vram_free(mtgpu_obj->handle);
			
		kfree(mtgpu_obj);
	}
}

int vpu_get_drm_dev_id(struct drm_device *drm)
{
	struct mt_chip *chip = to_chip(drm);
	int i;

	if (!chip)
		return -1;

	for (i = 0; i < chip->drm_dev_cnt; i++)
		if (drm == chip->drm_dev[i])
			break;

	if (i == chip->drm_dev_cnt)
		return -1;

	return i;
}

int vpu_get_drm_group_id(struct drm_device *drm)
{
	struct mt_chip *chip = to_chip(drm);
	int i;

	if (!chip)
		return -1;

	for (i = 0; i < chip->drm_dev_cnt; i++)
		if (drm == chip->drm_dev[i])
			break;

	if (i == chip->drm_dev_cnt)
		return -1;

	return chip->drm_video_group_idx[i];
}

bool vpu_drm_core_valid(struct mt_chip *chip, struct drm_device *drm, u32 core_idx)
{
	struct mt_core *core = &chip->core[core_idx];

	if (core->serve_all || vpu_get_drm_group_id(drm) == vpu_get_drm_group_id(core->drm_dev))
		return true;

	return false;
}

void *vpu_get_pvr_node(struct drm_device *drm)
{
	struct mtgpu_drm_private *drm_private = drm->dev_private;

	return drm_private->pvr_private.dev_node;
}

void vpu_get_drm_mode_args(struct drm_mode_create_dumb *args, u32 *height, u32 *width, u32 *bpp, u32 *flags)
{
	*height = args->height;
	*width = args->width;
	*bpp = args->bpp;
	*flags = args->flags;
}

void vpu_set_drm_mode_args(struct drm_mode_create_dumb *args, u32 handle, u32 pitch, u32 size)
{
	args->handle = handle;
	args->pitch = pitch;
	args->size = size;
}

int mtvpu_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	struct mtgpu_gem_object *gpu_obj = to_mtgpu_obj(obj);
	u64 pfn = gpu_obj->cpu_addr >> PAGE_SHIFT;
	u64 off;

	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_flags |= VM_MIXEDMAP;

	vma->vm_page_prot = pgprot_noncached(vm_get_page_prot(vma->vm_flags));

	for (off = vma->vm_start; off < vma->vm_end; off += PAGE_SIZE) {
		vmf_insert_pfn(vma, off, pfn);
		pfn ++;
	}

	return 0;
}
