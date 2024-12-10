/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_DMA_H__
#define __MTGPU_DMA_H__

#define DMA_COMM_DEVICE_SELF 0x8 /* card in card */
#define DMA_COMM_HOST_SELF 0x2  /* host and host */
#define DMA_COMM_DEVICE_AND_HOST 0x4 /* device and host */
#define DMA_COMM_DEVICE_AND_DEVICE 0x6 /* p2p */

#define DMA_WRITE 0
#define DMA_READ 1

#define MTGPU_DMA_DIR_MASK	0x1
#define MTGPU_DMA_DIR_WR	0
#define MTGPU_DMA_DIR_RD	0x1

enum mtgpu_dma_xfer_type {
	MTGPU_DMA_XFER_TYPE_BULK = 0,
	MTGPU_DMA_XFER_TYPE_SCATTER,
	MTGPU_DMA_XFER_TYPE_GATHER,
	MTGPU_DMA_XFER_TYPE_CNT,
};

enum mtgpu_dma_xfer_direction {
	MTGPU_DMA_DIR_S2D = 0,
	MTGPU_DMA_DIR_D2S,
	MTGPU_DMA_DIR_S2S,
	MTGPU_DMA_DIR_D2D,
	MTGPU_DMA_DIR_CNT,
};

enum mtgpu_dma_chan {
	MTGPU_DMA_CHAN0 = 0,
	MTGPU_DMA_CHAN1,
	MTGPU_DMA_CHAN2,
	MTGPU_DMA_CHAN3,
	MTGPU_DMA_CHAN4,
	MTGPU_DMA_CHAN5,
	MTGPU_DMA_CHAN6,
	MTGPU_DMA_CHAN7,
	MTGPU_DMA_CHAN_CNT,
};

struct mtgpu_dma_xfer_desc {
	/* local vram address */
	dma_addr_t device_addr;

	/* extern address(host ram or peer pcie bar address) */
	dma_addr_t system_addr;
	u64 size;
};

struct dma_capability {
	u64 desc_size_limit;
	u64 desc_count_limit;
	u32 desc_mem_size_limit;
	bool is_support_gather;
	bool is_support_scatter;
	u32 size_alignment;
	u32 offset_alignment;
	u32 bus_width;
	u32 addr_alignment;
	bool is_force_addr_aligned;
	bool is_force_size_aligned;
	bool is_support_dma_s2s;
	bool is_support_dma_d2d;
};

struct mtgpu_device;

struct dma_xfer_block {
	u64 dev_addr;
	u64 ext_addr;
	u64 block_size;
	struct page **pages;
	u64 pinned;
};

struct mtgpu_dma_ops {
	int (*init)(struct mtgpu_device *mtdev);
	int (*transmit)(void *dma_info, struct mtgpu_dma_xfer_desc *descs,
			int desc_cnt, int xfer_type, int type, int chan);
	int (*acquire_channel)(struct mtgpu_device *mtdev, int dir, u32 instance_id);
	int (*release_channel)(struct mtgpu_device *mtdev, int dir, int chan);
	void (*resume)(struct mtgpu_device *mtdev);
	int (*get_capabilities)(u32 type, struct dma_capability *dma_cap);
	void (*exit)(struct mtgpu_device *mtdev);
};

extern int mtgpu_dma_debug;

int mtgpu_dma_transfer(struct device *master, struct device *slave,
		       struct dma_xfer_block *xfer_blocks,
		       u32 block_nums,
		       u32 xfer_type);

int mtgpu_dma_transfer_user(struct device *drm_dev, u64 device_addr, void *system_vaddrs,
			    u64 size, bool mem_to_dev);

int mtgpu_dma_transfer_kernel(struct device *drm_dev, u64 device_addr, u64 *system_vaddrs,
			      u64 size, bool mem_to_dev);

int mtgpu_dma_transfer_sparse_user(struct device *drm_dev,
				   u64 *device_addrs, u64 *system_vaddrs,
				   u64 size, bool *valids, u32 offset_in_first_page,
				   u32 num_pages, u32 num_valid_pages, bool mem_to_dev);

void mtgpu_dma_chan_free(struct device *dev, void *chandata);

struct dma_chan *mtgpu_dma_chan(struct device *dev, char *name);
int mtgpu_dma_init(struct mtgpu_device *mtdev);
void mtgpu_dma_exit(struct mtgpu_device *mtdev);

#endif
