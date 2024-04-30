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
#include <drm/drm_gem.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_ioctl.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#endif

#ifndef SOC_MODE
#include "mtgpu_drv.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_device.h"
#include "mtgpu_smc.h"
#else
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <drm/drm_drv.h>
#endif
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

int mtvpu_log_level = WARN;
module_param(mtvpu_log_level, int, 0444);
MODULE_PARM_DESC(mtvpu_log_level,
		 "mtgpu vpu component log level lower is important.");

#ifdef SOC_MODE
extern struct drm_driver mtvpu_drm_driver;
#endif

#ifndef SOC_MODE
static int vpu_component_bind(struct device *dev, struct device *master, void *data)
#else
static int vpu_component_bind(struct platform_device *pdev)
#endif
{
	struct mt_chip *chip = NULL;
#ifndef SOC_MODE
	struct drm_device *drm = data;
	struct mtgpu_drm_private *private = drm->dev_private;
	struct platform_device *pdev = to_platform_device(dev);
	struct pci_dev *pcid;
	struct mtgpu_device *mtdev;
#else
	struct drm_device *drm;
#endif
	ulong offset = 0;
	char name[32];
	u32 core_bits = 0;
	int dec_bit_offset = 0;
	int enc_bit_offset = 8;
	int boda_bit_offset = 14;
	int i, j, ret;
	int jpu_core_size = 0;
	struct file_operations *vinfo, *fwinfo;
#ifdef SOC_MODE
	bool is_native = true;
	bool is_host = false;
	bool is_guest = false;
#else
	bool is_host = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_HOST;
	bool is_guest = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_GUEST;
	bool is_native = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_NATIVE;
#endif

	vpu_info("Init %s, Version %s\n", is_native ? "Native" : is_host ? "Host" : "Guest", MT_BUILD_VPU);
	chip = kzalloc(sizeof(struct mt_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

#ifdef SOC_MODE
	chip->dev = &pdev->dev;
	if (vpu_init_conf(0x0A00, chip, false))
		goto err_timer;

#else
	chip->inst_cnt = 0;
	chip->parent = dev->parent->parent;
	chip->dev = dev;
	chip->drm_host = drm;
	private->chip = chip;

	mtdev = dev_get_drvdata(chip->parent);
	if (unlikely(!mtdev)) {
		vpu_err("Failed to get mtdev\n");
		goto err_timer;
	}

	pcid = to_pci_dev(chip->parent);
	chip->pdev = pcid;

	if (vpu_init_conf(pcid->device, chip, is_guest))
		goto err_timer;
#endif

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
	}

#ifdef SOC_MODE
	drm = drm_dev_alloc(&mtvpu_drm_driver, chip->dev);
	if (!drm)
		goto err_lock;

	chip->drms[0] = drm;
	chip->mpc_drm_cnt = 1;

	dma_set_mask(chip->dev, DMA_BIT_MASK(40));
	drm_mode_config_init(drm);
	drm_mode_config_reset(drm);

	ret = drm_dev_register(drm, 0);
	if (ret)
		goto err_lock;

	drm->dev_private = chip;
	core_bits = 0;
#else
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
#endif

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
#ifdef SOC_MODE
		chip->vaddr = ioremap(chip->conf.regs_base, chip->conf.regs_chip_size);
#else
		chip->vaddr = ioremap(pci_resource_start(pcid, 0) + chip->conf.regs_base, chip->conf.regs_chip_size);
#endif
		if (!chip->vaddr)
			goto err_lock;

		for (i = 0; i < chip->conf.core_size; i++) {
			chip->core[i].regs = chip->vaddr + offset;
			if (chip->conf.type == TYPE_QUYU2 && chip->core[i].product_id == CODAJ12_CODE)
				offset += 0x1000;
			else
				offset += chip->conf.regs_core_size;
		}
	}

#ifdef SOC_MODE
	chip->bar_base = 0;
	chip->mem_group_cnt = 1;
	for (i = 0; i < chip->conf.core_size; i++) {
		chip->core[i].mem_group_id = chip->conf.core_group[chip->mem_group_cnt - 1][i];
		// should be 0xFF00000000 if SMMU enabled
		chip->core[i].mem_group_base = APOLLO_VPU_MEM_BASE & 0xFFFF00000000;
	}
	chip->mm = kzalloc(sizeof(struct drm_mm), GFP_KERNEL);
	if (!chip->mm)
		goto err_lock;
	drm_mm_init(chip->mm, APOLLO_VPU_MEM_BASE, APOLLO_VPU_MEM_SIZE);
#else
	chip->bar_base = pci_resource_start(pcid, 2);
	ret = vpu_init_mpc(chip);
	if (ret)
		goto err_lock;
#endif

	if (is_guest_cmds)
		vpu_init_guest_info(chip);

	ret = vpu_init_irq(chip, pdev);
	if (ret)
		goto err_lock;

	platform_set_drvdata(pdev, chip);

#ifndef SOC_MODE
	ret = vpu_fill_drm_ioctls(pvr_drm_ioctls, 128);
	if (ret)
		goto err_lock;
#endif

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
	} else if (is_host) {
		/* wake up the group1 thread, it is the main thread */
		sprintf(name, "vpu-host-g1%d", chip->idx);
		chip->host_threads[0] = kthread_create(vpu_host_thread_group1, chip, name);
		wake_up_process(chip->host_threads[0]);
	}

	vinfo = get_vinfo_fops();
	fwinfo = get_fwinfo_fops();
	sprintf(name, "mtvpu%d", chip->idx);
	chip->debugfs = debugfs_create_dir(name, NULL);
	if (chip->debugfs) {
		debugfs_create_file("info", 0444, chip->debugfs, chip, vinfo);
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

#ifdef SUPPORT_SMMU
	ret = vpu_smmu_init(chip);
	if (ret)
		goto err_lock;
#endif

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
err_host_init:
	for (i = 0; i < MAX_HOST_VPU_GROUPS_GEN1_GEN2; i++) {
		if (chip->vm_locks[i]) {
			mutex_destroy(chip->vm_locks[i]);
			kfree(chip->vm_locks[i]);
		}
		if (chip->host_thread_semas[i])
			kfree(chip->host_thread_semas[i]);
	}
	kfree(chip->timer);
	vpu_deinit_conf(chip);
err_timer:
	kfree(chip);
	vpu_err("Init Error\n");
	return -ENODEV;
}

