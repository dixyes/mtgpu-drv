/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_API_H_
#define _MTVPU_API_H_

#include "linux-types.h"

struct mt_node;
struct mt_file;
struct mt_virm;
struct file;
struct device;
struct timer_list;
struct drm_device;
struct drm_file;
struct drm_gem_object;
struct drm_mode_create_dumb;
struct vm_area_struct;
struct mtgpu_gem_object;
struct mtgpu_codec_priv_data;
struct devfreq;
struct devfreq_simple_ondemand_data;
struct devfreq_dev_profile;
struct devfreq_dev_status;
struct dev_pm_opp;

#define vpu_warn(fmt, ...) os_pr_warn("[mtvpu] " fmt, ##__VA_ARGS__)
#define vpu_info(fmt, ...) os_pr_info("[mtvpu] " fmt, ##__VA_ARGS__)
#define vpu_err(fmt, ...) os_pr_err("[mtvpu] " fmt, ##__VA_ARGS__)

extern struct platform_driver vpu_driver;

struct mt_chip *to_chip(struct drm_device *drm);

struct mt_file *os_get_drm_file_private_data(struct drm_file *file);
void os_set_drm_file_private_data(struct drm_file *file, struct mt_file *priv);
unsigned long os_memremap_wb(void);
void *os_memremap(resource_size_t offset, size_t size, unsigned long flags);
void os_memunmap(void *addr);

void set_mtgpu_obj_addr(struct mtgpu_gem_object *mtgem_obj, u64 base, u64 dev_addr);
void set_mtgpu_obj_type(struct mtgpu_gem_object *mtgem_obj, u32 group_id, u32 pool_id);
int get_mtgpu_obj_type(struct mtgpu_gem_object *mtgem_obj, u32 *group_id, u32 *pool_id);

struct mtgpu_gem_object *alloc_mtgpu_obj(void);

int vpu_get_group_id(struct drm_device *drm);
int vpu_get_drm_id(struct drm_device *drm);
bool vpu_drm_core_valid(struct mt_chip *chip, struct drm_device *drm, u32 core_idx);

void *vpu_get_pvr_node(struct drm_device *drm);

ssize_t vpu_info_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t fw_info_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t vpu_info_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
ssize_t vpu_log_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
struct file_operations *get_vinfo_fops(void);
struct file_operations *get_fwinfo_fops(void);
struct file_operations *get_vpulog_fops(void);

int vpu_vram_alloc(struct drm_device *drm, u32 group_id, u32 pool_id, u32 type, u64 size,
		   struct mtgpu_gem_object *mtgpu_obj);
void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj);
int vpu_gem_modify(struct drm_device *drm, struct mtgpu_gem_object *mtgpu_obj, u32 group_id);

int mtvpu_vram_alloc(struct drm_device *drm, u32 group_id, size_t size,
		     dma_addr_t *dev_addr, void **handle);

int mtvpu_drm_open(struct drm_device *drm, struct drm_file *file);
void mtvpu_drm_release(struct drm_device *drm, struct drm_file *file);
void mtvpu_gem_free_obj(struct drm_gem_object *obj);
struct sg_table *vpu_gem_map_internal(void *handle, size_t size);
void vpu_gem_unmap_internal(struct sg_table *sgt);
void dcache_flush(void *addr, size_t len);
int vpu_get_core_power(struct device *dev, int *power);
int vpu_set_core_power(struct device *dev, int power);

int mtvpu_job_submit(struct drm_device *drm, struct drm_file *file_priv, void __user *data,
		     uint32_t size, struct mtgpu_codec_priv_data *data_priv);
int mtvpu_bo_wait(struct drm_device *drm, struct drm_file *file_priv, u64 bo_handle, u32 offset, u32 flags, s64 timeout_ns);

struct devfreq *os_devfreq_add_device(struct device *dev,
				   struct devfreq_dev_profile *profile,
				   const char *governor_name,
				   void *data);
void os_devfreq_remove_device(struct device *dev, struct devfreq *devfreq);
int os_dev_pm_opp_add(struct device *dev, unsigned long freq, unsigned long u_volt);
void os_dev_pm_opp_remove(struct device *dev, unsigned long freq);
unsigned long os_dev_pm_opp_get_freq(struct dev_pm_opp *opp);
void os_dev_pm_opp_put(struct dev_pm_opp *opp);
int os_devfreq_register_opp_notifier(struct device *dev, struct devfreq *devfreq);
int os_devfreq_unregister_opp_notifier(struct device *dev,
				struct devfreq *devfreq);
struct dev_pm_opp *os_devfreq_recommended_opp(struct device *dev,
					   unsigned long *freq,
					   u32 flags);
int vpu_devfreq_target(struct device *dev, unsigned long *freq, u32 flags);
int vpu_devfreq_get_dev_status(struct device *dev, struct devfreq_dev_status *stat);
int vpu_devfreq_cur_freq(struct device *dev, unsigned long *freq);
void vpu_devfreq_set_dev_status(struct mt_chip *chip, struct devfreq_dev_status *stat);

int m1000_set_freq(struct mt_chip *chip, int idx, unsigned long freq);
void vpu_init_devfreq_profile(u64 freq, u32 poll_ms);
int	vpu_init_devfreq_data(struct mt_chip *chip);
void vpu_init_devfreq_freq(struct mt_chip *chip, u64 min_freq, u64 max_freq);
int vpu_sema_cmd_proc(u64 type, u64 *data, struct drm_file *file);
void vpu_sema_release(struct drm_file *file);

struct mt_vpu_cmd_que;
struct CodecInst;
struct spinlock;
int vpu_create_que(struct mt_vpu_cmd_que *que, struct spinlock *lock, struct CodecInst *handle, struct mt_virm *vm);
void vpu_destroy_que(struct mt_vpu_cmd_que *que);

struct work_struct;
void* vpu_create_irq_works(int num, void *ctx);
void vpu_destroy_irq_works(void *works, int num);
struct work_struct* vpu_irq_work_get_work(void *works, int idx);

void* vpu_irq_work_get_ctx(struct work_struct *work);
struct mt_intr_map* vpu_irq_work_get_map(struct work_struct *work);
#endif /* _MTVPU_API_H_ */
