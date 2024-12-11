/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/platform_device.h>
#include <linux/component.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/firmware.h>
#include <linux/kthread.h>
#include <linux/mod_devicetable.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <drm/drm_gem.h>
#include <drm/drm_drv.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_ioctl.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#endif

#include "mtgpu_drv.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_drm_internal.h"
#include "mtgpu_module_param.h"
#include "mtgpu_device.h"
#include "mtgpu_smc.h"
#include "os-interface.h"

#include "mtvpu_drv.h"
#include "mtvpu_mem.h"
#include "mtvpu_mon.h"
#include "mtvpu_pool.h"
#include "mtvpu_smmu.h"

#include "misc.h"

#ifndef MT_BUILD_VPU
#define MT_BUILD_VPU ""
#endif

int enable_pm_mtvpu_backup_device_memory = 0;
module_param(enable_pm_mtvpu_backup_device_memory, int, 0444);
MODULE_PARM_DESC(enable_pm_mtvpu_backup_device_memory,
		 "mtgpu enable vpu component backup device memory in power manager");

#ifdef DEBUG
int mtvpu_log_level = MTDEBUG;
#else
int mtvpu_log_level = WARN;
#endif

module_param(mtvpu_log_level, int, 0444);
MODULE_PARM_DESC(mtvpu_log_level,
		 "mtgpu vpu component log level lower is important.");


int get_mtvpu_log_level(){
	return mtvpu_log_level;
}
void set_mtvpu_log_level(int log_level){
	mtvpu_log_level = log_level;
}

