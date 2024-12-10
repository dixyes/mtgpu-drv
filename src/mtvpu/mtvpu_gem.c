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

struct mt_node *gem_malloc_node(struct mt_chip *chip, int idx, int inst_idx, u32 pool_id,
				struct drm_device *drm, struct mtvpu_mmu_ctx *mmu_ctx, u64 size, u32 type)
{
	struct mt_core *core = &chip->core[idx];
	struct mt_node *node;
	int ret = 0;

	if (mmu_ctx && mmu_ctx->mmu_enable)
		size = ALIGN(size, mmu_ctx->page_size);
	else
		size = ALIGN(size, gpu_page_size);

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;

	node->obj = kzalloc(sizeof(struct drm_gem_object), GFP_KERNEL);
	if (!node->obj) {
		kfree(node);
		return NULL;
	}
	node->mmu_ctx = mmu_ctx;

	if ((UMD_ALLOC_BUFFER(type) || chip->driver_mode == MTGPU_DRIVER_MODE_HOST)
	    && (chip->conf.type != TYPE_PIHU1)) {
		/* for sudi, qy1 host, use the buffers allocated from guest */
		if (type == DEC_FBCC_TBL || type == DEC_FBCY_TBL) {
			struct umd_alloc_buffer_union *tbl = &core->fbc_tbl[inst_idx];

			if (tbl->start_pa && tbl->used_size + size <= tbl->total_size) {
				node->dev_phys_addr = tbl->start_pa + tbl->used_size;
				tbl->used_size += size;
				node->vram_belonger = UMD_ALLOC;
				node->iova_addr = node->dev_phys_addr;
				VLOG(INFO, "get umd alloc fbc_tbl buffer! core:%d, inst:%d,"
				     "addr:%llx, size:%llx, type: %d, free:%llx\n",
				     idx, inst_idx, node->dev_phys_addr, size, type,
				     tbl->total_size - tbl->used_size);
			} else {
				VLOG(ERR, "get umd alloc fbc_tbl buffer failed! core:%d, inst:%d,"
				     "need size:%llx, free size:%llx, type: %d\n",
				     idx, inst_idx, size, tbl->total_size - tbl->used_size, type);
				goto nomem;
			}
		} else if (type == DEC_FBC) {
			int i;

			/* find a free fbc buffer */
			for (i = 0; i < FBC_COUNT_MAX; i++) {
				if (!core->fbc_buffers[inst_idx][i].used &&
				    core->fbc_buffers[inst_idx][i].dev_addr &&
				    core->fbc_buffers[inst_idx][i].size >= size) {
					node->dev_phys_addr = core->fbc_buffers[inst_idx][i].dev_addr;
					core->fbc_buffers[inst_idx][i].used = 1;
					node->vram_belonger = UMD_ALLOC;
					node->iova_addr = node->dev_phys_addr;
					VLOG(INFO, "get umd alloc fbc buffer! core:%d, inst:%d,"
					     "fbc_idx:%d, addr:%llx, size:%llx, type:%d, free:%d\n",
					     idx, inst_idx, i, node->dev_phys_addr, size, type,
					     core->fbc_buffers[inst_idx][i].size - size);
					break;
				}
			}

			if (i >= FBC_COUNT_MAX) {
				VLOG(ERR, "get umd alloc fbc buffer failed! core:%d, inst:%d,"
				     "fbc_idx: %d, need size:%llx, umd alloc size:%llx, type:%d\n",
				     idx, inst_idx, i, size, type,
				     core->fbc_buffers[inst_idx][0].size);
				goto nomem;
			}
		} else if (type == DEC_MV && chip->driver_mode != MTGPU_DRIVER_MODE_HOST) {
			struct umd_alloc_buffer_union *mv = &core->mv_buffers[inst_idx];
			if (mv->start_pa && mv->used_size + size <= mv->total_size) {
				node->dev_phys_addr = mv->start_pa + mv->used_size;
				mv->used_size += size;
				node->vram_belonger = UMD_ALLOC;
				node->iova_addr = node->dev_phys_addr;
				VLOG(INFO, "get umd alloc mv buffer! core:%d, inst:%d,"
				     "addr:%llx, size:%llx, type: %d, free:%llx\n",
				     idx, inst_idx, node->dev_phys_addr, size, type,
				     mv->total_size - mv->used_size);

			} else {
				VLOG(ERR, "get umd alloc mv buffer failed! core:%d, inst:%d,"
				     "need size:%llx, free size:%llx, type: %d\n",
				     idx, inst_idx, size, mv->total_size - mv->used_size, type);
				goto nomem;
			}
		} else if ((type == DEC_WORK || type == ENC_WORK) &&
			   chip->driver_mode != MTGPU_DRIVER_MODE_HOST) {
			if(!core->work_buffers[inst_idx].used &&
			   core->work_buffers[inst_idx].dev_addr &&
			   core->work_buffers[inst_idx].size >= size) {
				node->dev_phys_addr = core->work_buffers[inst_idx].dev_addr;
				core->work_buffers[inst_idx].used = 1;
				node->vram_belonger = UMD_ALLOC;
				node->iova_addr = node->dev_phys_addr;
				VLOG(INFO, "get umd alloc work buffer! core:%d, inst_idx:%d,"
				     "addr:%llx, size:%llx, type: %d, free:%llx\n",
				     idx, inst_idx, node->dev_phys_addr, size, type,
				     core->work_buffers[inst_idx].size - size);
			} else {
				VLOG(ERR, "get umd alloc work buffer failed! core:%d, inst:%d,"
				     "need size:%llx, umd alloc size:%llx, type: %d\n",
				     idx, inst_idx, size, core->work_buffers[0].size, type);
				goto nomem;
			}
		} else if (type == DEC_TASK && chip->driver_mode != MTGPU_DRIVER_MODE_HOST) {
			if(!core->task_buffers[inst_idx].used &&
			   core->task_buffers[inst_idx].dev_addr &&
			   core->task_buffers[inst_idx].size >= size) {
				node->dev_phys_addr = core->task_buffers[inst_idx].dev_addr;
				core->task_buffers[inst_idx].used = 1;
				node->vram_belonger = UMD_ALLOC;
				node->iova_addr = node->dev_phys_addr;
				VLOG(INFO, "get umd alloc task buffer! core:%d, inst:%d, addr:%llx,"
				     "size:%llx, type:%d, free:%d\n", idx, inst_idx, node->dev_phys_addr,
				      size, type, core->task_buffers[inst_idx].size - size);
			} else {
				VLOG(ERR, "get umd alloc task buffer failed! core:%d, inst:%d,"
				     "need size:%llx, umd alloc size:%llx, type: %d\n",
				     idx, inst_idx, size, core->task_buffers[inst_idx].size, type);
				goto nomem;
			}
		} else if (type == DEC_ETC && chip->driver_mode != MTGPU_DRIVER_MODE_HOST) {
			int i;

			/* find a free etc buffer */
			for (i = 0; i < DEC_ETC_NUM; i++) {
				if (!core->etc_buffers[inst_idx][i].used &&
				    core->etc_buffers[inst_idx][i].dev_addr &&
				    core->etc_buffers[inst_idx][i].size >= size) {
					node->dev_phys_addr = core->etc_buffers[inst_idx][i].dev_addr;
					core->etc_buffers[inst_idx][i].used = 1;
					node->vram_belonger = UMD_ALLOC;
					node->iova_addr = node->dev_phys_addr;
					VLOG(INFO, "get umd alloc etc buffer! core:%d, inst:%d, fbc_idx: %d,"
					     "addr:%llx, size:%llx, type: %d\n",
					     idx, inst_idx, i, node->dev_phys_addr, size, type);
					break;
				}
			}

			if (i >= DEC_ETC_NUM) {
				VLOG(ERR, "get umd alloc etc buffer failed! core:%d, inst:%d,"
				     "fbc_idx: %d, need size:%llx, umd alloc size:%llx, type:%d\n",
				     idx, inst_idx, i, size,
				     core->etc_buffers[inst_idx][0].size, type);
				goto nomem;
			}
		} else if (type == ENC_DEF_CDF && chip->driver_mode != MTGPU_DRIVER_MODE_HOST) {
			if (!core->def_cdf_buffers[inst_idx].used &&
			    core->def_cdf_buffers[inst_idx].dev_addr &&
			    core->def_cdf_buffers[inst_idx].size >= size) {
				node->dev_phys_addr = core->def_cdf_buffers[inst_idx].dev_addr;
				core->def_cdf_buffers[inst_idx].used = 1;
				node->vram_belonger = UMD_ALLOC;
				node->iova_addr = node->dev_phys_addr;
				VLOG(INFO, "get umd alloc enc_cdf buffer! core:%d, inst:%d, addr:%llx,"
				     "size:%llx, type: %d\n",
				     idx, inst_idx, node->dev_phys_addr, size, type);
			} else {
				VLOG(ERR, "get umd alloc enc_cdf buffer failed! core:%d, inst:%d,"
				     "need size:%llx, umd alloc size:%llx, type: %d\n",
				     idx, inst_idx, size, core->def_cdf_buffers[inst_idx].size, type);
				goto nomem;
			}
		} else {	/* chip->driver_mode == MTGPU_DRIVER_MODE_HOST */
			if (chip->soc_mode) {
				size = ALIGN(size, gpu_page_size * 2);
				ret = mtvpu_vram_alloc(drm, core->mem_group_id - 1, size,
						       &node->dev_phys_addr, &node->cpu_addr,
						       &node->iova_addr, &node->handle);
				if (ret) {
					vpu_err("soc mode alloc vram failed, core:%d, inst:%d!",
						idx, inst_idx);
					goto nomem;
				}
			} else {
				ret = mtgpu_vram_alloc(drm, MTGPU_SEGMENT_ID_VPU_HOST, size,
						       &node->dev_phys_addr, &node->handle);
				if (ret) {
					vpu_err("vdi alloc vcpu failed, core:%d, inst:%d!", idx, inst_idx);
					goto nomem;
				}

				if (inst_idx == 0xffff) {
					VLOG(INFO, "VRAM for fw and log, core:%d, addr:%llx size:%llx\n",
					     idx, node->dev_phys_addr, size);
				} else {
					core->inst_info[inst_idx].used_vcpu_size += size;
					node->vram_belonger = LINUX_HOST;
					VLOG(INFO, "alloc vram core:%d, inst:%d, addr:%llx,"
					     "size:%llx,vram_belonger:%d, type:%d", idx,
					     inst_idx, node->dev_phys_addr, size,
					     node->vram_belonger, type);
				}
			}
		}
	} else if (vpu_fixed_mem_qy2(chip, type)) {
		ret = vpu_mem_pool_alloc(chip, mmu_ctx, pool_id, size, &node->dev_phys_addr);
	} else {
		if (mmu_ctx && mmu_ctx->mmu_enable && type != FW_BUFFER) {
			ret = mtvpu_vm_pmr_create_and_map(OSGetDeviceNodeFromDrm(drm),
							mmu_ctx->vpu_ctx,
							size,
							PVRSRV_MEMALLOCFLAG_GPU_WRITEABLE |
							PVRSRV_MEMALLOCFLAG_GPU_READABLE |
							PVRSRV_MEMALLOCFLAG_CPU_WRITEABLE |
							PVRSRV_MEMALLOCFLAG_CPU_READABLE,
							&node->handle,
							&node->dev_virt_addr,
							&node->dev_phys_addr,
							&node->cpu_addr);
		} else if (chip->soc_mode) {
			size = ALIGN(size, gpu_page_size * 2);
			ret = mtvpu_vram_alloc(drm, core->mem_group_id - 1, size, &node->dev_phys_addr, &node->cpu_addr, &node->iova_addr, &node->handle);
		} else {
			ret = mtgpu_vram_alloc(drm, core->mem_group_id, size, &node->dev_phys_addr, &node->handle);
		}
	}

