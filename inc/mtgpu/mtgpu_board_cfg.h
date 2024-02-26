/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_BOARD_CFG_H__
#define __MTGPU_BOARD_CFG_H__

#include "os-interface.h"
#include "mtgpu_ipc.h"

#define PORT_DISABLED		0
/* DP port */
#define PORT_TYPE_DP		1
#define PORT_TYPE_EDP		2
#define PORT_TYPE_DP2VGA	3
#define PORT_TYPE_DP2HDMI	4
#define PORT_TYPE_DP2LVDS	5
/* HDMI port */
#define PORT_TYPE_HDMIA		1
#define PORT_TYPE_HDMIA2VGA	2
#define PORT_TYPE_HDMIA2LVDS	3
#define PORT_TYPE_HDMIA2DP	4

#define GBYTE	(0x40000000UL)
#define MBYTE	(0x100000UL)

#define BOARD_INFO_ROM_UNIT_MAX_LEN (64)

struct ddr_cap_info {
	u64 ddr_size;	/* Byte */
	u8 llc_type;
	u8 valid_channel_count;
	u8 rsv_1;
	u8 ddr_pcs_capacity;
	u8 ddr_pcs_count;
	u8 ddr_type;
	u16 ddr_frequency;
	u8 rsv[8];
};

struct board_system_cap_info {
	u8 efuse_valid;
	u8 board_type;
	u8 clock_set;
	u8 ipc_type;
	u8 pwm0_type;
	u8 pwm1_type;
	u8 pwm2_type;
	u8 pvt_type;
	u8 rsv[4];
};

struct pcie_cap_info {
	u16 vendor_id;
	u16 device_id_pf0;
	u16 device_id_pf1;
	u8 resize_bar_type;
	u8 aspm_ctrl;
	u8 pcie_int_type_pf0;
	u8 pcie_int_type_pf1;
	u8 pcie_enable_pf0;
	u8 pcie_enable_pf1;
	u8 rsv[4];
};

struct gpu_cap_info {
	u8 gpu_type;
	u8 mc_core_count;
	u8 mc_valid_core;
	u16 gpu_frequency;
	u8 rsv[7];
};

struct disp_cap_info {
	u8 dp0_type;
	u8 dp1_type;
	u8 dp2_type;
	u8 dp3_type;
	u8 hdmi_type;
	u16 disp0_max_hres;
	u16 disp0_max_vres;
	u16 disp0_max_clk;
	u16 disp1_max_hres;
	u16 disp1_max_vres;
	u16 disp1_max_clk;
	u16 disp2_max_hres;
	u16 disp2_max_vres;
	u16 disp2_max_clk;
	u16 disp3_max_hres;
	u16 disp3_max_vres;
	u16 disp3_max_clk;
	u8 rsv[7];
};

struct board_cap_info {
	struct ddr_cap_info ddr_cfg;
	struct board_system_cap_info board_sys;
	struct pcie_cap_info pcie_cfg;
	struct gpu_cap_info mc_cfg;
	struct disp_cap_info disp_cfg;
};

struct board_info_rom_info {
	u8 serial_number[BOARD_INFO_ROM_UNIT_MAX_LEN];
	u8 mem_manufact[BOARD_INFO_ROM_UNIT_MAX_LEN];
};

struct mtgpu_board_configs {
	struct board_cap_info board_cap;
	struct board_info_rom_info board_info_rom;
	bool secure_bit;
	u32 wafer_chip_id;
	u32 wafer_lot_no;
};

struct dp_phy_cfg_hdr {
	u32 cfg_size;
	u32 deemp0_cfg_off : 16;
	u32 deemp0_cfg_cnt : 16;
	u32 deemp35_cfg_off : 16;
	u32 deemp35_cfg_cnt : 16;
	u32 deemp6_cfg_off : 16;
	u32 deemp6_cfg_cnt : 16;
	u32 deemp95_cfg_off : 16;
	u32 deemp95_cfg_cnt : 16;
};

#define DP_CFG_GROUP_MAGIC	0x55AA

