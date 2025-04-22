/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */
#ifndef __VGPU_TYPES_H__
#define __VGPU_TYPES_H__

#include "linux-types.h"

/*
 * The first bit indicates the number of virtual displays;
 * The Second bit indicates the resolution type:  1 --- 1K; 2 --- 2k; 4 --- 4K;
 * The last 2 bits indicate the size of the vram memory of the vgpu type.
 *	Note: When the video memory exceeds 100G, there are some differences:
 *		128GB --- a2
 *		160GB --- a6
 *		192GB --- a9
 *		256GB --- b5
 *		512GB --- e1
 */
#define MTGPU_VGPU_TYPE_1100      "1100"
#define MTGPU_VGPU_TYPE_1101      "1101"
#define MTGPU_VGPU_TYPE_1102      "1102"
#define MTGPU_VGPU_TYPE_1104      "1104"
#define MTGPU_VGPU_TYPE_1108      "1108"
#define MTGPU_VGPU_TYPE_1116      "1116"
#define MTGPU_VGPU_TYPE_1132      "1132"

/* RESOLUTION */
#define MTGPU_VGPU_FHD    "1920x1080"      /* 1080P */
#define MTGPU_VGPU_WUXGA  "1920x1200"
#define MTGPU_VGPU_QHD    "2560x1440"      /* 2K    */
#define MTGPU_VGPU_WQXGA  "2560x1600"
#define MTGPU_VGPU_UHD    "3840x2160"      /* 4K    */
#define MTGPU_VGPU_DCI4K  "4096x2160"
#define MTGPU_VGPU_8K_UHD "7680x4320"	   /* 8K    */

struct pre_alloc_array {
	u32 mz_cnt;
	u32 instance_cnt;
	u64 **array;
};

struct mtgpu_vgpu_capacity {
	char *capacity;
	char name[20];
	char *max_resolution;     /* virtual display max resolution */
	u64 ddr_size;
	u32 virtual_display_num;  /* max virtual display num */
	u32 max_encode_num;       /* video encode num base 1080P resolution */
	u32 max_decode_num;       /* video decode num base 1080P resolution */
	u32 max_cnt_in_prd;
	u32 current_cnt;
	u64 actual_size;
	u32 vpu_vcpu_mem_size;
	u32 is_support_hdr;
	struct pre_alloc_array *pre_alloc_array;
};

struct mtgpu_vgpu_prd {
	char *name;
	u32 max_cnt;
};

typedef enum {
	SUDI_TYPES_GROUP,
	QUYUAN1_16G_TYPES_GROUP,
	QUYUAN1_16G_SRIOV_TYPES_GROUP,
	QUYUAN1_TYPES_GROUP,
	QUYUAN1_SRIOV_TYPES_GROUP
} mtgpu_vgpu_types_group;

extern struct mtgpu_vgpu_capacity mtgpu_vgpu_capacities[];

u32 get_vgpu_capacities_num(void);
int get_vgpu_capacity_resolution(const char *max_resolution, u32 *width, u32 *height);
u32 get_vgpu_prd(mtgpu_vgpu_types_group types, const struct mtgpu_vgpu_prd **prd);
bool vgpu_type_acquire(const struct mtgpu_vgpu_capacity *capacity);
void vgpu_type_release(const struct mtgpu_vgpu_capacity *capacity);
int vgpu_types_update_alloc_array(struct mtgpu_vgpu_capacity *capacity, void *opaque,
				  u64 granularity);
void vgpu_types_free_alloc_array(struct mtgpu_vgpu_capacity *capacity);
bool vgpu_types_is_type_of(const struct mtgpu_vgpu_capacity *capacity, const char *name);
u64 get_max_vgpu_ddr_size_of_types_group(mtgpu_vgpu_types_group types);
#endif /* __VGPU_TYPES_H__ */
