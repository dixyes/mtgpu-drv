/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTJPU_COMMON_H__
#define __MTJPU_COMMON_H__

struct kfifo;
struct semaphore;
struct drm_device;
struct mtjpu_device;
struct drm_ioctl_desc;
struct mtgpu_device;
struct mtjpu_device;
struct mtjpu_core;

struct mtjpu_device *drm_dev_to_jpu_device(struct drm_device *drm);
void mtjpu_device_init(struct mtjpu_device *jpu_dev, int idx);
void mtjpu_device_uninit(int idx);
void mtjpu_core_init(struct mtjpu_core *jpu_core, unsigned long long pcie_io_base);
void mtjpu_core_uninit(struct mtjpu_core *jpu_core);
struct mtjpu_core *jpu_alloc_core(struct mtjpu_device *jpu_dev);
int jpu_free_core(struct mtjpu_device *dev, int idx);
int mtjpu_kfifo_init(struct kfifo **fifo, int num);
void mtjpu_kfifo_free(struct kfifo *fifo);
int mtjpu_core_sema_init(struct semaphore **sema, int count);
void mtjpu_core_sema_free(struct semaphore *sema);
int mtjpu_device_lock_init(struct mtjpu_device *jpu_dev);
void mtjpu_device_lock_uninit(struct mtjpu_device *jpu_dev);
int mtjpu_fill_drm_ioctls(struct drm_ioctl_desc *dst, int num);
void mtjpu_device_jpu_init(struct mtjpu_device *jpu_dev, struct device *dev,
			   struct drm_device *drm, struct mtgpu_device *mtdev,
			   struct platform_device *plat_dev);

#endif /* __MTJPU_COMMON_H__ */
