/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_DMA_H__
#define __MTGPU_DMA_H__

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
};

struct mtgpu_device;

struct mtgpu_dma_ops {
	int (*init)(struct mtgpu_device *mtdev);
	int (*transmit)(void *dma_info, struct mtgpu_dma_xfer_desc *descs,
			int desc_cnt, int direction, int type, int chan);
	int (*get_capabilities)(u32 type, struct dma_capability *dma_cap);
	void (*exit)(struct mtgpu_device *mtdev);
};

extern int mtgpu_dma_debug;

int mtgpu_dma_transfer_user(struct device *drm_dev, u64 device_addr, void *system_vaddrs,
			    u64 size, bool mem_to_dev);

int mtgpu_dma_transfer_kernel(struct device *drm_dev, u64 device_addr, u64 *system_vaddrs,
			      u64 size, bool mem_to_dev);

int mtgpu_dma_transfer_sparse_user(struct device *drm_dev,
				   u64 *device_addrs, u64 *system_vaddrs,
				   u64 size, bool *valids, u32 offset_in_first_page,
				   u32 num_pages, u32 num_valid_pages, bool mem_to_dev);

int mtgpu_dma_transfer_p2p(struct device *local_dev, struct device *peer_dev,
			   u64 *local_address, u64 *peer_address,
			   u64 *size, int num, bool ispeer2local);

void mtgpu_dma_chan_free(struct device *dev, void *chandata);

struct dma_chan *mtgpu_dma_chan(struct device *dev, char *name);
int mtgpu_dma_init(struct mtgpu_device *mtdev);
void mtgpu_dma_exit(struct mtgpu_device *mtdev);

#endif