static int vpu_component_bind(struct device *dev, struct device *master, void *data)
{
	struct mt_chip *chip = NULL;
	struct drm_device *drm = data;
	struct mtgpu_drm_private *private = drm->dev_private;
	struct platform_device *pdev = to_platform_device(dev);
	struct pci_dev *pcid = NULL;
	struct mtgpu_device *mtdev;
	ulong offset = 0;
	char name[32];
	u32 core_bits = 0;
	int dec_bit_offset = 0;
	int enc_bit_offset = 8;
	int boda_bit_offset = 14;
	int i, j, ret;
	int jpu_core_size = 0;
	struct file_operations *vinfo, *fwinfo;

	bool is_host = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_HOST;
	bool is_guest = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_GUEST;
	bool is_native = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_NATIVE;

	vpu_info("Init %s, Version %s\n", is_native ? "Native" : is_host ? "Host" : "Guest", MT_BUILD_VPU);
	chip = kzalloc(sizeof(struct mt_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->inst_cnt = 0;
	chip->parent = master->parent;
	chip->dev = dev;
	chip->drm_host = drm;
	private->chip = chip;
	chip->driver_mode = mtgpu_get_driver_mode();

	if (os_dev_is_pci(chip->parent))
		chip->soc_mode = false;
	else
		chip->soc_mode = true;

	if (!chip->soc_mode) {
		pcid = to_pci_dev(chip->parent);
		chip->pdev = pcid;
	} else {
		chip->pdev = NULL;
	}

	mtdev = dev_get_drvdata(chip->parent);
	if (unlikely(!mtdev)) {
		vpu_err("Failed to get mtdev\n");
		goto err_timer;
	}

	if (chip->soc_mode) {
		if (vpu_init_conf(0x0A00, chip, false))
			goto err_timer;
	} else {
		if (vpu_init_conf(pcid->device, chip, is_guest))
			goto err_timer;
	}

	chip->timer = os_create_timer_list();
	if (!chip->timer)
		goto err_timer;

	for (i = 0; i < MAX_HOST_VPU_GROUPS_GEN1_GEN2; i++) {
		chip->host_thread_semas[i] = kzalloc(sizeof(*chip->host_thread_semas[i]), GFP_KERNEL);
		if (!chip->host_thread_semas[i])
			goto err_host_init;
		sema_init(chip->host_thread_semas[i], 0);

		chip->vm_locks[i] = kzalloc(sizeof(*chip->vm_locks[i]), GFP_KERNEL);
		if (!chip->vm_locks[i])
			goto err_host_init;
		mutex_init(chip->vm_locks[i]);

		INIT_LIST_HEAD(&chip->vm_heads[i]);
	}

	ret = vpu_mmu_context_create(drm, &chip->mmu_ctx);
	if (ret)
		goto err_create_mmu_ctx;

	chip->shared_mem_lock = kzalloc(sizeof(*chip->shared_mem_lock), GFP_KERNEL);
	if (!chip->shared_mem_lock)
		goto err_shared_mem_lock;
	spin_lock_init(chip->shared_mem_lock);

	chip->mm_lock = kzalloc(sizeof(*chip->mm_lock), GFP_KERNEL);
	if (!chip->mm_lock)
		goto err_mm_lock;
	mutex_init(chip->mm_lock);

	chip->sync.intr_lock = kzalloc(sizeof(*chip->sync.intr_lock), GFP_KERNEL);
	if (!chip->sync.intr_lock)
		goto err_intr_lock;
	spin_lock_init(chip->sync.intr_lock);

	chip->sync.sync_lock = kzalloc(sizeof(*chip->sync.sync_lock), GFP_KERNEL);
	if (!chip->sync.sync_lock)
		goto err_sync_lock;
	spin_lock_init(chip->sync.sync_lock);

	chip->mpc_lock = kzalloc(sizeof(*chip->mpc_lock), GFP_KERNEL);
	if (!chip->mpc_lock)
		goto err_mpc_lock;
	spin_lock_init(chip->mpc_lock);

	chip->inst_cnt_lock = kzalloc(sizeof(*chip->inst_cnt_lock), GFP_KERNEL);
	if (!chip->inst_cnt_lock)
		goto err_inst_cnt_lock;
	mutex_init(chip->inst_cnt_lock);

	chip->pool_lock = kzalloc(sizeof(*chip->pool_lock), GFP_KERNEL);
	if (!chip->pool_lock)
		goto err_pool_lock;
	spin_lock_init(chip->pool_lock);

	for (i = 0; i < chip->conf.core_size; i++) {
		chip->sync.core_lock[i] = kzalloc(sizeof(*chip->sync.core_lock[i]), GFP_KERNEL);
		if (!chip->sync.core_lock[i])
			goto err_core_lock;
		spin_lock_init(chip->sync.core_lock[i]);
	}

	chip->sync.sema = kzalloc(sizeof(*chip->sync.sema), GFP_KERNEL);
	if (!chip->sync.sema)
		goto err_core_lock;
	sema_init(chip->sync.sema, 0);

	for (i = 0; i < SYNC_ADDR_SIZE; i++) {
		chip->sync.addr_wait[i] = kzalloc(sizeof(struct wait_queue_head), GFP_KERNEL);
		if (!chip->sync.addr_wait[i])
			goto err_sync_addr_wait;

		init_waitqueue_head(chip->sync.addr_wait[i]);
	}

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++) {
				chip->sync.inst_wait[i][j] =
					kzalloc(sizeof(struct wait_queue_head), GFP_KERNEL);
				if (!chip->sync.inst_wait[i][j])
					goto err_sync_inst_wait;

				init_waitqueue_head(chip->sync.inst_wait[i][j]);
			}
		}
	}

	chip->sync.addr_idx = 0;
	chip->sync.idx = 0;

	for (i = 0; i < chip->conf.core_size; i++) {
		chip->core[i].regs_lock = kzalloc(sizeof(*chip->core[i].regs_lock), GFP_KERNEL);
		if (!chip->core[i].regs_lock)
			goto err_lock;
		mutex_init(chip->core[i].regs_lock);

		chip->core[i].open_lock = kzalloc(sizeof(*chip->core[i].open_lock), GFP_KERNEL);
		if (!chip->core[i].open_lock)
			goto err_lock;
		mutex_init(chip->core[i].open_lock);

		if (chip->conf.product[i] == WAVE517_CODE
		    || chip->conf.product[i] == WAVE627_CODE
		    || chip->conf.product[i] == WAVE627B_CODE)
			for (j = 0; j < INST_MAX_SIZE; j++) {
				chip->core[i].inst_lock[j] =
					kzalloc(sizeof(*chip->core[i].inst_lock[j]), GFP_KERNEL);
				if (!chip->core[i].inst_lock[j])
					goto err_lock;
				mutex_init(chip->core[i].inst_lock[j]);
			}
		if (chip->conf.type == TYPE_PIHU1) {
			chip->core[i].mem_group_id = 1;
			chip->core[i].mem_group_base = 0;
		}
	}

	chip->mpc_drm_cnt = mtgpu_get_primary_core_count(mtdev);
	chip->mem_group_cnt = mtdev->video_group_cnt;
	for (i = 0, j = 0; i < MTGPU_CORE_COUNT_MAX; i++) {
		if (mtdev->drm_dev[i]) {
			chip->drms[j] = os_dev_get_drvdata(&mtdev->drm_dev[i]->dev);
			if (!chip->drms[j])
				goto err_lock;
#ifdef SUPPORT_ION
			chip->ions[j] = mtdev->ion_dev[j];
#endif
			chip->mpc_group_ids[j] = mtdev->video_mpc_group_ids[j];
			private = chip->drms[j]->dev_private;
			private->chip = chip;
			j++;
		}
	}
	if (j != chip->mpc_drm_cnt)
		goto err_lock;

	ret = mtgpu_smc_get_vpu_core_info(chip->parent, &core_bits);
	if (ret)
		core_bits = 0;

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			if ((core_bits >> dec_bit_offset) & 0x1) {
				vpu_info("dec core:%d offset:%d not available\n", i, dec_bit_offset);
				chip->core[i].available = 0;
			} else {
				chip->core[i].available = 1;
				chip->core[i].drm = drm;
				chip->core[i].product_id = WAVE517_CODE;
				vpu_info("dec core:%d offset:%d available\n", i, dec_bit_offset);
			}
			dec_bit_offset++;
		} else if (chip->conf.product[i] == WAVE627_CODE ||
			   chip->conf.product[i] == WAVE627B_CODE) {
			if ((core_bits >> enc_bit_offset) & 0x1) {
				vpu_info("enc core:%d offset:%d not available\n", i, enc_bit_offset);
				chip->core[i].available = 0;
			} else {
				chip->core[i].available = 1;
				chip->core[i].drm = drm;
				chip->core[i].product_id = chip->conf.product[i];
				vpu_info("enc core:%d offset:%d available\n", i, enc_bit_offset);
			}
			enc_bit_offset++;
		} else if (chip->conf.product[i] == CODA980_CODE) {
			if ((core_bits >> boda_bit_offset) & 0x1) {
				vpu_info("dec core:%d offset:%d not available\n", i, boda_bit_offset);
				chip->core[i].available = 0;
			} else {
				chip->core[i].available = 1;
				chip->core[i].drm = drm;
				chip->core[i].product_id = CODA980_CODE;
				vpu_info("dec core:%d offset:%d available\n", i, boda_bit_offset);
			}
		} else if (chip->conf.product[i] == CODAJ12_CODE) {
			chip->core[i].available = 1;
			chip->core[i].drm = drm;
			chip->core[i].product_id = CODAJ12_CODE;
			jpu_core_size++;
			vpu_info("jpu core:%d available\n", i);
		} else if (chip->conf.product[i] == CORTEXA35_CODE) {
			chip->core[i].available = 1;
			chip->core[i].drm = drm;
			chip->core[i].product_id = CORTEXA35_CODE;
			vpu_info("mcu core:%d available\n", i);
		}
		chip->core[i].idx = i;
		chip->core[i].priv = chip;
		for (j = 0; j < INST_MAX_SIZE; j++) {
			INIT_LIST_HEAD(&chip->core[i].mm_head[j]);
		}
	}

	chip->jpu_core_sema = kzalloc(sizeof(*chip->jpu_core_sema), GFP_KERNEL);
	if (!chip->jpu_core_sema)
		goto err_lock;
	sema_init(chip->jpu_core_sema, jpu_core_size);

	if (is_native || is_host) {
		if (chip->soc_mode)
			chip->vaddr = ioremap(chip->conf.regs_base, chip->conf.regs_chip_size);
		else
			chip->vaddr = ioremap(pci_resource_start(pcid, 0) + chip->conf.regs_base, chip->conf.regs_chip_size);

		if (!chip->vaddr)
			goto err_lock;

		for (i = 0; i < chip->conf.core_size; i++) {
			chip->core[i].regs = chip->vaddr + offset;
			offset += chip->conf.regs_core_size[i];
		}
	}

	if (chip->soc_mode) {
		if (enable_reserved_memory)
			chip->mem_group_cnt = 1;
		else
			chip->mem_group_cnt = 2;

		vpu_smmu_init(chip);

		chip->bar_base = 0;
		for (i = 0; i < chip->conf.core_size; i++) {
			chip->core[i].mem_group_id = chip->conf.core_group[chip->mem_group_cnt - 1][i];
			if (chip->io_domain) {
				if (chip->core[i].mem_group_id == 1)
					chip->core[i].mem_group_base = VPU_SMMU_MEM_BASE1;
				else if (chip->core[i].mem_group_id == 2)
					chip->core[i].mem_group_base = VPU_SMMU_MEM_BASE2;
			} else if (enable_reserved_memory) {
				chip->core[i].mem_group_base = mtdev->gpu_mem.base & VPU_SMMU_MEM_BASE_MASK;
			} else {
				vpu_err("smmu must be enabled!\n");
				goto err_lock;
			}
			vpu_info("core %d, mem base 0x%x\n", i, chip->core[i].mem_group_base >> 32);
		}
	} else {
		chip->bar_base = pci_resource_start(pcid, 2);
		ret = vpu_init_mpc(chip);
		if (ret)
			goto err_lock;
	}

	if (is_guest_cmds)
		vpu_init_guest_mem(chip);

	ret = vpu_init_irq(chip, pdev);
	if (ret)
		goto err_lock;

	platform_set_drvdata(pdev, chip);

	ret = vpu_fill_drm_ioctls(pvr_drm_ioctls, 128);
	if (ret)
		goto err_lock;

	sprintf(name, "mtvpu-sync/%d", chip->idx);
	chip->sync_thread = kthread_create(vpu_sync_thread, chip, name);
	wake_up_process(chip->sync_thread);

	if (chip->conf.type == TYPE_QUYU2) {
		if (is_host)
			vpu_set_vm_core(chip);
		if (is_host || is_guest) {
			for (i = 0; i < chip->conf.core_size; i++) {
				if (chip->conf.product[i] != CODA980_CODE)
					vpu_load_firmware(chip, i, NULL);
			}
		}
	} else if (chip->conf.type == TYPE_PIHU1) {
		for (i = 0; i < chip->conf.core_size; i++) {
			if (chip->conf.product[i] == CORTEXA35_CODE)
				vpu_load_firmware(chip, i, NULL);
		}
	} else if (is_host) {
		/* wake up the group1 thread, it is the main thread */
		sprintf(name, "vpu-host-g1%d", chip->idx);
		chip->host_threads[0] = kthread_create(vpu_host_thread1, chip, name);
		wake_up_process(chip->host_threads[0]);
	}

	vinfo = get_vinfo_fops();
	fwinfo = get_fwinfo_fops();
	sprintf(name, "mtvpu%d", chip->idx);
	chip->debugfs = debugfs_create_dir(name, NULL);
	if (chip->debugfs) {
		debugfs_create_file("info", 0666, chip->debugfs, chip, vinfo);
		debugfs_create_file("fw", 0444, chip->debugfs, chip, fwinfo);
	}

	os_set_timer_list_drvdata(chip->timer, chip);
	timer_setup(chip->timer, vpu_monitor, 0);
	mod_timer(chip->timer, jiffies + msecs_to_jiffies(1000));

	if (is_native || is_host) {
		for (i = 0; i < chip->conf.core_size; i++) {
			struct mt_core *core = &chip->core[i];

			core->core_freq = vpu_get_clk(chip, i);
		}

		for (i = 0; i < chip->conf.core_size; i++)
			if (chip->conf.product[i] == WAVE627_CODE)
				vpu_slice_mode_config(chip, i);
	}

	vpu_report_power_state(chip, 0);

	return 0;

