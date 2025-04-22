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

#include "mtgpu_drv.h"
#include "mtgpu_drm_gem.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drm_internal.h"
#include "mtgpu_module_param.h"
#include "os-interface-drm.h"

#include "mtvpu_drv.h"
#include "mtvpu_gem.h"
#include "mtvpu_pool.h"
#include "mtvpu_smmu.h"
#include "mtvpu_mem.h"
#include "vpuapifunc.h"
#include "misc.h"
#include "helper.h"

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

#if 0
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
		vpu_err("ion allocate size %lld failed\n", size);
		return NULL;
	}

	node->ion_buf = ion_buf;
	node->dev_phys_addr = get_dev_addr_dma_buf(ion_buf);
	node->obj->size = ion_buf->size;
	vpu_info("core %d ion malloc internal 0x%llx\n", idx, node->dev_phys_addr);
	return node;
}
#endif
#endif

struct mt_node *gem_malloc_node(struct mt_chip *chip, int idx,
				struct drm_device *drm, u64 size, u32 type)
{
	struct mt_core *core = &chip->core[idx];
	struct mt_node *node;
	u32 max_page_size;
	int ret = 0;

	/* only for fw buffer; fix this if needed */
	if (type != FW_BUFFER)
		return NULL;

	max_page_size = gpu_page_size < OS_VAL(PAGE_SIZE) ? OS_VAL(PAGE_SIZE) : gpu_page_size;
	size = ALIGN(size, max_page_size);

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;

	node->obj = kzalloc(sizeof(struct drm_gem_object), GFP_KERNEL);
	if (!node->obj) {
		kfree(node);
		return NULL;
	}

	if (chip->driver_mode == MTGPU_DRIVER_MODE_HOST) {
		ret = mtgpu_vram_alloc(drm, MTGPU_SEGMENT_ID_VPU_HOST, size,
					&node->dev_phys_addr, &node->handle);
		if (ret) {
			vpu_err("vdi alloc vcpu failed, core:%d!\n", idx);
			goto nomem;
		}
	/*
	} else if (vpu_fixed_mem_qy2(chip, type)) {
		ret = vpu_mem_pool_alloc(chip, pool_id, size, &node->dev_phys_addr);
	*/
	} else {
		if (chip->soc_mode)
			ret = mtvpu_vram_alloc(drm, core->mem_group_id, size, &node->dev_phys_addr, &node->handle);
		else
			ret = mtgpu_vram_alloc(drm, core->mem_group_id, size, &node->dev_phys_addr, &node->handle);
	}

	drm_gem_private_object_init(drm, node->obj, size);
	VLOG(MTDEBUG, "vpu_gem_alloc_node: type:%d, size:%llx, mem_group_id:%d, dev_phys_addr:%llx,"
	     "handle:%llx\n", type, size, core->mem_group_id, node->dev_phys_addr, node->handle);
	if (ret) {
		vpu_err("Error mtgpu_vram_alloc\n");
		goto unref;
	}

	node->size = size;
	return node;

unref:
	drm_gem_object_release(node->obj);
nomem:
	kfree(node->obj);
	kfree(node);
	return NULL;
}

void gem_free_node(struct mt_chip *chip, struct mt_node *node)
{
	/* fw buffer only */

	/*
	if (node->pool_id > 0)
		vpu_mem_pool_free(chip, node->pool_id);
	*/
	if (chip->soc_mode && chip->io_domain)
		vpu_smmu_unmap(chip, node->dev_phys_addr);
	mtgpu_vram_free(node->handle);

	drm_gem_object_release(node->obj);
}
