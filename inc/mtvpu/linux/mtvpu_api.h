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

extern struct platform_driver vpu_driver;
extern bool is_guest_cmds;

struct mt_chip *to_chip(struct drm_device *drm);

struct mt_file *os_get_drm_file_private_data(struct drm_file *file);
void os_set_drm_file_private_data(struct drm_file *file, struct mt_file *priv);

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
struct file_operations *get_vinfo_fops(void);
struct file_operations *get_fwinfo_fops(void);

int vpu_vram_alloc(struct drm_device *drm, u32 group_id, u32 pool_id, u32 type, u64 size, struct mtgpu_gem_object *mtgpu_obj);
void vpu_vram_free(struct mtgpu_gem_object *mtgpu_obj);

int vpu_gem_modify(struct drm_device *drm, struct mtgpu_gem_object *mtgpu_obj, u32 group_id, u32 inc_size, u32 copy);

int mtvpu_drm_open(struct drm_device *drm, struct drm_file *file);
void mtvpu_drm_release(struct drm_device *drm, struct drm_file *file);
int mtvpu_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma);
void mtvpu_gem_free_obj(struct drm_gem_object *obj);

int vpu_init_guest_capibility(struct mt_chip *chip);
#endif /* _MTVPU_API_H_ */