	drm_gem_private_object_init(drm, node->obj, size);

	if (ret) {
		vpu_err("Error mtgpu_vram_alloc\n");
		goto unref;
	}

	node->size = size;
	node->pool_id = pool_id;
	return node;

unref:
	drm_gem_object_release(node->obj);
nomem:
	kfree(node->obj);
	kfree(node);
	return NULL;
}

/* check work buffer, task buffer cross with code buffer.
 * refer: https://confluence.mthreads.com/display/VD/VPU+buf+require#
 */
struct mt_node *gem_malloc(struct mt_chip *chip, int idx, int inst_idx, u64 size, u32 type,
			   struct list_head *mm_head)
{
	struct mt_node *node = NULL;
	struct drm_device *drm;
	int drm_id;
	struct mtvpu_mmu_ctx *mmu_ctx;
#ifndef SUPPORT_ION
	u32 pool_id = chip->core[idx].pool_ids[inst_idx];
#endif

	drm_id = chip->core[idx].drm_ids[inst_idx];
	if (drm_id < 0 || drm_id > chip->mpc_drm_cnt - 1)
		return NULL;

	drm = chip->drms[drm_id];
	if (!drm)
		return NULL;

	mmu_ctx = chip->core[idx].mmu_ctx[inst_idx];

#ifdef SUPPORT_ION
	node = ion_malloc_node(chip, idx, drm_id, size);
#else
	node = gem_malloc_node(chip, idx, inst_idx, pool_id, drm, mmu_ctx, size, type);
#endif
	if (!node) {
		gem_dump_core(chip, idx);
		return NULL;
	}

