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
#include <linux/devfreq.h>
#include <linux/pm_opp.h>
#include <linux/acpi.h>
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

static u64 vdi_guest_mem_usage = 0;

struct file_operations vinfo_fops = {
	.owner = THIS_MODULE,
	.read = vpu_info_read,
	.write = vpu_info_write,
};

struct file_operations fwinfo_fops = {
	.owner = THIS_MODULE,
	.read = fw_info_read,
};

struct file_operations vpulog_fops = {
	.owner = THIS_MODULE,
	.read = vpu_log_read,
};

struct file_operations *get_vinfo_fops(void)
{
	return &vinfo_fops;
}

struct file_operations *get_fwinfo_fops(void)
{
	return &fwinfo_fops;
}

struct file_operations *get_vpulog_fops(void)
{
	return &vpulog_fops;
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
	int err = 0;

	if (vpu_fixed_mem_qy2(chip, type)) {
		err = vpu_mem_pool_alloc(chip, pool_id, size, &mtgpu_obj->dev_addr);

		set_mtgpu_obj_type(mtgpu_obj, group_id, pool_id);
		set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	} else {
		if (chip->soc_mode)
			err = mtvpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);
		else
			err = mtgpu_vram_alloc(drm, group_id, size, &mtgpu_obj->dev_addr, &mtgpu_obj->handle);
		if(!err) {
			vdi_guest_mem_usage += size;
			VLOG(MTDEBUG, "guest alloc vram succ, pid:%4d, group:%d, type:%d, addr:0x%08llx,"
			     "size:0x%08llx, vdi_guest_mem_usage:0x%08llx\n", current->pid, group_id,
			     type, mtgpu_obj->dev_addr, size, vdi_guest_mem_usage += size);
		} else
			VLOG(MTDEBUG, "guest alloc vram fail, pid:%4d, group:%d, type:%d, addr:0x%08llx,"
			     "size:0x%08llx, vdi_guest_mem_usage:0x%08llx\n", current->pid, group_id,
			     type, mtgpu_obj->dev_addr, size, vdi_guest_mem_usage);
		set_mtgpu_obj_type(mtgpu_obj, group_id, 0);
		set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);
	}

	return err;
}

