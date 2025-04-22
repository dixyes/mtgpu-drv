/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_BO_H_
#define _MTGPU_BO_H_

#define MTGPU_METADATA_MAX_NUM	32

struct _PMR_;

struct mtgpu_bo_metadata {
	u64 id;
	u64 size;
	void *ptr;
	struct list_head node;
};

struct mtgpu_bo_metadata_list {
	struct list_head head;
	struct spinlock *lock;
	u32 total_num;
};

u64 mtgpu_bo_flag_from_pmr_flag(u64 pmr_flags);
u64 mtgpu_bo_flag_to_pmr_flag(u64 bo_flags);
u32 mtgpu_bo_domain_from_pmr_flag(u64 pmr_flags);

void mtgpu_bo_delete_global_handle(u64 global_handle);

bool mtgpu_bo_has_metadata(struct device *dev, PMR *pmr);
int mtgpu_bo_get_metadata(struct device *dev,
			  void *handle,
			  void __user *metadata_addr,
			  u64 metadata_size,
			  u64 metadata_id);
int mtgpu_bo_add_metadata(void *handle,
			  void __user *metadata_addr,
			  u64 metadata_size,
			  u64 metadata_id);
int mtgpu_bo_metadata_list_create(struct device *dev, struct _PMR_ *pmr);
void mtgpu_bo_metadata_list_destroy(struct mtgpu_bo_metadata_list *metadata_list);

#endif /* _MTGPU_BO_H_ */
