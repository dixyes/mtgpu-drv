/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/slab.h>
#include <linux/device.h>

#include <drm/drm_file.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_device.h>
#include <drm/drm_ioctl.h>
#endif

#ifdef SUPPORT_ION
#include "ion/ion.h"
#endif

#include "mtgpu_mdev.h"
#include "mtgpu_drm_gem.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drv.h"

#include "osfunc.h"

#include "mtvpu_drv.h"
#include "mtvpu_gem.h"
#include "mtvpu_pool.h"
#include "vpuapifunc.h"
#include "mtvpu_pool.h"

#include "misc.h"

#ifdef SUPPORT_ION
u64 get_dev_addr_dma_buf(struct dma_buf *psDmaBuf)
{
	struct ion_buffer *buffer;
	struct sg_table *table;
	ion_phys_addr_t paddr;

	if (!psDmaBuf)
		return 0;
	buffer = psDmaBuf->priv;
	if (!buffer)
		return 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	table = buffer->priv_virt;
#else
	table = buffer->sg_table;
#endif
	paddr = sg_dma_address(table->sgl);
	return paddr;
}

int ion_free_node(struct mt_node *node)
{
	struct dma_buf *psDmaBuf;
	struct ion_buffer *ion_buf;

	if (!node)
		return -1;
	psDmaBuf = node->ion_buf;
	if (!psDmaBuf)
		return -1;
	ion_buf = psDmaBuf->priv;

	return ion_free(ion_buf);
}

struct mt_node *ion_malloc_internal(struct mt_chip *chip, int idx, int drm_id, u64 size)
{
	struct mt_node *node;
	struct dma_buf *ion_buf;

	size = ALIGN(size, gpu_page_size);
	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;

	node->obj = kzalloc(sizeof(*node->obj), GFP_KERNEL);
	if (!node->obj) {
		kfree(node);
		return NULL;
	}

	ion_buf = ion_alloc(chip->ion_dev[drm_id], size, 1 << chip->core[idx].heap_id, 0);
	if (!ion_buf) {
		kfree(node->obj);
		kfree(node);
		pr_err("ion allocate size %lld failed\n", size);
		return NULL;
	}
	node->ion_buf = ion_buf;
	node->dev_addr = get_dev_addr_dma_buf(ion_buf);
	node->obj->size = ion_buf->size;
	pr_info("core %d ion malloc internal 0x%llx\n", idx, node->dev_addr);
	return node;
}
#endif

struct mt_node *gem_malloc_internal(struct mt_chip *chip, int idx, u32 pool_id, struct drm_device *drm, u64 size, u32 mem_type)
{
	struct mt_core *core = &chip->core[idx];
	struct mt_node *node;
	u64 dev_addr;
	size_t obj_size;
	int ret = 0;

	size = ALIGN(size, gpu_page_size);
	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;

	node->obj = kzalloc(sizeof(struct drm_gem_object), GFP_KERNEL);
	if (!node->obj) {
		kfree(node);
		return NULL;
	}

	if ((VPU_IS_POOL_ID_VALID(pool_id) && vpu_fixed_128M_mem(mem_type)) ||
	    VDI_POOL_ID_VALID(pool_id)) {
		node->pool_id = pool_id;
		ret = vpu_alloc_mem_pool(chip, pool_id > 0xFF000000 ? (pool_id - 0xFF000000) :
					 pool_id, size, &node->dev_addr);
	} else
		ret = mtgpu_vram_alloc(drm, core->mem_group_id, size,
					  &node->dev_addr, &node->handle);

	drm_gem_private_object_init(drm, node->obj, size);

	if (ret) {
		pr_err("error, mtgpu_vram_alloc\n");
		goto unref;
	}

	dev_addr = get_mt_node_addr(node);
	obj_size = get_mt_node_size(node);

	return node;

unref:
	drm_gem_object_release(node->obj);
	kfree(node->obj);
	kfree(node);
	return NULL;
}

/* check work buffer, task buffer cross with code buffer.
 * refer: https://confluence.mthreads.com/display/VD/VPU+buf+require#
 */