void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj)
{
	struct mt_chip *chip;
	struct mtvpu_gem_priv *priv;

	if (mtgpu_obj && mtgpu_obj->obj) {
		chip = to_chip(mtgpu_obj->obj->dev);
		priv = (struct mtvpu_gem_priv *)mtgpu_obj->private_data;

		if (mtgpu_obj->handle) {
			if (chip->soc_mode) {
				if (chip->io_domain) {
					vpu_smmu_unmap(chip, mtgpu_obj->dev_addr);
					if (priv && priv->priv_data)
						mtgpu_vram_vunmap(mtgpu_obj->handle, priv->priv_data);
				}
				mtgpu_vram_free(mtgpu_obj->handle);
			} else {
				mtgpu_vram_free(mtgpu_obj->handle);
			}
		}

		vdi_guest_mem_usage -= mtgpu_obj->obj->size;
		VLOG(MTDEBUG, "guest free vram, pid:%4d, addr:0x%08llx, size:0x%08llx,"
		     "vdi_guest_mem_usage:0x%08llx\n", current->pid, mtgpu_obj->dev_addr,
		     mtgpu_obj->obj->size, vdi_guest_mem_usage -= mtgpu_obj->obj->size);
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

/* only used for group change in soc mode */
int vpu_gem_modify(struct drm_device *drm, struct mtgpu_gem_object *mtgpu_obj, u32 group_id)
{
	struct mt_chip *chip = to_chip(drm);
	struct sg_table *sgt;
	int ret, size;

	if (!chip || !mtgpu_obj)
		return -1;

	if (!chip->soc_mode)
		return -1;

	if (chip->io_domain)
		vpu_smmu_unmap(chip, mtgpu_obj->dev_addr);
	else
		return -1;

	mtgpu_obj->dev_addr = 0;
	size = os_get_drm_gem_object_size(mtgpu_obj->obj);
	sgt = vpu_gem_map_internal(mtgpu_obj->handle, size);
	ret = vpu_smmu_map_sg(chip, sgt, size, group_id, &mtgpu_obj->dev_addr);
	vpu_gem_unmap_internal(sgt);

	if (ret)
		return ret;
	set_mtgpu_obj_type(mtgpu_obj, group_id, 0);
	set_mtgpu_obj_addr(mtgpu_obj, chip->bar_base, mtgpu_obj->dev_addr);

	return ret;
}

void mtvpu_gem_free_obj(struct drm_gem_object *obj)
{
	struct mtgpu_gem_object *mtgpu_obj = os_get_drm_gem_object_drvdata(obj);

	vpu_vram_free(mtgpu_obj);
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

static int acpi_dev_pm_explicit_get(struct acpi_device *device, int *state)
{
	unsigned long long psc;
	acpi_status status;

	status = acpi_evaluate_integer(device->handle, "_PSC", NULL, &psc);
	if (ACPI_FAILURE(status))
		return -ENODEV;

	*state = psc;
	return 0;
}

static int acpi_dev_pm_explicit_set(struct acpi_device *adev, int state)
{
	acpi_status status;
	if (adev->power.states[state].flags.explicit_set) {
		char method[5] = { '_', 'P', 'S', '0' + state, '\0' };

		status = acpi_evaluate_object(adev->handle, method, NULL, NULL);
		if (ACPI_FAILURE(status))
			return -ENODEV;
	}
	return 0;
}

/* for M1000 power acquire */
int vpu_get_core_power(struct device *dev, int *power)
{
	int ret = -1;
	struct acpi_device *adev = ACPI_COMPANION(dev);

	if (adev) {
		ret = acpi_dev_pm_explicit_get(adev, power);
	}
	return ret;
}

/* for M1000 power control */
int vpu_set_core_power(struct device *dev, int power)
{
	int ret = -1;
	struct acpi_device *adev = ACPI_COMPANION(dev);

	if (adev) {
		if (power == 0)
			ret = acpi_dev_pm_explicit_set(adev, ACPI_STATE_D3_HOT);
		else if (power == 1)
			ret = acpi_dev_pm_explicit_set(adev, ACPI_STATE_D0);
		else
			vpu_err("set invalid power state %d\n", power);
	}
	return ret;
}

struct devfreq *os_devfreq_add_device(struct device *dev,
				   struct devfreq_dev_profile *profile,
				   const char *governor_name,
				   void *data)
{
	return devm_devfreq_add_device(dev, profile, governor_name, data);
}

void os_devfreq_remove_device(struct device *dev, struct devfreq *devfreq)
{
	devm_devfreq_remove_device(dev, devfreq);
}

int os_dev_pm_opp_add(struct device *dev, unsigned long freq, unsigned long u_volt)
{
	return dev_pm_opp_add(dev, freq, u_volt);
}

void os_dev_pm_opp_remove(struct device *dev, unsigned long freq)
{
	dev_pm_opp_remove(dev, freq);
}

unsigned long os_dev_pm_opp_get_freq(struct dev_pm_opp *opp)
{
	return dev_pm_opp_get_freq(opp);
}

void os_dev_pm_opp_put(struct dev_pm_opp *opp)
{
	dev_pm_opp_put(opp);
}

int os_devfreq_register_opp_notifier(struct device *dev, struct devfreq *devfreq)
{
	return devfreq_register_opp_notifier(dev, devfreq);
}

int os_devfreq_unregister_opp_notifier(struct device *dev,
				struct devfreq *devfreq)
{
	return devfreq_unregister_opp_notifier(dev, devfreq);
}

struct dev_pm_opp *os_devfreq_recommended_opp(struct device *dev,
					   unsigned long *freq,
					   u32 flags)
{
	return devfreq_recommended_opp(dev, freq, flags);
}

struct devfreq_dev_profile vpu_devfreq_dev_profile =
{
	.target             = vpu_devfreq_target,
	.get_dev_status     = vpu_devfreq_get_dev_status,
	.get_cur_freq       = vpu_devfreq_cur_freq,
};

void vpu_init_devfreq_profile(u64 freq, u32 poll_ms)
{
	vpu_devfreq_dev_profile.initial_freq = freq;
	vpu_devfreq_dev_profile.polling_ms = poll_ms;
}

int	vpu_init_devfreq_data(struct mt_chip *chip)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 10))
	chip->devfreq_data = os_kzalloc(sizeof(struct devfreq_simple_ondemand_data));
	if (!chip->devfreq_data)
		return -OS_VAL(ENOMEM);

	chip->devfreq_data->upthreshold = 90;
	chip->devfreq_data->downdifferential = 50;
#endif
	return 0;
}

