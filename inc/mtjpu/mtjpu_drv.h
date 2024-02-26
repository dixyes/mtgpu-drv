/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTJPU_DRV_H__
#define __MTJPU_DRV_H__

#include "mtjpu_config.h"

struct kfifo;
struct semaphore;
struct drm_device;
struct device;
struct pci_dev;
struct platform_device;
struct spinlock;

struct mtjpu_core {
	int idx;
	unsigned long long reg_phys_base;
	unsigned long long reg_kernel_base;
};

struct mtjpu_device {
	unsigned int idx;
	unsigned long pcie_io_base;
	unsigned long pcie_mem_base;
	struct pci_dev *pcie_dev;
	struct device *parent;
	struct device *dev;
	struct drm_device *drm;
	struct platform_device *plat_dev;
	struct mtjpu_core jpu_core[MAX_NUM_JPU_CORE];
	struct semaphore *core_sema;
	struct kfifo *fifo;
	struct spinlock *in_fifo_lock;
	struct spinlock *out_fifo_lock;
};

extern struct platform_driver jpu_driver;

#endif /* __MTJPU_DRV_H__ */
