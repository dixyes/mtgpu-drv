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
struct mtvpu_gem_object;

#define vpu_warn(fmt, ...) os_pr_warn("[mtvpu] " fmt, ##__VA_ARGS__)
#define vpu_info(fmt, ...) os_pr_info("[mtvpu] " fmt, ##__VA_ARGS__)
#define vpu_err(fmt, ...) os_pr_err("[mtvpu] " fmt, ##__VA_ARGS__)

extern struct platform_driver vpu_driver;
extern bool is_guest_cmds;

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
struct file_operations *get_vinfo_fops(void);
struct file_operations *get_fwinfo_fops(void);

int vpu_vram_alloc(struct drm_device *drm, u32 group_id, u32 pool_id, u32 type, u64 size,
		   struct mtgpu_gem_object *mtgpu_obj);
void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj);
int vpu_gem_modify(struct drm_device *drm, struct mtgpu_gem_object *mtgpu_obj, u32 group_id, u32 inc_size, u32 copy);

int mtvpu_vram_alloc(struct drm_device *drm, u32 group_id, size_t size,
		     dma_addr_t *dev_addr, void **virt_addr, dma_addr_t *iova_addr, void **handle);
void mtvpu_vram_free(struct drm_gem_object *obj, dma_addr_t dev_addr, void *virt_addr, dma_addr_t iova_addr, void *handle);

int mtvpu_drm_open(struct drm_device *drm, struct drm_file *file);
void mtvpu_drm_release(struct drm_device *drm, struct drm_file *file);
void mtvpu_gem_free_obj(struct drm_gem_object *obj);
struct sg_table *vpu_gem_map_internal(void *handle, size_t size);
void vpu_gem_unmap_internal(struct sg_table *sgt);
void *vpu_gem_vmap_internal(void *handle, u64 size, u64 *private_data);
void vpu_gem_vunmap_internal(void *handle, u64 private_data);
void dcache_flush(void *addr, size_t len);
int mtvpu_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma);
int mtvpu_gem_dmabuf_map(struct sg_table *sgt, struct mtgpu_gem_object *mtgpu_obj);

int mtvpu_job_submit(struct drm_device *drm, struct drm_file *file_priv, void __user *data, uint32_t size);
int mtvpu_bo_wait(struct drm_device *drm, struct drm_file *file_priv, u64 bo_handle, u32 offset, u32 flags, s64 timeout_ns);

#ifdef __aarch64__
u64 os_kernel_ds(void);
u64 os_get_fs(void);
void os_set_fs(u64 fs);
long os_vfs_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif

#endif /* _MTVPU_API_H_ */
