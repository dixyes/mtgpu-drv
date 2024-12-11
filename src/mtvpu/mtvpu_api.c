/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <drm/drm_device.h>
#include <drm/drm_gem.h>
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
#include "mtgpu_module_param.h"
#include "os-interface-drm.h"
#include "mtvpu_drv.h"
#include "mtvpu_api.h"
#include "mtvpu_pool.h"
#include "mtvpu_smmu.h"
#include "mtvpu_mem.h"
#include "misc.h"

bool is_guest_cmds = false;

struct file_operations vinfo_fops = {
	.owner = THIS_MODULE,
	.read = vpu_info_read,
	.write = vpu_info_write,
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

unsigned long os_memremap_wb(void)
{
	return MEMREMAP_WB;
}

void *os_memremap(resource_size_t offset, size_t size, unsigned long flags)
{
	return memremap(offset, size, flags);
}

void os_memunmap(void *addr)
{
	memunmap(addr);
}

int mtvpu_vram_alloc(struct drm_device *drm, u32 group_id, size_t size,
		     dma_addr_t *dev_addr, void **handle)
{
	int err;
	struct mt_chip *chip = to_chip(drm);
	struct sg_table *sgt;

	if (!chip->io_domain) {
		if (enable_reserved_memory) {
			err = mtgpu_vram_alloc(drm, group_id - 1, size, dev_addr, handle);
			if (err) {
				OS_DRM_ERROR("%s(): allocate dumb buffer size %x, group %d failed\n", __func__, size, group_id);
				return -OS_VAL(ENOMEM);
			}
		} else {
			OS_DRM_ERROR("%s(): reserved memory must be used if smmu not enabled! \n", __func__);
			return -OS_VAL(ENOMEM);
		}
	} else {
		err = mtgpu_system_alloc(drm, size, handle);
		if (err) {
			OS_DRM_ERROR("%s(): mtgpu_system_alloc() failed\n", __func__);
			return -OS_VAL(ENOMEM);
		}
		sgt = vpu_gem_map_internal(*handle, size);
		/* dev addr will use iova addr if smmu enabled */
		err = vpu_smmu_map_sg(chip, sgt, size, group_id, dev_addr);
		vpu_gem_unmap_internal(sgt);
		if (err) {
			vpu_err("vpu smmu map failed!");
			return err;
		}
	}
	//vpu_info("mtvpu alloc group %d, dev addr %llx\n", group_id, *dev_addr);
	return 0;
}

void *vpu_gem_vmap_internal(void *handle, u64 size, u64 *private_data)
{
	void *kaddr;
	int err;

	if (!handle) {
		vpu_err("handle is NULL\n");
		return NULL;
	}
	err = mtgpu_vram_vmap(handle, size, private_data, &kaddr);
	if (err) {
		vpu_err("failed to acquire cpu kernel address for handle 0x%llx\n", (u64)handle);
		return NULL;
	}
	//vpu_info("map gem handle %llx to cpu kernel addr 0x%llx\n", (u64)handle, (u64)kaddr);
	return kaddr;
}

void vpu_gem_vunmap_internal(void *handle, u64 private_data)
{
	mtgpu_vram_vunmap(handle, private_data);
}

struct sg_table *vpu_gem_map_internal(void *handle, size_t size)
{
	struct sg_table *sgt;
	struct scatterlist *sgl;
	struct page *cpu_page;
	IMG_CPU_PHYADDR *cpu_pa;
	IMG_BOOL *valid;
	u32 num_pages;
	u64 sg_idx = 0;
	int ret;

	ret = os_sg_table_create(&sgt);
	if (ret)
		return NULL;

	num_pages = size / OS_VAL(PAGE_SIZE);
	valid = os_kvzalloc(sizeof(*valid) * num_pages);
	if (!valid)
		goto err_free_sgt;

	cpu_pa = os_kvzalloc(sizeof(*cpu_pa) * num_pages);
	if (!cpu_pa)
		goto err_free_valid;