struct dp_phy_cfg_grp_hdr {
	u32 cfg_grp_magic : 16;
	u32 cfg_cnt : 16;
	u32 lane0_related : 1;
	u32 lane1_related : 1;
	u32 lane2_related : 1;
	u32 lane3_related : 1;
	u32 rbr_related : 1;
	u32 hbr_related : 1;
	u32 hbr2_related : 1;
	u32 hbr3_related : 1;
	u32 dp_port : 4;
	u32 config_mask_rsv : 18;
	u32 vmargin : 2;
};

struct dp_phy_cfg_reg {
	u32 offset : 28;
	u32 type : 4;
	u32 value;
};

enum dp_phy_cfg_reg_type {
	PHY_MMIO,
	DP_MMIO,
	INDEX_IO,
	FIRMWARE_IO
};

#define BOARD_INFO(dev, fmt, ...)				\
	os_dev_info(dev, "[BOARD][INFO] " fmt, ##__VA_ARGS__)
#define BOARD_ERROR(dev, fmt, ...)				\
	os_dev_err(dev, "[BOARD][ERROR][%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define BOARD_WARN(dev, fmt, ...)				\
	os_dev_warn(dev, "[BOARD][WARN][%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

static inline void mtgpu_board_cfg_ddr_size_convert(u64 *ddr_size)
{
	if (*ddr_size == 129)	/* 512M */
		*ddr_size = 512 * MBYTE;
	else	/* 1, 2, 3 .... G */
		*ddr_size *= GBYTE;
}

static inline int
mtgpu_get_board_info_by_dma_coherent(struct device *dev, void *ipc_info, int ipc_info_size,
				     int ipc_event,
				     int dma_alloc_coherent_size,
				     int (*dma_addr_offset)(struct device *dev,
							    dma_addr_t *dma_addr_src,
							    dma_addr_t *dma_addr_dst),
				     void (*ipc_request_param_setting)(void *ipc_param,
								       dma_addr_t dma_addr_dst,
								       int *ipc_param_size),
				     int (*dma_data_valid)(void *dma_data))
{
	struct ipc_msg ipc_msg = {0};
	struct ipc_msg_hdr *header = &ipc_msg.header;
	dma_addr_t dma_addr_src, dma_addr_dst;
	void *dma_data;
	void *ipc_param;
	int ipc_param_size = 0;

	dma_data = os_dma_alloc_coherent(dev, dma_alloc_coherent_size, &dma_addr_src,
					 OS_VAL(GFP_DMA));
	if (!dma_data) {
		BOARD_ERROR(dev, "alloc dma address for info_rom failed\n");
		return -OS_VAL(EINVAL);
	}
	dma_addr_dst = dma_addr_src;

	if (dma_addr_offset && dma_addr_offset(dev, &dma_addr_src, &dma_addr_dst)) {
		BOARD_ERROR(dev, "dma_addr_offset error\n");
		goto err;
	}
	BOARD_INFO(dev, "alloc_dma_addr:0x%llx, dma_addr_offset:0x%llx\n", dma_addr_src,
		   dma_addr_dst);

	ipc_param = (void *)&ipc_msg.data[0];
	if (ipc_request_param_setting)
		ipc_request_param_setting(ipc_param, dma_addr_dst, &ipc_param_size);

	header->event_type = EVENT_TYPE_HOST_TO_SMC;
	header->event_id   = ipc_event;
	header->response   = 1;
	header->source     = MTGPU_IPC_NODE_PCIE;
	header->target     = MTGPU_IPC_NODE_SMC;
	header->data_size  = ipc_param_size / sizeof(u32);

	if (mtgpu_ipc_transmit(dev, &ipc_msg) != PCIE_IPC_MSG_DONE_WITH_RESPONSE) {
		BOARD_ERROR(dev, "get board info failed\n");
		goto err;
	}

	if (dma_data_valid && dma_data_valid(dma_data))
		goto err;

	os_memcpy(ipc_info, dma_data, ipc_info_size);
	os_dma_free_coherent(dev, dma_alloc_coherent_size, dma_data, dma_addr_src);

	return 0;

err:
	os_dma_free_coherent(dev, dma_alloc_coherent_size, dma_data, dma_addr_src);

	return -OS_VAL(EINVAL);
}

#endif /* __MTGPU_BOARD_CFG_H__ */