	/*
	 * Be careful. The VRAM for VPU can't use the address during
	 * 0 - (2M(code buffer) + 16K(log buffer)) and 0xfffef000UL - 4G (last 68K) of each 4G.
	 * VPU cpu's bit width is 32 bit. Video driver will remap the binary buffer to above
	 * address. If driver gives the buffers to VPU, VCPU will broke its binary buffer or other
	 * critical buffers. This will cause VPU hang.
	 */
	if (chip->conf.type != TYPE_PIHU1) {
		if ((node->dev_phys_addr & 0xffffffffUL) < (WAVE5_MAX_CODE_BUF_SIZE + FW_LOG_BUFFER_SIZE) ||
		    ((node->dev_phys_addr + size) & 0xffffffffUL) > W_VCPU_SPM_ADDR) {
			pr_err("mtvpu Error address(%llx to %llx size:%llx)\n", node->dev_phys_addr,
			node->dev_phys_addr + size, size);

			if (node->handle) {
				if (chip->soc_mode) {
					if (chip->io_domain)
						vpu_smmu_unmap(chip, node->iova_addr);
					else if (!enable_reserved_memory)
						os_dma_free_coherent(node->obj->dev->dev,
								node->obj->size,
								node->cpu_addr,
								node->dev_phys_addr);
				}
				mtgpu_vram_free(node->handle);
			}
			if (node->obj) {
				drm_gem_object_release(node->obj);
				kfree(node->obj);
			}
			kfree(node);
			return NULL;
		}

	}
	mutex_lock(chip->mm_lock);
	list_add(&node->list, mm_head);
	mutex_unlock(chip->mm_lock);