	ret = PMR_CpuPhysAddr(handle, OS_VAL(PAGE_SHIFT), num_pages, 0, cpu_pa, valid);
	if (ret != PVRSRV_OK)
		goto err_free_pa;

	/* This gem object was exported to other drm driver,
	 * it must be a system backed object. */
	if (os_sg_alloc_table(sgt, num_pages)) {
		vpu_err("%s(): sgtable alloc multi sglist failed\n", __func__);
		goto err_free_pa;
	}
	os_for_each_sg(OS_SG_TABLE_MEMBER(sgt, sgl), sgl,
		       OS_SG_TABLE_MEMBER(sgt, nents), sg_idx) {
		cpu_page = os_phys_to_page(cpu_pa[sg_idx].uiAddr);
		os_set_sg_page(sgl, cpu_page, OS_VAL(PAGE_SIZE), 0);
		os_set_sg_dma_address(sgl, cpu_pa[sg_idx].uiAddr);
		os_set_sg_dma_len(sgl, OS_VAL(PAGE_SIZE));
	}
	os_kvfree(cpu_pa);
	os_kvfree(valid);

	return sgt;

err_free_pa:
	os_kvfree(cpu_pa);
err_free_valid:
	os_kvfree(valid);
err_free_sgt:
	os_sg_table_destroy(sgt);
	return NULL;
}

void vpu_gem_unmap_internal(struct sg_table *sgt)
{
	os_sg_free_table(sgt);
	os_kfree(sgt);
}

/* only for ioctl */
int vpu_vram_alloc(struct drm_device *drm, u32 group_id, u32 pool_id, u32 type, u64 size,
		   struct mtgpu_gem_object *mtgpu_obj)
{
	struct mt_chip *chip = to_chip(drm);
	struct mtvpu_gem_priv *priv = (struct mtvpu_gem_priv *)mtgpu_obj->private_data;
	struct mtvpu_mmu_ctx *mmu_ctx = priv->mmu_ctx;
	int err = 0;

	if (vpu_fixed_mem_qy2(chip, type)) {
		err = vpu_mem_pool_alloc(chip, mmu_ctx, pool_id, size, &mtgpu_obj->dev_addr);

		set_mtgpu_obj_type(mtgpu_obj, group_id, pool_id);
		set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	} else {
		if (mmu_ctx && mmu_ctx->mmu_enable) {
			err = mtvpu_vm_pmr_create_and_map(OSGetDeviceNodeFromDrm(drm),
						      mmu_ctx->vpu_ctx,
						      size,
						      PVRSRV_MEMALLOCFLAG_GPU_WRITEABLE |
						      PVRSRV_MEMALLOCFLAG_GPU_READABLE |
						      PVRSRV_MEMALLOCFLAG_CPU_WRITEABLE |
						      PVRSRV_MEMALLOCFLAG_CPU_READABLE,
						      &mtgpu_obj->handle,
						      &mtgpu_obj->dev_addr,
						      &priv->dev_phy_addr,
						      (void **)&mtgpu_obj->cpu_addr);
			/* Set mtgpu_obj type to use memory management function provided by kmd */
			mtgpu_obj->type = 0;
		} else if (chip->soc_mode) {
			err = mtvpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);
		} else {
			err = mtgpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);
		}

		set_mtgpu_obj_type(mtgpu_obj, group_id, 0);
		set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	}

	return err;
}