err_lock:
	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE
		    || chip->conf.product[i] == WAVE627_CODE
		    || chip->conf.product[i] == WAVE627B_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++) {
				if (chip->core[i].inst_lock[j]) {
					mutex_destroy(chip->core[i].inst_lock[j]);
					kfree(chip->core[i].inst_lock[j]);
				}
			}
		}
	}

	kfree(chip->sync.sema);
	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->core[i].open_lock) {
			mutex_destroy(chip->core[i].open_lock);
			kfree(chip->core[i].open_lock);
		}
		if (chip->core[i].regs_lock) {
			mutex_destroy(chip->core[i].regs_lock);
			kfree(chip->core[i].regs_lock);
		}
	}
err_sync_inst_wait:
	os_kfree(chip->jpu_core_sema);

	for (i = 0; i < chip->conf.core_size; i++)
		if (chip->conf.product[i] == WAVE517_CODE)
			for (j = 0; j < INST_MAX_SIZE; j++)
				if (chip->sync.inst_wait[i][j])
					kfree(chip->sync.inst_wait[i][j]);
err_sync_addr_wait:
	for (i = 0; i < SYNC_ADDR_SIZE; i++)
		if (chip->sync.addr_wait[i])
			kfree(chip->sync.addr_wait[i]);
err_core_lock:
	kfree(chip->pool_lock);
	for (i = 0; i < CORE_MAX_SIZE; i++)
		if (chip->sync.core_lock[i])
			kfree(chip->sync.core_lock[i]);
