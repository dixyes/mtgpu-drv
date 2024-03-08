/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/slab.h>
#include <linux/device.h>

#include <linux/io.h>
#include <drm/drm_file.h>
#include <drm/drm_gem.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_device.h>
#include <drm/drm_ioctl.h>
#endif

#ifdef SUPPORT_ION
#include "ion/ion.h"
#endif

#ifndef SOC_MODE
#include "mtgpu_drv.h"
#include "mtgpu_drm_gem.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drm_internal.h"
#endif

#include "mtvpu_drv.h"
#include "mtvpu_gem.h"
#include "mtvpu_pool.h"
#include "vpuapifunc.h"
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

struct mt_node *ion_malloc_node(struct mt_chip *chip, int idx, int drm_id, u64 size)
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

	ion_buf = ion_alloc(chip->ions[drm_id], size, 1 << chip->core[idx].heap_id, 0);
	if (!ion_buf) {
		kfree(node->obj);
		kfree(node);
		pr_err("mtvpu ion allocate size %lld failed\n", size);
		return NULL;
	}

	node->ion_buf = ion_buf;
	node->dev_addr = get_dev_addr_dma_buf(ion_buf);
	node->obj->size = ion_buf->size;
	pr_info("mtvpu core %d ion malloc internal 0x%llx\n", idx, node->dev_addr);
	return node;
}
#endif

struct mt_node *gem_malloc_node(struct mt_chip *chip, int idx, u32 pool_id, struct drm_device *drm, u64 size, u32 type)
{
	struct mt_core *core = &chip->core[idx];
	struct mt_node *node;
	int ret = 0, host = 0;

	if (pool_id > 0xFF000000) {
		pool_id -= 0xFF000000;
		host = 1;
	}

	size = ALIGN(size, gpu_page_size);

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;

	node->obj = kzalloc(sizeof(struct drm_gem_object), GFP_KERNEL);
	if (!node->obj) {
		kfree(node);
		return NULL;
	}

	if (host || vpu_fixed_mem_qy2(chip, type))
		ret = vpu_mem_pool_alloc(chip, pool_id, size, &node->dev_addr);
	else
#ifdef SOC_MODE
		ret = mtvpu_vram_alloc(drm, core->mem_group_id, size, &node->dev_addr, &node->handle);
		pr_info("gem malloc node size %llx, addr %llx\n", size, node->dev_addr);
#else
		ret = mtgpu_vram_alloc(drm, core->mem_group_id, size, &node->dev_addr, &node->handle);
#endif

	drm_gem_private_object_init(drm, node->obj, size);

	if (ret) {
		pr_err("mtvpu Error mtgpu_vram_alloc\n");
		goto unref;
	}

	node->pool_id = pool_id;
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
struct mt_node *gem_malloc(struct mt_chip *chip, int idx, int instIdx, u64 size, u32 type, struct list_head *mm_head)
{
	struct mt_node *node = NULL;
	struct drm_device *drm;
	int drm_id;
#ifndef SUPPORT_ION
	u32 pool_id = chip->core[idx].pool_ids[instIdx];
#endif

	drm_id = chip->core[idx].drm_ids[instIdx];
	if (drm_id < 0 || drm_id > chip->mpc_drm_cnt - 1)
		return NULL;

	drm = chip->drms[drm_id];
	if (!drm)
		return NULL;

#ifdef SUPPORT_ION
	node = ion_malloc_node(chip, idx, drm_id, size);
#else
	node = gem_malloc_node(chip, idx, pool_id, drm, size, type);
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
	    ((node->dev_addr + size) & 0xffffffffUL) > W_VCPU_SPM_ADDR) {
		pr_err("mtvpu Error address(%llx to %llx size:%llx)\n", node->dev_addr,
		       node->dev_addr + size, size);

		if (node->handle)
#ifdef SOC_MODE
			mtvpu_vram_free(node->obj, node->handle);
#else
			mtgpu_vram_free(node->handle);
#endif
			
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

void gem_free_node(struct mt_chip *chip, struct mt_node *node)
{
	if (node->vir_addr)
#ifdef SOC_MODE
		memunmap(node->vir_addr);
#else
		iounmap(node->vir_addr);
#endif

	if (node->bak_addr)
		vfree(node->bak_addr);

#ifdef SUPPORT_ION
	ion_free_node(node);
#else
	/* VDI case will not destroy the pool here */
	if (node->pool_id > 0)
		vpu_mem_pool_free(chip, node->pool_id);
	else
#ifdef SOC_MODE
		mtvpu_vram_free(node->obj, node->handle);
#else
		mtgpu_vram_free(node->handle);
#endif

	drm_gem_object_release(node->obj);
#endif
}

void gem_free_all(struct mt_chip *chip, struct list_head *mm_head)
{
	struct mt_node *node = NULL, *next = NULL;

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
	struct mt_node *node = NULL, *next = NULL;

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
	struct mt_node *node = NULL;
	int inst;

	for (inst = 0; inst < INST_MAX_SIZE; inst ++) {
		list_for_each_entry(node, &core->mm_head[inst], list) {
			if (node->dev_addr == dev_addr) {
				memset_io(node->vir_addr, 0, node->obj->size);
#ifdef SOC_MODE
				os_dcache_clean(node->vir_addr, node->obj->size);
#endif
				return;
			}
		}
	}
}