void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj)
{
	struct mt_chip *chip;
	struct mtvpu_gem_priv *priv;

	if (mtgpu_obj) {
		chip = to_chip(mtgpu_obj->obj->dev);
		priv = (struct mtvpu_gem_priv *)mtgpu_obj->private_data;

		if (mtgpu_obj->handle) {
			if (priv && priv->mmu_ctx && priv->mmu_ctx->mmu_enable) {
				mtvpu_vm_pmr_unmap_and_destroy(priv->mmu_ctx->vpu_ctx, mtgpu_obj->handle, mtgpu_obj->dev_addr, (void *)mtgpu_obj->cpu_addr);
			} else if (chip->soc_mode) {
				if (chip->io_domain) {
					vpu_smmu_unmap(chip, mtgpu_obj->dev_addr);
					if (priv && priv->priv_data)
						vpu_gem_vunmap_internal(mtgpu_obj->handle, priv->priv_data);
				}
				mtgpu_vram_free(mtgpu_obj->handle);
			} else {
				mtgpu_vram_free(mtgpu_obj->handle);
			}
		}

		if (mtgpu_obj->obj) {
			os_drm_gem_object_release(mtgpu_obj->obj);
			kfree(mtgpu_obj->obj);
		}
		if (priv)
			kfree(priv);
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

static void copy_phys_addr(u64 dst, u64 src, u64 size, bool soc_mode)
{
	void *datnew, *datold;

	if (soc_mode) {
		datnew = (void *)dst;
		datold = (void *)src;
	} else {
		datnew = memremap(dst, size, MEMREMAP_WC);
		datold = memremap(src, size, MEMREMAP_WC);
	}
	if (datnew && datold)
		memcpy(datnew, datold, size);

	if (soc_mode)
		dcache_flush(datnew, size);

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
	struct mtvpu_mmu_ctx *mmu_ctx = priv->mmu_ctx;
	struct vm_area_struct *vma;
	u64 pfn;
	int err;

	if (!mtgpu_new)
		return -ENOMEM;

	if (mmu_ctx && mmu_ctx->mmu_enable) {
		return -EPERM;
	} else if (chip->soc_mode)
		err = mtvpu_vram_alloc(drm, group_id, mtgpu_obj->obj->size + inc_size, &mtgpu_new->dev_addr, &mtgpu_new->handle);
	else
		err = mtgpu_vram_alloc(drm, group_id, mtgpu_obj->obj->size + inc_size, &mtgpu_new->dev_addr, &mtgpu_new->handle);
	if (err) {
		kfree(mtgpu_new);
		return -ENOMEM;
	}

	set_mtgpu_obj_type(mtgpu_new, group_id, 0);
	set_mtgpu_obj_addr(mtgpu_new, chip->bar_base, mtgpu_new->dev_addr);

	if (copy)
		copy_phys_addr(mtgpu_new->cpu_addr, mtgpu_obj->cpu_addr, mtgpu_obj->obj->size, chip->soc_mode);

	if (chip->soc_mode && chip->io_domain)
		vpu_smmu_unmap(chip, mtgpu_obj->dev_addr);
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

void mtvpu_gem_free_obj(struct drm_gem_object *obj)
{
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

	vpu_vram_free(mtgpu_obj);
}

int mtvpu_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	struct mt_chip *chip = to_chip(obj->dev);
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);
	struct mtvpu_gem_priv *priv = (struct mtvpu_gem_priv *)mtgpu_obj->private_data;
	u64 pfn;

	if (!chip->soc_mode) {
		/* only do this in pcie mode for performance reason; remove this later */
		pfn = PHYS_PFN(mtgpu_obj->cpu_addr);
		if (priv)
			priv->vma = vma;
		return remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end - vma->vm_start, vma->vm_page_prot);
	}
	return -1;
}

int mtvpu_gem_dmabuf_map(struct sg_table *sgt, struct mtgpu_gem_object *mtgpu_obj)
{
	/* remove this later */
	return -1;
}

long os_vfs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return vfs_ioctl(file, cmd, arg);
}

#ifdef __aarch64__
void dcache_flush(void *addr, size_t len)
{
  char *base = (char *)addr;
  int offset;
  for (offset = 0; offset < len; offset += 64)
    asm volatile ("dc civac, %0" :: "r" (base + offset));
}
#else
void dcache_flush(void *addr, size_t len)
{
}
#endif