struct mt_node *gem_malloc(struct mt_chip *chip, int idx, int instIdx, u64 size,
			u32 mem_type, struct list_head *mm_head)
{
	struct mt_node *node = NULL;
	int drm_id;
	struct drm_device *drm_dev;
#ifndef SUPPORT_ION
	u32 pool_id = chip->core[idx].pool_ids[instIdx];
#endif

	drm_id = chip->core[idx].drm_ids[instIdx];
	if (drm_id < 0 || drm_id > chip->drm_dev_cnt - 1)
		return NULL;

	drm_dev = chip->drm_dev[drm_id];
	if (!drm_dev)
		return NULL;

#ifdef SUPPORT_ION
	node = ion_malloc_internal(chip, idx, drm_id, size);
#else
	node = gem_malloc_internal(chip, idx, pool_id, drm_dev, size, mem_type);
#endif
	if (!node)
		return NULL;

	/*
	 * Be careful. The VRAM for VPU can't use the address during
	 * 0 - (2M(code buffer) + 16K(log buffer)) and 0xfffef000UL - 4G (last 68K) of each 4G.
	 * VPU cpu's bit width is 32 bit. Video driver will remap the binary buffer to above
	 * address. If driver gives the buffers to VPU, VCPU will broke its binary buffer or other
	 * critical buffers. This will cause VPU hang.
	 */
	if ((node->dev_addr & 0xffffffffUL) < (WAVE5_MAX_CODE_BUF_SIZE + FW_LOG_BUFFER_SIZE) ||
	    ((node->dev_addr + size) & 0xffffffffUL) >= W_VCPU_SPM_ADDR) {
		pr_err("Illegal VRAM address(%llx) for VPU\n", node->dev_addr);

		if (node->handle)
			mtgpu_vram_free(node->handle);
			
		if (node->obj) {
			drm_gem_object_release(node->obj);
			kfree(node->obj);
		}
		kfree(node);

		return NULL;
	}

	mutex_lock(chip->mm_lock);
	list_add(&node->list, mm_head);
	mutex_unlock(chip->mm_lock);

	return node;
}

/* for ANDROID, gem malloc cannot use ION during insmod */
void gem_free_internal(struct mt_chip *chip, struct mt_node *node)
{
	if (node->vir_addr)
		iounmap(node->vir_addr);

	if (node->bak_addr)
		vfree(node->bak_addr);

	mtgpu_vram_free(node->handle);
	drm_gem_object_release(node->obj);
}

void gem_free_node(struct mt_chip *chip, struct mt_node *node)
{

	if (node->vir_addr)
		iounmap(node->vir_addr);

	if (node->bak_addr)
		vfree(node->bak_addr);

#ifdef SUPPORT_ION
	ion_free_node(node);
#else
	/* VDI case will not destroy the pool here */
	if VPU_IS_POOL_ID_VALID(node->pool_id)
		vpu_destroy_mem_pool(chip, node->pool_id);
	else if (!VDI_POOL_ID_VALID(node->pool_id))
		mtgpu_vram_free(node->handle);

	drm_gem_object_release(node->obj);
#endif
}

void gem_free_all(struct mt_chip *chip, struct list_head *mm_head)
{
	struct mt_node *node, *next;

	mutex_lock(chip->mm_lock);

	list_for_each_entry_safe(node, next, mm_head, list) {
		gem_free_node(chip, node);

		list_del(&node->list);

		kfree(node->obj);
		kfree(node);
	}

	mutex_unlock(chip->mm_lock);
}

void gem_free(struct mt_chip *chip, u64 dev_addr, struct list_head *mm_head)
{
	struct mt_node *node, *next;

	mutex_lock(chip->mm_lock);

	list_for_each_entry_safe(node, next, mm_head, list) {
		if (node->dev_addr == dev_addr) {
			gem_free_node(chip, node);

			list_del(&node->list);

			kfree(node->obj);
			kfree(node);
			break;
		}
	}

	mutex_unlock(chip->mm_lock);
}

void gem_clear(struct mt_core *core, u64 dev_addr)
{
	struct mt_node *node;
	int inst;

	for (inst = 0; inst < INST_MAX_SIZE; inst ++) {
		list_for_each_entry(node, &core->mm_head[inst], list) {
			if (node->dev_addr == dev_addr) {
				memset_io(node->vir_addr, 0, node->obj->size);
				return;
			}
		}
	}
}
