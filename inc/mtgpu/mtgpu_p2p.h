/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_P2P_H__
#define __MTGPU_P2P_H__

#define MTGPU_P2P_MAJOR_VERSION_OFFSET 16
#define MTGPU_P2P_MAJOR_VERSION_MASK 0xffff0000
#define MTGPU_P2P_MINOR_VERSION_MASK 0x0000ffff

#define MTGPU_P2P_MAJOR_VERSION(v) \
	(((v) & MTGPU_P2P_MAJOR_VERSION_MASK) >> MTGPU_P2P_MAJOR_VERSION_OFFSET)

#define MTGPU_P2P_MINOR_VERSION(v) \
	(((v) & MTGPU_P2P_MINOR_VERSION_MASK))

#define MTGPU_P2P_VERSION(major, minor) \
	(((major) << MTGPU_P2P_MAJOR_VERSION_OFFSET) | (minor))

#define MTGPU_P2P_PAGE_TABLE_VERSION  MTGPU_P2P_VERSION(1, 0)
#define MTGPU_P2P_DMA_MAPPING_VERSION MTGPU_P2P_VERSION(1, 0)

struct mtgpu_p2p_page_table {
	u32 version;
	u32 page_size;
	u64 *cpu_pa_array;
	u32 array_count;
};

struct mtgpu_dma_mapping {
	u32 version;
	u64 *dma_address_array;
	u64 *dma_length_array;
	u32 array_count;
};

/**
 * mtgpu_p2p_get_pages - Get the pages that map to a range of GPU virtual memory
 *
 * @gpu_va:		The gpu virtual address.
 * @length:		The length of the requested p2p mapping.
 * @page_table_ptr:	A pointer to an array of structures with p2p ptes.
 *
 * Search physical memory resource mapped to the virtual address, return 0 and
 * p2p page table on success. Returns -EINVAL if an invalid argument was supplied,
 * -ENOMEM if the driver failed to allocate memory or if insufficient resources were
 * available to complete the operation.
 */
int mtgpu_p2p_get_pages(u64 gpu_va, u64 length, struct mtgpu_p2p_page_table **page_table_ptr);

/**
 * mtgpu_p2p_put_pages - Release a set of pages previously made accessible to
 * a third-party device.
 *
 * @gpu_va:		The gpu virtual address.
 * @page_table:		A pointer to the array of structures with p2p ptes.
 *
 * Free all resources allocated by get pages above, return 0 on success.
 */
int mtgpu_p2p_put_pages(u64 gpu_va, struct mtgpu_p2p_page_table *page_table);

/**
 * mtgpu_p2p_dma_map_pages - Map the physical pages retrieved by get pages.
 *
 * @dma_device:		The dma device.
 * @page_table:		A pointer to the array of structures with p2p ptes.
 * @dma_mapping:	A pointer to the array of structures with dma mappings.
 *
 * This function is dummy and 0 is returned.
 */
int mtgpu_p2p_dma_map_pages(struct device *dma_device,
			    struct mtgpu_p2p_page_table *page_table,
			    struct mtgpu_dma_mapping **dma_mapping);

/**
 * mtgpu_p2p_dma_unmap_pages - Unmap the physical pages
 *
 * @dma_device:		The dma device.
 * @dma_mapping:	A pointer to the array of structures with dma mappings.
 *
 * This function is dummy and 0 is returned.
 */
int mtgpu_p2p_dma_unmap_pages(struct device *dma_device, struct mtgpu_dma_mapping *dma_mapping);

#endif /* __MTGPU_P2P_H__ */