err_pool_lock:
	mutex_destroy(chip->inst_cnt_lock);
	kfree(chip->inst_cnt_lock);
err_inst_cnt_lock:
	kfree(chip->mpc_lock);
err_mpc_lock:
	kfree(chip->sync.sync_lock);
err_sync_lock:
	kfree(chip->sync.intr_lock);
err_intr_lock:
	mutex_destroy(chip->mm_lock);
	kfree(chip->mm_lock);
err_mm_lock:
	kfree(chip->shared_mem_lock);
err_shared_mem_lock:
	if (chip->mmu_ctx) {
		vpu_mmu_context_destroy(chip->mmu_ctx);
		chip->mmu_ctx = NULL;
	}
err_create_mmu_ctx:
	for (i = 0; i < MAX_HOST_VPU_GROUPS_GEN1_GEN2; i++) {
		if (chip->vm_locks[i]) {
			mutex_destroy(chip->vm_locks[i]);
			kfree(chip->vm_locks[i]);
		}
		if (chip->host_thread_semas[i])
			kfree(chip->host_thread_semas[i]);
	}
err_host_init:
	kfree(chip->timer);
	vpu_deinit_conf(chip);
err_timer:
	kfree(chip);
	vpu_err("Init Error\n");
	return -ENODEV;
}

