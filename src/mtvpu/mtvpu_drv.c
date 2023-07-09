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
#include <drm/drm_gem.h>
#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#else
#include <drm/drm_ioctl.h>
#include <drm/drm_device.h>
#endif

#include "mtgpu_drv.h"
#include "mtgpu_drm_drv.h"
#include "mtgpu_device.h"
#include "mtgpu_smc.h"

#include "mtvpu_drv.h"
#include "mtvpu_mem.h"
#include "mtvpu_mon.h"
#include "mtvpu_clk.h"
#include "os-interface.h"

#include "misc.h"
#include "mtvpu_pool.h"

#ifndef MT_BUILD_VPU
#define MT_BUILD_VPU ""
#endif

static int mtvpu_component_bind(struct device *dev, struct device *master, void *data)
{
	struct mt_chip *chip = NULL;
	struct drm_device *drm = data;
	struct mtgpu_drm_private *private = drm->dev_private;
	struct platform_device *pdev = to_platform_device(dev);
	struct pci_dev *pcid;
	struct mtgpu_device *mtdev;
	ulong offset = 0;
	char name[32];
	u32 core_bits = 0;
	int dec_bit_offset = 0;
	int enc_bit_offset = 8;
	int boda_bit_offset = 14;
	int i, j, ret;
	struct file_operations *vinfo, *fwinfo;
	bool is_host = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_HOST;
	bool is_guest = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_GUEST;
	bool is_native = mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_NATIVE;

	pr_info("Init VPU %s, Version %s\n", is_native ? "Native" : "Host", MT_BUILD_VPU);
	chip = kzalloc(sizeof(struct mt_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->parent = dev->parent->parent;
	chip->dev = dev;
	chip->drm = drm;
	private->chip = chip;

	mtdev = dev_get_drvdata(chip->parent);
	if (unlikely(!mtdev)) {
		pr_err("Failed to get mtdev\n");
		goto err_timer;
	}
	/* fetch data from mtdev directly */
	/* fix this if mtgpu_video_platform_data is used to pass parameters */
	chip->idx = mtdev->dev_id;

	chip->timer = malloc_mt_timer();
	if (!chip->timer)
		goto err_timer;

	pcid = to_pci_dev(chip->parent);
	chip->pdev = pcid;

	vpu_init_conf(pcid->device, chip, is_guest);

	chip->host_thread_sema = kzalloc(sizeof(*chip->host_thread_sema), GFP_KERNEL);
	if (!chip->host_thread_sema)
		goto err_host_thread_sema;
	sema_init(chip->host_thread_sema, 0);

	chip->vm_lock = kzalloc(sizeof(*chip->vm_lock), GFP_KERNEL);
	if (!chip->vm_lock)
		goto err_vm_lock;
	mutex_init(chip->vm_lock);

	chip->vm_lock_group3 = kzalloc(sizeof(*chip->vm_lock_group3), GFP_KERNEL);
	if (!chip->vm_lock_group3)
		goto err_vm_lock_group3;
	mutex_init(chip->vm_lock_group3);

	chip->vm_sema = kzalloc(sizeof(*chip->vm_sema), GFP_KERNEL);
	if (!chip->vm_sema)
		goto err_vm_sema;
	sema_init(chip->vm_sema, 0);

	chip->mem_lock = kzalloc(sizeof(*chip->mem_lock), GFP_KERNEL);
	if (!chip->mem_lock)
		goto err_mem_lock;
	spin_lock_init(chip->mem_lock);

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
			return -ENOMEM;

		init_waitqueue_head(chip->sync.addr_wait[i]);
	}

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++) {
				chip->sync.inst_wait[i][j] =
					kzalloc(sizeof(struct wait_queue_head), GFP_KERNEL);
				if (!chip->sync.inst_wait[i][j])
					return -ENOMEM;

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

		if (chip->conf.product[i] == WAVE517_CODE)
			for (j = 0; j < INST_MAX_SIZE; j++) {
				chip->core[i].inst_lock[j] =
					kzalloc(sizeof(*chip->core[i].inst_lock[j]), GFP_KERNEL);
				if (!chip->core[i].inst_lock[j])
					goto err_lock;
				mutex_init(chip->core[i].inst_lock[j]);
			}
	}

	INIT_LIST_HEAD(&chip->vm_head);
	INIT_LIST_HEAD(&chip->vm_head_group3);

	if (is_native || is_host) {
		chip->drm_dev_cnt = mtgpu_get_primary_core_count(mtdev);
		chip->mem_group_cnt = mtdev->video_group_cnt;
		for (i = 0, j = 0; i < MTGPU_CORE_COUNT_MAX; i++) {
			if (mtdev->drm_dev[i]) {
				chip->drm_dev[j] = os_dev_get_drvdata(&mtdev->drm_dev[i]->dev);
				if (!chip->drm_dev[j])
					goto err;
#ifdef SUPPORT_ION
				chip->ion_dev[j] = mtdev->ion_dev[j];
#endif
				chip->drm_video_group_idx[j] = mtdev->video_group_idx[j];
				private = chip->drm_dev[j]->dev_private;
				private->chip = chip;
				j++;
			}
		}
		if (j != chip->drm_dev_cnt)
			goto err;
	}

	ret = mtgpu_smc_get_vpu_core_info(chip->parent, &core_bits);
	if (ret)
		core_bits = 0;

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			if ((core_bits >> dec_bit_offset) & 0x1) {
				pr_info("dec core:%d offset:%d Not available\n", i, dec_bit_offset);
				chip->core[i].available = 0;
			} else {
				chip->core[i].available = 1;
				chip->core[i].drm_dev = drm;
				chip->core[i].product_id = WAVE517_CODE;
				pr_info("dec core:%d offset:%d Is available\n", i, dec_bit_offset);
			}
			dec_bit_offset++;
		} else if (chip->conf.product[i] == WAVE627_CODE) {
			if ((core_bits >> enc_bit_offset) & 0x1) {
				pr_info("enc core:%d offset:%d Not available\n", i, enc_bit_offset);
				chip->core[i].available = 0;
			} else {
				chip->core[i].available = 1;
				chip->core[i].drm_dev = drm;
				chip->core[i].product_id = WAVE627_CODE;
				pr_info("enc core:%d offset:%d Is available\n", i, enc_bit_offset);
			}
			enc_bit_offset++;
		} else if (chip->conf.product[i] == BODA950_CODE) {
			if ((core_bits >> boda_bit_offset) & 0x1) {
				pr_info("dec core:%d offset:%d Not available\n", i, boda_bit_offset);
				chip->core[i].available = 0;
			} else {
				chip->core[i].available = 1;
				chip->core[i].drm_dev = drm;
				chip->core[i].product_id = BODA950_CODE;
				pr_info("dec core:%d offset:%d Is available\n", i, boda_bit_offset);
			}
		}

		for (j = 0; j < INST_MAX_SIZE; j++) {
			INIT_LIST_HEAD(&chip->core[i].mm_head[j]);
		}
	}

	chip->vaddr = ioremap(pci_resource_start(pcid, 0) + chip->conf.regs_base, chip->conf.regs_chip_size);
	if (!chip->vaddr)
		goto err;

	chip->bar_base = pci_resource_start(pcid, 2);

	for (i = 0; i < chip->conf.core_size; i++) {
		chip->core[i].regs = chip->vaddr + offset;
		offset += chip->conf.regs_core_size;
	}

	if (is_native) {
		ret = vpu_init_segment(chip);
		if (ret)
			goto err;

		ret = vpu_assign_segment_to_core(chip);
		if (ret)
			goto err;

		ret = vpu_assign_heap_to_core(chip);
		if (ret)
			goto err;
	}

	if (is_guest_cmds)
		vpu_init_guest_mem(chip);

	ret = vpu_init_irq(chip, pdev);
	if (ret)
		goto err;

	platform_set_drvdata(pdev, chip);

	ret = vpu_fill_drm_ioctls(pvr_drm_ioctls, 128);
	if (ret)
		goto err;

	ret = vpu_chip_add(chip);
	if (ret < 0)
		goto err;

	sprintf(name, "vpu-sync/%d", chip->idx);
	chip->sync_thread = kthread_create(vpu_sync_thread, chip, name);
	wake_up_process(chip->sync_thread);

	if (chip->conf.type == TYPE_QUYU2) {
		if (is_host)
			vpu_set_vm_core(chip);
		if (is_host || is_guest) {
			for (i = 0; i < chip->conf.core_size; i++) {
				if (chip->conf.product[i] != BODA950_CODE)
					vpu_load_firmware(chip, i, NULL);
			}
		}
	} else if (is_host) {
		sprintf(name, "vpu-host/%d", chip->idx);
		chip->host_thread = kthread_create(vpu_host_thread, chip, name);
		wake_up_process(chip->host_thread);

		chip->workqueue = alloc_workqueue("vpu-work", WQ_HIGHPRI | WQ_UNBOUND, 16);
	}

	vinfo = get_vinfo_fops();
	fwinfo = get_fwinfo_fops();
	sprintf(name, "mtvpu%d", chip->idx);
	chip->debugfs = debugfs_create_dir(name, NULL);
	if (chip->debugfs) {
		debugfs_create_file("info", 0444, chip->debugfs, chip, vinfo);
		debugfs_create_file("fw", 0444, chip->debugfs, chip, fwinfo);
	}

	set_mt_timer_data(chip->timer, chip);
	timer_setup(chip->timer, vpu_monitor, 0);
	mod_timer(chip->timer, jiffies + msecs_to_jiffies(1000));

	for (i = 0; i < chip->conf.core_size; i++) {
		struct mt_core *core = &chip->core[i];

		core->core_freq = mtvpu_get_clk(chip, i);
	}

	for (i = 0; i < chip->conf.core_size; i++)
		if (chip->conf.product[i] == WAVE627_CODE)
			mtvpu_slice_mode_config(chip, i);

	return 0;
