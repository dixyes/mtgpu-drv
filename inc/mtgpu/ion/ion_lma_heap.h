/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __ION_LMA_HEAP__
#define __ION_LMA_HEAP__

struct ion_device;
struct ion_platform_heap {
	u32 type;
	unsigned int id;
	const char *name;
	phys_addr_t vram_base;
};

struct ion_heap *ion_lma_heap_create(struct ion_platform_heap *heap_data, bool allow_cpu_map);
void ion_lma_heap_destroy(struct ion_heap *heap);
void ion_heap_private_init(struct ion_device *idev, struct device *dev);

#endif /* __ION_LMA_HEAP__ */