static void vpu_component_unbind(struct device *dev, struct device *master, void *data)
{
	struct mt_chip *chip;
	struct mt_core *core;
	int idx, i, j;

	chip = dev_get_drvdata(dev);

	del_timer_sync(chip->timer);

	kfree(chip->timer);

	if (chip->debugfs)
		debugfs_remove_recursive(chip->debugfs);

	for (i = 0; i < MAX_HOST_VPU_GROUPS_GEN1_GEN2; i++) {
		if (chip->host_threads[i])
			kthread_stop(chip->host_threads[i]);
	}

	if (chip->sync_thread)
		kthread_stop(chip->sync_thread);

	for (idx = 0; idx < chip->conf.core_size; idx++) {
		core = &chip->core[idx];
		if (core->bak_addr) {
			vfree(core->bak_addr);
			core->bak_addr = NULL;
		}
		if (core->fw) {
			vpu_hw_deinit(chip->conf.core_base + idx);
			release_firmware(core->fw);
			core->inited = 0;
			core->fw = NULL;
		}

		if (core->open_lock) {
			mutex_destroy(core->open_lock);
			kfree(core->open_lock);
			core->open_lock = NULL;
		}
		if (core->regs_lock) {
			mutex_destroy(core->regs_lock);
			kfree(core->regs_lock);
			core->regs_lock = NULL;
		}
	}

	for (i = 0; i < MEM_POOL_MAX_SIZE; i ++)
		vpu_destroy_mem_pool(chip, i + 1);

	vpu_free_irq(chip);
	vpu_free_fw_mem(chip);
	kfree(chip->jpu_core_sema);
	kfree(chip->sync.sema);
	kfree(chip->sync.sync_lock);
	kfree(chip->pool_lock);
	mutex_destroy(chip->inst_cnt_lock);
	kfree(chip->inst_cnt_lock);
	kfree(chip->mpc_lock);

	for (i = 0; i < SYNC_ADDR_SIZE; i++)
		kfree(chip->sync.addr_wait[i]);
	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++)
				kfree(chip->sync.inst_wait[i][j]);
		}
	}

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE
		    || chip->conf.product[i] == WAVE627_CODE
		    || chip->conf.product[i] == WAVE627B_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++) {
				if (chip->core[i].inst_lock[j]) {
					mutex_destroy(chip->core[i].inst_lock[j]);
					kfree(chip->core[i].inst_lock[j]);
				}
			}
		}
	}

	for (idx = 0; idx < CORE_MAX_SIZE; idx++)
		if (chip->sync.core_lock[idx])
			kfree(chip->sync.core_lock[idx]);

	kfree(chip->sync.intr_lock);
	kfree(chip->shared_mem_lock);
	mutex_destroy(chip->mm_lock);
	kfree(chip->mm_lock);

	for (i = 0; i < MAX_HOST_VPU_GROUPS_GEN1_GEN2; i++) {
		mutex_destroy(chip->vm_locks[i]);
		kfree(chip->vm_locks[i]);
		kfree(chip->host_thread_semas[i]);
	}

	if (chip->soc_mode)
		vpu_smmu_deinit(chip);

	if (chip->mmu_ctx) {
		vpu_mmu_context_destroy(chip->mmu_ctx);
		chip->mmu_ctx = NULL;
	}

	vpu_deinit_conf(chip);
	kfree(chip);
}