void vpu_init_devfreq_freq(struct mt_chip *chip, u64 min_freq, u64 max_freq)
{
	if (!chip->dev_freq)
		return;
	chip->dev_freq->scaling_min_freq = min_freq;
	chip->dev_freq->scaling_max_freq = max_freq;
}

void vpu_devfreq_set_dev_status(struct mt_chip *chip, struct devfreq_dev_status *stat)
{
	struct mt_core *core;
	int drm_idx, i;
	int max_index = 0;
	u64 cycle, max_cycle = 0;

	if (!chip || !stat)
		return;

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			core = &chip->core[i];
			cycle = 0;
			for (drm_idx = 0; drm_idx < chip->mpc_drm_cnt; drm_idx++)
				cycle += core->core_drm_cycle[drm_idx];
			if (cycle > max_cycle) {
				max_cycle = cycle;
				max_index = i;
			}
		}
	}

	stat->current_frequency = chip->curr_freq;
	stat->busy_time = max_cycle;
	stat->total_time = chip->core[max_index].core_freq / 1000 * VPU_UTIL_DURATION;
}

enum {
	MT_VPU_CREATE_SEMA = 0,
	MT_VPU_DESTROY_SEMA,
	MT_VPU_WAIT_SEMA,
};

int vpu_sema_cmd_proc(u64 type, u64 *data, struct drm_file *file)
{
	struct mt_file *priv = os_get_drm_file_private_data(file);
	struct mtvpu_sema_list *cur, *next;
	int ret = -1;
	switch (type)
	{
	case MT_VPU_CREATE_SEMA:
		cur = kzalloc(sizeof(*cur), GFP_KERNEL);
		if (!cur)
			return -ENOMEM;
		cur->sema = kzalloc(sizeof(*(cur->sema)), GFP_KERNEL);
		if (!cur->sema) {
			kfree(cur);
			return -ENOMEM;
		}
		sema_init(cur->sema, 0);
		os_mutex_lock(priv->file_lock);
		list_add_tail(&cur->list, &priv->sema_head);
		os_mutex_unlock(priv->file_lock);
		*data = (u64)cur->sema;
		ret = 0;
		break;
	case MT_VPU_DESTROY_SEMA:
		os_mutex_lock(priv->file_lock);
		list_for_each_entry_safe(cur, next, &priv->sema_head, list) {
			if (*data == (u64)cur->sema) {
				list_del(&cur->list);
				kfree(cur->sema);
				kfree(cur);
				ret = 0;
				break;
			}
		}
		os_mutex_unlock(priv->file_lock);
		break;

	default:
		vpu_err("wrong sema cmd in vpu: %d\n", (u32)type);
		return -1;
	}
	return ret;
}

void vpu_sema_release(struct drm_file *file)
{
	struct mt_file *priv = os_get_drm_file_private_data(file);
	struct mtvpu_sema_list *cur, *next;
	os_mutex_lock(priv->file_lock);
	list_for_each_entry_safe(cur, next, &priv->sema_head, list) {
		list_del(&cur->list);
		vpu_warn("maybe need to destroy the sema %p in user space.", cur->sema);
		kfree(cur->sema);
		kfree(cur);
	}
	os_mutex_unlock(priv->file_lock);
}

static inline int vpu_create_cmd(struct mt_vpu_cmd *cmd)
{
	int ret = 0;

	cmd->addr_wait = kzalloc(sizeof(struct wait_queue_head), GFP_KERNEL);
	if (likely(cmd->addr_wait))
		init_waitqueue_head(cmd->addr_wait);
	else
		ret = -1;
	return ret;
}