err:
err_lock:
	for (i = 0; i < SYNC_ADDR_SIZE; i++)
		kfree(chip->sync.addr_wait[i]);

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++)
				kfree(chip->sync.inst_wait[i][j]);
		}
	}

	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
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
err_core_lock:
	kfree(chip->pool_lock);
	for (i = 0; i < CORE_MAX_SIZE; i++)
		if (chip->sync.core_lock[i])
			kfree(chip->sync.core_lock[i]);
err_pool_lock:
	kfree(chip->sync.sync_lock);
err_sync_lock:
	kfree(chip->sync.intr_lock);
err_intr_lock:
	mutex_destroy(chip->mm_lock);
	kfree(chip->mm_lock);
err_mm_lock:
	kfree(chip->mem_lock);
err_mem_lock:
	kfree(chip->vm_sema);
err_vm_sema:
	mutex_destroy(chip->vm_lock_group3);
	kfree(chip->vm_lock_group3);
err_vm_lock_group3:
	mutex_destroy(chip->vm_lock);
	kfree(chip->vm_lock);
err_vm_lock:
	kfree(chip->host_thread_sema);
err_host_thread_sema:
	kfree(chip->timer);
err_timer:
	kfree(chip);
	pr_err("VPU Init Error\n");
	return -ENODEV;
}

