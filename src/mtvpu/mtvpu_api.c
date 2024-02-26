/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <drm/drm_device.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#endif

#include "pvrsrv.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drm_gem.h"
#include "mtgpu_drm_internal.h"
#include "os-interface-drm.h"

#include "mtvpu_drv.h"
#include "mtvpu_api.h"
#include "mtvpu_pool.h"
#include "misc.h"

bool is_guest_cmds = false;

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

void set_mtgpu_obj_addr(struct mtgpu_gem_object *mtgpu_obj, u64 base, u64 dev_addr)
{
	mtgpu_obj->dev_addr = dev_addr;
	mtgpu_obj->cpu_addr = base + dev_addr;
}

void set_mtgpu_obj_type(struct mtgpu_gem_object *mtgpu_obj, u32 group_id, u32 pool_id)
{
	u32 offset = (group_id << 8) + pool_id;
	mtgpu_obj->type = MTGEM_TYPE_VPU_START + offset;
}

int get_mtgpu_obj_type(struct mtgpu_gem_object *mtgpu_obj, u32 *group_id, u32 *pool_id)
{
	int offset = mtgpu_obj->type - MTGEM_TYPE_VPU_START;

	if (offset < 0)
		return -EINVAL;

	if (group_id)
		*group_id = (offset >> 8) & 0xFF;
	if (pool_id)
		*pool_id = offset & 0xFF;

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

/* only for ioctl */
int vpu_vram_alloc(struct drm_device *drm, u32 group_id, u32 pool_id, u32 type, u64 size, struct mtgpu_gem_object *mtgpu_obj)
{
	struct mt_chip *chip = to_chip(drm);
	int err;

	if (vpu_fixed_mem_qy2(chip, type)) {
		err = vpu_mem_pool_alloc(chip, pool_id, size, &mtgpu_obj->dev_addr);

		set_mtgpu_obj_type(mtgpu_obj, group_id, pool_id);
		set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	} else {
		err = mtgpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);

		set_mtgpu_obj_type(mtgpu_obj, group_id, 0);
		set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	}

	return err;
}

void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj)
{
	if (mtgpu_obj) {
		if (mtgpu_obj->private_data)
			kfree((void *)mtgpu_obj->private_data);

		if (mtgpu_obj->handle)
			mtgpu_vram_free(mtgpu_obj->handle);

		if (mtgpu_obj->obj) {
			os_drm_gem_object_release(mtgpu_obj->obj);
			kfree(mtgpu_obj->obj);
		}

		kfree(mtgpu_obj);
	}
}

int vpu_get_drm_id(struct drm_device *drm)
{
	struct mt_chip *chip = to_chip(drm);
	int i;

	if (!chip)
		return -1;

	for (i = 0; i < chip->mpc_drm_cnt; i++)
		if (drm == chip->drms[i])
			break;

	if (i == chip->mpc_drm_cnt)
		return -1;

	return i;
}

int vpu_get_group_id(struct drm_device *drm)
{
	struct mt_chip *chip = to_chip(drm);
	int i;

	if (!chip)
		return -1;

	for (i = 0; i < chip->mpc_drm_cnt; i++)
		if (drm == chip->drms[i])
			break;

	if (i == chip->mpc_drm_cnt)
		return -1;

	return chip->mpc_group_ids[i];
}

bool vpu_drm_core_valid(struct mt_chip *chip, struct drm_device *drm, u32 core_idx)
{
	struct mt_core *core = &chip->core[core_idx];

	if (core->serve_all || vpu_get_group_id(drm) == vpu_get_group_id(core->drm))
		return true;

	return false;
}

void *vpu_get_pvr_node(struct drm_device *drm)
{
	struct mtgpu_drm_private *drm_private = drm->dev_private;

	return drm_private->pvr_private.dev_node;
}

static void copy_phys_addr(u64 dst, u64 src, u64 size)
{
	void *datnew, *datold;

	datnew = memremap(dst, size, MEMREMAP_WC);
	datold = memremap(src, size, MEMREMAP_WC);
	if (datnew && datold)
		memcpy(datnew, datold, size);

	if (datnew)
		memunmap(datnew);
	if (datold)
		memunmap(datold);
}

int vpu_gem_modify(struct drm_device *drm, struct mtgpu_gem_object *mtgpu_obj, u32 group_id, u32 inc_size, u32 copy)
{
	struct mt_chip *chip = to_chip(drm);
	struct mtgpu_gem_object *mtgpu_new = alloc_mtgpu_obj();
	struct mtvpu_gem_priv *priv = (struct mtvpu_gem_priv *)mtgpu_obj->private_data;
	struct vm_area_struct *vma;
	u64 pfn;
	int err;

	if (!mtgpu_new)
		return -ENOMEM;

	err = mtgpu_vram_alloc(drm, group_id, mtgpu_obj->obj->size + inc_size, &mtgpu_new->dev_addr, &mtgpu_new->handle);
	if (err) {
		kfree(mtgpu_new);
		return -ENOMEM;
	}

	set_mtgpu_obj_type(mtgpu_new, group_id, 0);
	set_mtgpu_obj_addr(mtgpu_new, chip->bar_base, mtgpu_new->dev_addr);

	if (copy)
		copy_phys_addr(mtgpu_new->cpu_addr, mtgpu_obj->cpu_addr, mtgpu_obj->obj->size);

	mtgpu_vram_free(mtgpu_obj->handle);

	/* obj copy */
	mtgpu_obj->handle = mtgpu_new->handle;
	mtgpu_obj->dev_addr = mtgpu_new->dev_addr;
	mtgpu_obj->cpu_addr = mtgpu_new->cpu_addr;
	mtgpu_obj->type = mtgpu_new->type;

	mtgpu_obj->obj->size += inc_size;

	if (!priv || !priv->vma)
		goto exit;

	vma = priv->vma;
	pfn = mtgpu_new->cpu_addr >> PAGE_SHIFT;

	zap_vma_ptes(vma, vma->vm_start, vma->vm_end - vma->vm_start);

	vma->vm_end += inc_size;
	remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end - vma->vm_start, vma->vm_page_prot);

exit:
	kfree(mtgpu_new);
	return 0;
}

int mtvpu_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);
	struct mtvpu_gem_priv *priv = (struct mtvpu_gem_priv *)mtgpu_obj->private_data;
	u64 pfn = mtgpu_obj->cpu_addr >> PAGE_SHIFT;

	if (priv)
		priv->vma = vma;

	vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(vma->vm_flags));

	return remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end - vma->vm_start, vma->vm_page_prot);
}

void mtvpu_gem_free_obj(struct drm_gem_object *obj)
{
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

	vpu_vram_free(mtgpu_obj);
}

int vpu_init_guest_capibility(struct mt_chip *chip)
{
	u32 width, height;
	u32 max_enc_num, max_dec_num;

	//todo: get limits from kmd
	width = 1920;
	height = 1080;
	max_enc_num = 1;
	max_dec_num = 1;

	chip->codec_limit.width = ALIGN(width, 128);
	chip->codec_limit.height = ALIGN(height, 128);
	chip->codec_limit.max_enc_num = max_enc_num;
	chip->codec_limit.max_dec_num = max_dec_num;

	return 0;
}