static inline void vpu_destroy_cmd(struct mt_vpu_cmd *cmd)
{
	if (cmd->addr_wait)
		kfree(cmd->addr_wait);
}

static inline void vpu_init_cmd(struct mt_vpu_cmd *cmd)
{
	init_waitqueue_head(cmd->addr_wait);
	cmd->addr_blocked = FALSE;
	cmd->status = 0;
	cmd->sur_addr = 0;
	cmd->sem = 0;
}

static void vpu_init_que(struct mt_vpu_cmd_que *que, struct CodecInst *handle, struct mt_virm *vm)
{
	int i;

	spin_lock(que->lock);
	init_waitqueue_head(que->inst_wait);
	que->pid = vm ? 0xFF000000 + vm->vm_id : current->tgid;

	for (i = 0; i < INST_Q_DEPTH; i++)
		vpu_init_cmd(que->cmd + i);

	que->vm = vm;
	que->handle = handle;
	que->rd_idx = 0;
	que->wr_idx = 0;
	que->count = 0;
	que->inst_blocked = FALSE;
	spin_unlock(que->lock);
}

int vpu_create_que(struct mt_vpu_cmd_que *que, struct spinlock *lock, struct CodecInst *handle, struct mt_virm *vm)
{
	int ret = 0, i;

	que->lock = lock;
	que->inst_wait = kzalloc(sizeof(struct wait_queue_head), GFP_KERNEL);
	que->pid = current->tgid;

	if (likely(que->inst_wait)) {
		for (i = 0; i < INST_Q_DEPTH; i++) {
			ret = vpu_create_cmd(que->cmd + i);
			if (unlikely(ret))
				goto err;
		}
		vpu_init_que(que, handle, vm);
	} else
		ret = -1;
	return ret;
err:
	for(i = i - 1; i >= 0; i--)
		vpu_destroy_cmd(que->cmd + i);
	kfree(que->inst_wait);
	return ret;
}

static void vpu_deinit_que(struct mt_vpu_cmd_que *que)
{
	spin_lock(que->lock);
	que->count = 0;
	que->rd_idx = 0;
	que->wr_idx = 0;
	que->vm = NULL;
	que->handle = NULL;
	spin_unlock(que->lock);
}

void vpu_destroy_que(struct mt_vpu_cmd_que *que)
{
	int i;

	vpu_deinit_que(que);
	for(i = 0; i < INST_Q_DEPTH; i++)
		vpu_destroy_cmd(que->cmd + i);
	if (que->inst_wait)
		kfree(que->inst_wait);
}

struct mt_vpu_irq_worker {
	struct work_struct work;
	struct mt_intr_map map;
	void *ctx;
};

void* vpu_create_irq_works(int num, void *ctx)
{
	struct mt_vpu_irq_worker *worker;
	int i;

	worker = kzalloc(sizeof(*worker) * num, GFP_KERNEL);
	if (!worker)
		return NULL;

	for (i = 0; i < num; i++) {
		INIT_WORK(&worker[i].work, vpu_irq_work);
		worker[i].ctx = ctx;
	}

	return worker;
}

void vpu_destroy_irq_works(void *works, int num)
{
	struct mt_vpu_irq_worker *worker = works;
	int i;

	for (i = 0; i < num; i++)
		cancel_work_sync(&worker[i].work);

	kfree(worker);
}

struct work_struct* vpu_irq_work_get_work(void *works, int idx)
{
	struct mt_vpu_irq_worker *worker = works;

	return &worker[idx].work;
}

void* vpu_irq_work_get_ctx(struct work_struct *work)
{
	struct mt_vpu_irq_worker *worker =
		container_of(work, struct mt_vpu_irq_worker, work);

	return worker->ctx;
}

struct mt_intr_map* vpu_irq_work_get_map(struct work_struct *work)
{
	struct mt_vpu_irq_worker *worker =
		container_of(work, struct mt_vpu_irq_worker, work);

	return &worker->map;
}