static const struct component_ops mtvpu_component_ops = {
	.bind   = vpu_component_bind,
	.unbind = vpu_component_unbind,
};

static struct platform_device_id vpu_id_tbl[] = {
	{ .name = "mtgpu_vde" },
	{}
};

static struct of_device_id vpu_of_id_tbl[] = {
	{ .compatible = "mthreads,apollo-vpu"}
};

static struct acpi_device_id vpu_acpi_id_tbl[] = {
	{.id = "MVPU0001", .driver_data = 0}
};

static int vpu_probe(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &mtvpu_component_ops);
}

static int vpu_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &mtvpu_component_ops);
	return 0;
}

static const struct dev_pm_ops vpu_pm_ops = {
	.suspend = vpu_suspend,
	.resume  = vpu_resume,
	.freeze  = vpu_suspend,
	.restore = vpu_resume,
};

struct platform_driver vpu_driver = {
	.driver = {
		.name = "mtgpu_vde",
		.pm = &vpu_pm_ops,
		.of_match_table = vpu_of_id_tbl,
		.acpi_match_table = vpu_acpi_id_tbl,
	},
	.probe = vpu_probe,
	.remove = vpu_remove,
	.id_table = vpu_id_tbl,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
MODULE_IMPORT_NS(DMA_BUF);
#endif