	return node;
}

struct mt_node *get_node_by_iova(struct mt_core *core, dma_addr_t iova_addr)
{
	int inst;
	struct mt_node *node = NULL;

	for (inst = 0; inst < INST_MAX_SIZE; inst++) {
		list_for_each_entry(node, &core->mm_head[inst], list) {
			if (node->iova_addr == iova_addr) {
				return node;
			}
		}
	}
	vpu_err("cannot find mt node for iova %llx\n", iova_addr);
	return NULL;
}

void gem_free_node(struct mt_chip *chip, struct mt_node *node)
{
	struct mtvpu_mmu_ctx *mmu_ctx = node->mmu_ctx;

	/* only do unmap when mmu disable */
	if (node->cpu_addr && (!mmu_ctx || !mmu_ctx->mmu_enable)) {
		if (chip->soc_mode) {
			if (chip->io_domain) {
				vpu_gem_vunmap_internal(node->handle, node->private_data);
			} else if (enable_reserved_memory) {
				os_memunmap(node->cpu_addr);
			}
		} else {
			iounmap(node->cpu_addr);
		}
	}

	if (node->bak_cpu_addr)
		vfree(node->bak_cpu_addr);

#ifdef SUPPORT_ION
	ion_free_node(node);
#else

	/* VDI case will not destroy the pool here */
	if (node->pool_id > 0)
		vpu_mem_pool_free(chip, node->pool_id);
	else if (node->vram_belonger != UMD_ALLOC) {
		if (mmu_ctx && mmu_ctx->mmu_enable) {
			mtvpu_vm_pmr_unmap_and_destroy(mmu_ctx->vpu_ctx, node->handle, node->dev_virt_addr, node->cpu_addr);
		} else if (chip->soc_mode) {
			if (chip->io_domain)
				vpu_smmu_unmap(chip, node->iova_addr);
			else if (!enable_reserved_memory)
				os_dma_free_coherent(node->obj->dev->dev,
							  node->obj->size,
							  node->cpu_addr,
							  node->dev_phys_addr);
			mtgpu_vram_free(node->handle);
		} else {
			mtgpu_vram_free(node->handle);
		}
	}

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
		if (node->dev_phys_addr == dev_addr || node->dev_virt_addr == dev_addr) {
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
	struct mt_chip *chip = core->priv;
	struct mt_node *node = NULL;
	int inst;

	for (inst = 0; inst < INST_MAX_SIZE; inst ++) {
		list_for_each_entry(node, &core->mm_head[inst], list) {
			if (node->dev_phys_addr == dev_addr || node->dev_virt_addr == dev_addr) {
				memset_io(node->cpu_addr, 0, node->obj->size);
				if (chip->soc_mode)
					dcache_flush(node->cpu_addr, node->obj->size);
				return;
			}
		}
	}
}

void gem_dump_instance(struct mt_chip *chip, struct mt_core *core, u32 instIdx)
{
	u32 nodeIdx = 0;
	struct mt_node *node = NULL, *next = NULL;
	struct list_head *mm_head = &core->mm_head[instIdx];

	if (list_empty(mm_head))
		return;

	mutex_lock(chip->mm_lock);
	vpu_info("Instance#%d:\n", instIdx);
	list_for_each_entry_safe(node, next, mm_head, list) {
		vpu_info("\tnode#%d start 0x%llx, end 0x%llx, size 0x%llx\n", nodeIdx++,
			node->dev_phys_addr, node->dev_phys_addr + node->size, node->size);
	}
	mutex_unlock(chip->mm_lock);
}

void gem_dump_core(struct mt_chip *chip, u32 coreIdx)
{
	u32 instIdx = 0;
	struct mt_core *core = get_core(coreIdx);

	if (!core)
		return;

	vpu_info("Core#%d Memory Info\n", coreIdx);
	for (instIdx = 0; instIdx < INST_MAX_SIZE; instIdx++) {
		gem_dump_instance(chip, core, instIdx);
	}
}