#ifdef SOC_MODE
static void vpu_component_unbind(struct platform_device *pdev)
#else
static void vpu_component_unbind(struct device *dev, struct device *master, void *data)
#endif
{
	struct mt_chip *chip;
	struct mt_core *core;
	int idx, i, j;

#ifdef SOC_MODE
	chip = os_platform_get_drvdata(pdev);
	drm_dev_unregister(chip->drms[0]);
	drm_dev_put(chip->drms[0]);
#else
	chip = dev_get_drvdata(dev);
#endif
	del_timer_sync(chip->timer);

	kfree(chip->timer);

#ifdef SUPPORT_SMMU
	vpu_smmu_deinit(chip);
#endif

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
			vpu_hw_reset(chip->conf.core_base + idx);
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
	vpu_free_mem(chip);
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

	vpu_deinit_conf(chip);
	kfree(chip);
}

#ifndef SOC_MODE
static const struct component_ops mtvpu_component_ops = {
	.bind   = vpu_component_bind,
	.unbind = vpu_component_unbind,
};

static struct platform_device_id vpu_id_tbl[] = {
	{ .name = "mtgpu_vde" },
	{}
};
#else
static struct of_device_id vpu_of_id_tbl[] = {
	{ .compatible = "mthreads,apollo-vpu"}
};

static struct acpi_device_id vpu_acpi_id_tbl[] = {
	{.id = "MVPU0001", .driver_data = 0}
};
#endif

static int vpu_probe(struct platform_device *pdev)
{
#ifdef SOC_MODE
	return vpu_component_bind(pdev);
#else
	return component_add(&pdev->dev, &mtvpu_component_ops);
#endif
}

static int vpu_remove(struct platform_device *pdev)
{
#ifdef SOC_MODE
	vpu_component_unbind(pdev);
#else
	component_del(&pdev->dev, &mtvpu_component_ops);
#endif

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
#ifdef SOC_MODE
		.of_match_table = vpu_of_id_tbl,
		.acpi_match_table = vpu_acpi_id_tbl,
#endif
	},
	.probe = vpu_probe,
	.remove = vpu_remove,
#ifndef SOC_MODE
	.id_table = vpu_id_tbl,
#endif
};

#ifdef SOC_MODE
static int __init vpu_init(void)
{
	int ret;
	ret = platform_driver_register(&vpu_driver);

	return ret;
}

static void __exit vpu_exit(void)
{
	platform_driver_unregister(&vpu_driver);
	return;
}

module_init(vpu_init);
module_exit(vpu_exit);

MODULE_DESCRIPTION("MooreThreads mtvpu soc driver");
MODULE_LICENSE("Dual MIT/GPL");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
MODULE_IMPORT_NS(DMA_BUF);
#endif
#endif