static void mtvpu_component_unbind(struct device *dev, struct device *master, void *data)
{
	struct mt_chip *chip = dev_get_drvdata(dev);
	struct mt_core *core;
	int idx;
	int i,j;

	del_timer_sync(chip->timer);

	kfree(chip->timer);

	if (chip->debugfs)
		debugfs_remove_recursive(chip->debugfs);

	if (chip->host_thread)
		kthread_stop(chip->host_thread);

	if (chip->group3_thread)
		kthread_stop(chip->group3_thread);

	if (chip->sync_thread)
		kthread_stop(chip->sync_thread);

	for (idx = 0; idx < chip->conf.core_size; idx++) {
		core = &chip->core[idx];

		if (core->bak_addr)
			vfree(core->bak_addr);

		if (!core->fw)
			continue;

		mtvpu_hw_reset(chip->conf.core_base + idx);
		mtvpu_vpu_deinit(chip->conf.core_base + idx);

		release_firmware(core->fw);

		if (chip->core[idx].open_lock) {
			mutex_destroy(chip->core[idx].open_lock);
			kfree(chip->core[idx].open_lock);
		}
		if (chip->core[idx].regs_lock) {
			mutex_destroy(chip->core[idx].regs_lock);
			kfree(chip->core[idx].regs_lock);
		}
	}

	vpu_free_irq(chip);
	vpu_free_mem(chip);
	vpu_destroy_all_pools(chip);
	kfree(chip->sync.sema);
	kfree(chip->sync.sync_lock);
	kfree(chip->pool_lock);

	for (i = 0; i < SYNC_ADDR_SIZE; i++)
		kfree(chip->sync.addr_wait[i]);
	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
			for (j = 0; j < INST_MAX_SIZE; j++)
				kfree(chip->sync.inst_wait[i][j]);
		}
	}
	for (i = 0; i < chip->conf.core_size; i++) {
		if (chip->conf.product[i] == WAVE517_CODE) {
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
	mutex_destroy(chip->vm_lock);
	kfree(chip->vm_lock);
	mutex_destroy(chip->vm_lock_group3);
	kfree(chip->vm_lock_group3);
	mutex_destroy(chip->mm_lock);
	kfree(chip->mm_lock);
	kfree(chip->vm_sema);
	kfree(chip->host_thread_sema);

	vpu_chip_del(chip);
	kfree(chip);
}

static const struct component_ops mtvpu_component_ops = {
	.bind   = mtvpu_component_bind,
	.unbind = mtvpu_component_unbind,
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
	.suspend = mtvpu_suspend,
	.resume  = mtvpu_resume,
	.freeze  = mtvpu_suspend,
	.restore = mtvpu_resume,
};

static struct platform_device_id vpu_id_tbl[] = {
	{ .name = "mtgpu_vde" },
	{}
};

struct platform_driver vpu_driver = {
	.driver = {
		.name = "mtgpu_vde",
		.pm = &vpu_pm_ops,
	},
	.probe = vpu_probe,
	.remove = vpu_remove,
	.id_table = vpu_id_tbl,
};
