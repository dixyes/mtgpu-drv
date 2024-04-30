/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */


#ifndef __MTGPU_DEFS_H__
#define __MTGPU_DEFS_H__

#define MTGPU_CORE_COUNT_MAX        (32)
#define MTGPU_CORE_COUNT_7	    (7)

/* TODO: Currently assume the MTGPU default video ram size is 16G.
 * It should get the video ram size from SMC in the future for each
 * mtgpu graphics card
 */
#define MTGPU_VRAM_DEFAULT_SIZE		(0x400000000UL)

#define MTGPU_DSP_MEM_SIZE		(0x1000000UL)
/* In QY, SMC does not allocate ddr mem anymore. Just extend the smc
 * reserved size to MTGPU_SMC_RESERVED_SIZE but FEC consumes it.
 */
#define MTGPU_SMC_RESERVED_SIZE		(0x200000UL)
#define MTGPU_SMC_MEM_SIZE		(MTGPU_DSP_MEM_SIZE + MTGPU_SMC_RESERVED_SIZE)
#define MTGPU_RESERVED_MEM_SIZE		(0x10000000UL)

/* MTGPU_FEC_BL_BASE follows closely after MTGPU_DSP_MEM_SIZE */
#define MTGPU_FEC_BL_BASE		(0x1000000UL)
#define MTGPU_FEC_IMAGE_BASE		(0x1800000UL)
#define MTGPU_FEC_KERNEL_PARAMS_BASE	(MTGPU_FEC_BL_BASE + 0xFFC00)
#define MTGPU_FEC_KERNEL_PARAMS_LEN	0x400
/* Specify fec reserved size as MTGPU_SMC_RESERVED_SIZE by default to avoid
 * memory allocation issue in small ddr capability SKUs. Meanwhile assign module
 * params proper size to support full FEC function.
 */
#define MTGPU_FEC_RESERVED_SIZE		(mtgpu_smc_mem_size - MTGPU_DSP_MEM_SIZE)

#define PCI_VENDOR_ID_XIX		(0x10EE)
#define DEVICE_ID_HAPS_SUDI104		(0x9011)

#define PCI_ANY_ID_MT			(~0)
#define PCI_VENDOR_ID_MT		(0x1ED5)
#define DEVICE_ID_SUDI104		(0x0100)
#define DEVICE_ID_MTT_S10		(0x0101)
#define DEVICE_ID_MTT_S30_2_Core	(0x0102)
#define DEVICE_ID_MTT_S30_4_Core	(0x0103)
#define DEVICE_ID_MTT_S1000M		(0x0121)
#define DEVICE_ID_MTT_S50		(0x0105)
#define DEVICE_ID_MTT_S60		(0x0106)
#define DEVICE_ID_MTT_S100		(0x0111)
#define DEVICE_ID_MTT_S1000		(0x0122)
#define DEVICE_ID_MTT_S2000		(0x0123)

#define DEVICE_ID_QUYUAN1		(0x0200)
#define DEVICE_ID_MTT_S80		(0x0201)
#define DEVICE_ID_MTT_S70		(0x0202)
#define DEVICE_ID_MTT_X300		(0x0211)
#define DEVICE_ID_MTT_S3000		(0x0222)

#define DEVICE_ID_QUYUAN2		(0x0300)
#define DEVICE_ID_MTT_S90		(0x0301)
#define DEVICE_ID_MTT_G3D80		(0x0303)
#define	DEVICE_ID_MTT_S4000_8CORE	(0x0323)
#define DEVICE_ID_MTT_G3S90		(0x0321)
#define DEVICE_ID_MTT_S4000_7CORE	(0x0327)

#define DEVICE_ID_QUYUAN1_VF		(0x02aa)
#define DEVICE_ID_QUYUAN2_VF		(0x03aa)

#define DEVICE_ID_PINGHU1		(0x0400)

#define GET_DEVICE_ID(mtdev)       (os_get_pci_device_id(os_to_pci_dev((mtdev)->dev)) & 0xff00)

#define DEVICE_IS_SUDI(mtdev)     (GET_DEVICE_ID(mtdev) == DEVICE_ID_SUDI104)
#define DEVICE_IS_QUYUAN1(mtdev)  (GET_DEVICE_ID(mtdev) == DEVICE_ID_QUYUAN1)
#define DEVICE_IS_QUYUAN2(mtdev)  (GET_DEVICE_ID(mtdev) == DEVICE_ID_QUYUAN2)
#define DEVICE_IS_PINGHU1(mtdev)  (GET_DEVICE_ID(mtdev) == DEVICE_ID_PINGHU1)

/* ALL MT Soc Platform common definition */
#define PCI_STD_NUM_BARS			6

#define MTGPU_DEVICE_NAME		"mtgpu"

#define MTGPU_DEVICE_NAME_VIDEO		"mtgpu_video"
#define MTGPU_DEVICE_NAME_AUDIO		"mtgpu_audio"

#define MTGPU_DEVICE_NAME_DISPC		"mtgpu-dispc"
#define MTGPU_DEVICE_NAME_DP		"mtgpu-dp"
#define MTGPU_DEVICE_NAME_DP_PHY	"mtgpu-dp-phy"
#define MTGPU_DEVICE_NAME_HDMI		"mtgpu-hdmi"
#define MTGPU_DEVICE_NAME_DSC		"mtgpu-dsc"

/* Valid values for the MTGPU_MEMORY_CONFIG configuration option */
#define MTGPU_MEMORY_LOCAL			1
#define MTGPU_MEMORY_HOST			2
#define MTGPU_MEMORY_HYBRID			3

#define MTGPU_SYS_BAR               0
#define MTGPU_DDR_BAR               2

#define MTGPU_MAX_CURSOR_SIZE	64

#define MTGPU_PCIE_IRQ_NUM (8)

/* sriov is turned off by default */
#define MTGPU_DISABLE_SRIOV             (0)
#define MTGPU_ENABLE_SRIOV              (1)

/* SoC Register's Resource index */
#define MTGPU_SOC_GPU_RES		(0)
#define MTGPU_SOC_DAA_RES		(1)
#define MTGPU_SOC_MISC_RES		(2)

/*
 * NATIVE: same as bare-metal
 * HOST: used in hypervisor in virtualization scenario.
 * GUEST: used in guest OS  in virtualization scenario.
 */
#define MTGPU_DRIVER_MODE_NATIVE	-1
#define MTGPU_DRIVER_MODE_HOST		0
#define MTGPU_DRIVER_MODE_GUEST		1

#define MTGPU_VGPU_FW_PAGE_TABLE_SIZE	(1 * 1024 * 1024)

enum mtgpu_interrupt_id {
	MTGPU_INTERRUPT_LOCK_ERR = 1,
	MTGPU_INTERRUPT_FEC,
	MTGPU_INTERRUPT_SMC,
	MTGPU_INTERRUPT_FECM,
	MTGPU_INTERRUPT_CMC,
	MTGPU_INTERRUPT_AUDIO_0,
	MTGPU_INTERRUPT_AUDIO_1,
	MTGPU_INTERRUPT_AUDIO_2,
	MTGPU_INTERRUPT_AUDIO_3,
	MTGPU_INTERRUPT_VID_BODA955,
	MTGPU_INTERRUPT_VID_WAVE517,
	MTGPU_INTERRUPT_VID_WAVE517_1,
	MTGPU_INTERRUPT_VID_WAVE517_2,
	MTGPU_INTERRUPT_VID_WAVE517_3,
	MTGPU_INTERRUPT_VID_WAVE517_4,
	MTGPU_INTERRUPT_VID_WAVE517_5,
	MTGPU_INTERRUPT_VID_WAVE627,
	MTGPU_INTERRUPT_VID_WAVE627_1,
	MTGPU_INTERRUPT_VID_WAVE627_2,
	MTGPU_INTERRUPT_VID_WAVE627_3,
	MTGPU_INTERRUPT_JPU0,
	MTGPU_INTERRUPT_JPU1,
	MTGPU_INTERRUPT_JPU2,
	MTGPU_INTERRUPT_JPU3,
	MTGPU_INTERRUPT_GPU0,
	MTGPU_INTERRUPT_GPU1,
	MTGPU_INTERRUPT_GPU2,
	MTGPU_INTERRUPT_GPU3,
	MTGPU_INTERRUPT_GPU4,
	MTGPU_INTERRUPT_GPU5,
	MTGPU_INTERRUPT_GPU6,
	MTGPU_INTERRUPT_GPU7,
	MTGPU_INTERRUPT_DISPC0,
	MTGPU_INTERRUPT_DISPC1,
	MTGPU_INTERRUPT_DISPC2,
	MTGPU_INTERRUPT_DISPC3,
	MTGPU_INTERRUPT_DPTX0,
	MTGPU_INTERRUPT_DPTX1,
	MTGPU_INTERRUPT_DPTX2,
	MTGPU_INTERRUPT_DPTX3,
	MTGPU_INTERRUPT_DPTX0_PHY,
	MTGPU_INTERRUPT_DPTX1_PHY,
	MTGPU_INTERRUPT_HDMI,
	MTGPU_INTERRUPT_PCIE_DMA,
	MTGPU_INTERRUPT_PCIE_LOCAL,
	MTGPU_INTERRUPT_CTRL_MTLINK0,
	MTGPU_INTERRUPT_CTRL_MTLINK1,
	MTGPU_INTERRUPT_CTRL_MTLINK2,
	MTGPU_INTERRUPT_CTRL_MTLINK3,
	MTGPU_INTERRUPT_CTRL_MTLINK4,
	MTGPU_INTERRUPT_CTRL_MTLINK5,
	MTGPU_INTERRUPT_CTRL_MTLINK6,
	MTGPU_INTERRUPT_CTRL_MTLINK7,
	MTGPU_INTERRUPT_CTRL_MTLINK8,
	MTGPU_INTERRUPT_CTRL_MTLINK9,
	MTGPU_INTERRUPT_CTRL_MTLINK10,
	MTGPU_INTERRUPT_CTRL_MTLINK11,
	MTGPU_INTERRUPT_CTRL_MTLINK12,
	MTGPU_INTERRUPT_CTRL_MTLINK13,
	MTGPU_INTERRUPT_WRAP_MTLINK0,
	MTGPU_INTERRUPT_WRAP_MTLINK1,
	MTGPU_INTERRUPT_WRAP_MTLINK2,
	MTGPU_INTERRUPT_WRAP_MTLINK3,
	MTGPU_INTERRUPT_COUNT
};

enum mtgpu_softirq_tx_id {
	MTGPU_SOFTIRQ_TO_SMC_0,
	MTGPU_SOFTIRQ_TO_SMC_1,
	MTGPU_SOFTIRQ_TO_SMC_2,
	MTGPU_SOFTIRQ_TO_SMC_3,
	MTGPU_SOFTIRQ_TO_SMC_4,
	MTGPU_SOFTIRQ_TO_SMC_5,
	MTGPU_SOFTIRQ_TO_SMC_6,
	MTGPU_SOFTIRQ_TO_SMC_7,
	MTGPU_SOFTIRQ_TO_SMC_8,
	MTGPU_SOFTIRQ_TO_SMC_9,
	MTGPU_SOFTIRQ_TO_SMC_10,
	MTGPU_SOFTIRQ_TO_SMC_11,
	MTGPU_SOFTIRQ_TO_SMC_12,
	MTGPU_SOFTIRQ_TO_SMC_13,
	MTGPU_SOFTIRQ_TO_SMC_14,
	MTGPU_SOFTIRQ_TO_SMC_15,
	MTGPU_SOFTIRQ_TO_SMC_16,
	MTGPU_SOFTIRQ_TO_SMC_17,
	MTGPU_SOFTIRQ_TO_SMC_18,
	MTGPU_SOFTIRQ_TO_SMC_19,
	MTGPU_SOFTIRQ_TO_SMC_20,
	MTGPU_SOFTIRQ_TO_SMC_21,
	MTGPU_SOFTIRQ_TO_SMC_22,
	MTGPU_SOFTIRQ_TO_SMC_23,
	MTGPU_SOFTIRQ_TO_SMC_24,
	MTGPU_SOFTIRQ_TO_SMC_25,
	MTGPU_SOFTIRQ_TO_SMC_26,
	MTGPU_SOFTIRQ_TO_SMC_27,
	MTGPU_SOFTIRQ_TO_SMC_28,
	MTGPU_SOFTIRQ_TO_SMC_29,
	MTGPU_SOFTIRQ_TO_SMC_30,
	MTGPU_SOFTIRQ_TO_SMC_31,

	MTGPU_SOFTIRQ_TO_FEC_0,
	MTGPU_SOFTIRQ_TO_FEC_1,
	MTGPU_SOFTIRQ_TO_FEC_2,
	MTGPU_SOFTIRQ_TO_FEC_3,
	MTGPU_SOFTIRQ_TO_FEC_4,
	MTGPU_SOFTIRQ_TO_FEC_5,
	MTGPU_SOFTIRQ_TO_FEC_6,
	MTGPU_SOFTIRQ_TO_FEC_7,
	MTGPU_SOFTIRQ_TO_FEC_8,
	MTGPU_SOFTIRQ_TO_FEC_9,
	MTGPU_SOFTIRQ_TO_FEC_10,
	MTGPU_SOFTIRQ_TO_FEC_11,
	MTGPU_SOFTIRQ_TO_FEC_12,
	MTGPU_SOFTIRQ_TO_FEC_13,
	MTGPU_SOFTIRQ_TO_FEC_14,
	MTGPU_SOFTIRQ_TO_FEC_15,
	MTGPU_SOFTIRQ_TO_FEC_16,
	MTGPU_SOFTIRQ_TO_FEC_17,
	MTGPU_SOFTIRQ_TO_FEC_18,
	MTGPU_SOFTIRQ_TO_FEC_19,
	MTGPU_SOFTIRQ_TO_FEC_20,
	MTGPU_SOFTIRQ_TO_FEC_21,
	MTGPU_SOFTIRQ_TO_FEC_22,
	MTGPU_SOFTIRQ_TO_FEC_23,
	MTGPU_SOFTIRQ_TO_FEC_24,
	MTGPU_SOFTIRQ_TO_FEC_25,
	MTGPU_SOFTIRQ_TO_FEC_26,
	MTGPU_SOFTIRQ_TO_FEC_27,
	MTGPU_SOFTIRQ_TO_FEC_28,
	MTGPU_SOFTIRQ_TO_FEC_29,
	MTGPU_SOFTIRQ_TO_FEC_30,
	MTGPU_SOFTIRQ_TO_FEC_31,

	MTGPU_SOFTIRQ_TO_FEC_M_0,
	MTGPU_SOFTIRQ_TO_FEC_M_1,
	MTGPU_SOFTIRQ_TO_FEC_M_2,
	MTGPU_SOFTIRQ_TO_FEC_M_3,
	MTGPU_SOFTIRQ_TO_FEC_M_4,
	MTGPU_SOFTIRQ_TO_FEC_M_5,
	MTGPU_SOFTIRQ_TO_FEC_M_6,
	MTGPU_SOFTIRQ_TO_FEC_M_7,
	MTGPU_SOFTIRQ_TO_FEC_M_8,
	MTGPU_SOFTIRQ_TO_FEC_M_9,
	MTGPU_SOFTIRQ_TO_FEC_M_10,
	MTGPU_SOFTIRQ_TO_FEC_M_11,
	MTGPU_SOFTIRQ_TO_FEC_M_12,
	MTGPU_SOFTIRQ_TO_FEC_M_13,
	MTGPU_SOFTIRQ_TO_FEC_M_14,
	MTGPU_SOFTIRQ_TO_FEC_M_15,
	MTGPU_SOFTIRQ_TO_FEC_M_16,
	MTGPU_SOFTIRQ_TO_FEC_M_17,
	MTGPU_SOFTIRQ_TO_FEC_M_18,
	MTGPU_SOFTIRQ_TO_FEC_M_19,
	MTGPU_SOFTIRQ_TO_FEC_M_20,
	MTGPU_SOFTIRQ_TO_FEC_M_21,
	MTGPU_SOFTIRQ_TO_FEC_M_22,
	MTGPU_SOFTIRQ_TO_FEC_M_23,
	MTGPU_SOFTIRQ_TO_FEC_M_24,
	MTGPU_SOFTIRQ_TO_FEC_M_25,
	MTGPU_SOFTIRQ_TO_FEC_M_26,
	MTGPU_SOFTIRQ_TO_FEC_M_27,
	MTGPU_SOFTIRQ_TO_FEC_M_28,
	MTGPU_SOFTIRQ_TO_FEC_M_29,
	MTGPU_SOFTIRQ_TO_FEC_M_30,
	MTGPU_SOFTIRQ_TO_FEC_M_31,

	MTGPU_SOFTIRQ_TO_CMC_0,
	MTGPU_SOFTIRQ_TO_CMC_1,
	MTGPU_SOFTIRQ_TO_CMC_2,
	MTGPU_SOFTIRQ_TO_CMC_3,
	MTGPU_SOFTIRQ_TO_CMC_4,
	MTGPU_SOFTIRQ_TO_CMC_5,
	MTGPU_SOFTIRQ_TO_CMC_6,
	MTGPU_SOFTIRQ_TO_CMC_7,
	MTGPU_SOFTIRQ_TO_CMC_8,
	MTGPU_SOFTIRQ_TO_CMC_9,
	MTGPU_SOFTIRQ_TO_CMC_10,
	MTGPU_SOFTIRQ_TO_CMC_11,
	MTGPU_SOFTIRQ_TO_CMC_12,
	MTGPU_SOFTIRQ_TO_CMC_13,
	MTGPU_SOFTIRQ_TO_CMC_14,
	MTGPU_SOFTIRQ_TO_CMC_15,
	MTGPU_SOFTIRQ_TO_CMC_16,
	MTGPU_SOFTIRQ_TO_CMC_17,
	MTGPU_SOFTIRQ_TO_CMC_18,
	MTGPU_SOFTIRQ_TO_CMC_19,
	MTGPU_SOFTIRQ_TO_CMC_20,
	MTGPU_SOFTIRQ_TO_CMC_21,
	MTGPU_SOFTIRQ_TO_CMC_22,
	MTGPU_SOFTIRQ_TO_CMC_23,
	MTGPU_SOFTIRQ_TO_CMC_24,
	MTGPU_SOFTIRQ_TO_CMC_25,
	MTGPU_SOFTIRQ_TO_CMC_26,
	MTGPU_SOFTIRQ_TO_CMC_27,
	MTGPU_SOFTIRQ_TO_CMC_28,
	MTGPU_SOFTIRQ_TO_CMC_29,
	MTGPU_SOFTIRQ_TO_CMC_30,
	MTGPU_SOFTIRQ_TO_CMC_31,
	MTGPU_TX_SOFTIRQ_CNT
};

enum mtgpu_softirq_rx_id {
	MTGPU_SOFTIRQ_FROM_SMC_0,
	MTGPU_SOFTIRQ_FROM_SMC_1,
	MTGPU_SOFTIRQ_FROM_SMC_2,
	MTGPU_SOFTIRQ_FROM_SMC_3,
	MTGPU_SOFTIRQ_FROM_SMC_4,
	MTGPU_SOFTIRQ_FROM_SMC_5,
	MTGPU_SOFTIRQ_FROM_SMC_6,
	MTGPU_SOFTIRQ_FROM_SMC_7,
	MTGPU_SOFTIRQ_FROM_SMC_8,
	MTGPU_SOFTIRQ_FROM_SMC_9,
	MTGPU_SOFTIRQ_FROM_SMC_10,
	MTGPU_SOFTIRQ_FROM_SMC_11,
	MTGPU_SOFTIRQ_FROM_SMC_12,
	MTGPU_SOFTIRQ_FROM_SMC_13,
	MTGPU_SOFTIRQ_FROM_SMC_14,
	MTGPU_SOFTIRQ_FROM_SMC_15,
	MTGPU_SOFTIRQ_FROM_SMC_16,
	MTGPU_SOFTIRQ_FROM_SMC_17,
	MTGPU_SOFTIRQ_FROM_SMC_18,
	MTGPU_SOFTIRQ_FROM_SMC_19,
	MTGPU_SOFTIRQ_FROM_SMC_20,
	MTGPU_SOFTIRQ_FROM_SMC_21,
	MTGPU_SOFTIRQ_FROM_SMC_22,
	MTGPU_SOFTIRQ_FROM_SMC_23,
	MTGPU_SOFTIRQ_FROM_SMC_24,
	MTGPU_SOFTIRQ_FROM_SMC_25,
	MTGPU_SOFTIRQ_FROM_SMC_26,
	MTGPU_SOFTIRQ_FROM_SMC_27,
	MTGPU_SOFTIRQ_FROM_SMC_28,
	MTGPU_SOFTIRQ_FROM_SMC_29,
	MTGPU_SOFTIRQ_FROM_SMC_30,
	MTGPU_SOFTIRQ_FROM_SMC_31,

	MTGPU_SOFTIRQ_FROM_FEC_0,
	MTGPU_SOFTIRQ_FROM_FEC_1,
	MTGPU_SOFTIRQ_FROM_FEC_2,
	MTGPU_SOFTIRQ_FROM_FEC_3,
	MTGPU_SOFTIRQ_FROM_FEC_4,
	MTGPU_SOFTIRQ_FROM_FEC_5,
	MTGPU_SOFTIRQ_FROM_FEC_6,
	MTGPU_SOFTIRQ_FROM_FEC_7,
	MTGPU_SOFTIRQ_FROM_FEC_8,
	MTGPU_SOFTIRQ_FROM_FEC_9,
	MTGPU_SOFTIRQ_FROM_FEC_10,
	MTGPU_SOFTIRQ_FROM_FEC_11,
	MTGPU_SOFTIRQ_FROM_FEC_12,
	MTGPU_SOFTIRQ_FROM_FEC_13,
	MTGPU_SOFTIRQ_FROM_FEC_14,
	MTGPU_SOFTIRQ_FROM_FEC_15,
	MTGPU_SOFTIRQ_FROM_FEC_16,
	MTGPU_SOFTIRQ_FROM_FEC_17,
	MTGPU_SOFTIRQ_FROM_FEC_18,
	MTGPU_SOFTIRQ_FROM_FEC_19,
	MTGPU_SOFTIRQ_FROM_FEC_20,
	MTGPU_SOFTIRQ_FROM_FEC_21,
	MTGPU_SOFTIRQ_FROM_FEC_22,
	MTGPU_SOFTIRQ_FROM_FEC_23,
	MTGPU_SOFTIRQ_FROM_FEC_24,
	MTGPU_SOFTIRQ_FROM_FEC_25,
	MTGPU_SOFTIRQ_FROM_FEC_26,
	MTGPU_SOFTIRQ_FROM_FEC_27,
	MTGPU_SOFTIRQ_FROM_FEC_28,
	MTGPU_SOFTIRQ_FROM_FEC_29,
	MTGPU_SOFTIRQ_FROM_FEC_30,
	MTGPU_SOFTIRQ_FROM_FEC_31,

	MTGPU_SOFTIRQ_FROM_FEC_M_0,
	MTGPU_SOFTIRQ_FROM_FEC_M_1,
	MTGPU_SOFTIRQ_FROM_FEC_M_2,
	MTGPU_SOFTIRQ_FROM_FEC_M_3,
	MTGPU_SOFTIRQ_FROM_FEC_M_4,
	MTGPU_SOFTIRQ_FROM_FEC_M_5,
	MTGPU_SOFTIRQ_FROM_FEC_M_6,
	MTGPU_SOFTIRQ_FROM_FEC_M_7,
	MTGPU_SOFTIRQ_FROM_FEC_M_8,
	MTGPU_SOFTIRQ_FROM_FEC_M_9,
	MTGPU_SOFTIRQ_FROM_FEC_M_10,
	MTGPU_SOFTIRQ_FROM_FEC_M_11,
	MTGPU_SOFTIRQ_FROM_FEC_M_12,
	MTGPU_SOFTIRQ_FROM_FEC_M_13,
	MTGPU_SOFTIRQ_FROM_FEC_M_14,
	MTGPU_SOFTIRQ_FROM_FEC_M_15,
	MTGPU_SOFTIRQ_FROM_FEC_M_16,
	MTGPU_SOFTIRQ_FROM_FEC_M_17,
	MTGPU_SOFTIRQ_FROM_FEC_M_18,
	MTGPU_SOFTIRQ_FROM_FEC_M_19,
	MTGPU_SOFTIRQ_FROM_FEC_M_20,
	MTGPU_SOFTIRQ_FROM_FEC_M_21,
	MTGPU_SOFTIRQ_FROM_FEC_M_22,
	MTGPU_SOFTIRQ_FROM_FEC_M_23,
	MTGPU_SOFTIRQ_FROM_FEC_M_24,
	MTGPU_SOFTIRQ_FROM_FEC_M_25,
	MTGPU_SOFTIRQ_FROM_FEC_M_26,
	MTGPU_SOFTIRQ_FROM_FEC_M_27,
	MTGPU_SOFTIRQ_FROM_FEC_M_28,
	MTGPU_SOFTIRQ_FROM_FEC_M_29,
	MTGPU_SOFTIRQ_FROM_FEC_M_30,
	MTGPU_SOFTIRQ_FROM_FEC_M_31,

	MTGPU_SOFTIRQ_FROM_CMC_0,
	MTGPU_SOFTIRQ_FROM_CMC_1,
	MTGPU_SOFTIRQ_FROM_CMC_2,
	MTGPU_SOFTIRQ_FROM_CMC_3,
	MTGPU_SOFTIRQ_FROM_CMC_4,
	MTGPU_SOFTIRQ_FROM_CMC_5,
	MTGPU_SOFTIRQ_FROM_CMC_6,
	MTGPU_SOFTIRQ_FROM_CMC_7,
	MTGPU_SOFTIRQ_FROM_CMC_8,
	MTGPU_SOFTIRQ_FROM_CMC_9,
	MTGPU_SOFTIRQ_FROM_CMC_10,
	MTGPU_SOFTIRQ_FROM_CMC_11,
	MTGPU_SOFTIRQ_FROM_CMC_12,
	MTGPU_SOFTIRQ_FROM_CMC_13,
	MTGPU_SOFTIRQ_FROM_CMC_14,
	MTGPU_SOFTIRQ_FROM_CMC_15,
	MTGPU_SOFTIRQ_FROM_CMC_16,
	MTGPU_SOFTIRQ_FROM_CMC_17,
	MTGPU_SOFTIRQ_FROM_CMC_18,
	MTGPU_SOFTIRQ_FROM_CMC_19,
	MTGPU_SOFTIRQ_FROM_CMC_20,
	MTGPU_SOFTIRQ_FROM_CMC_21,
	MTGPU_SOFTIRQ_FROM_CMC_22,
	MTGPU_SOFTIRQ_FROM_CMC_23,
	MTGPU_SOFTIRQ_FROM_CMC_24,
	MTGPU_SOFTIRQ_FROM_CMC_25,
	MTGPU_SOFTIRQ_FROM_CMC_26,
	MTGPU_SOFTIRQ_FROM_CMC_27,
	MTGPU_SOFTIRQ_FROM_CMC_28,
	MTGPU_SOFTIRQ_FROM_CMC_29,
	MTGPU_SOFTIRQ_FROM_CMC_30,
	MTGPU_SOFTIRQ_FROM_CMC_31,
	MTGPU_RX_SOFT_INT_CNT
};

#define MTGPU_DISP_DEV_NUM		(32)
#define MTGPU_CORE_DEFAULT_BITMASK	BIT(0)

#define BYTES_COUNT_64_BIT		8
#define BYTES_COUNT_32_BIT		4
#define BYTES_COUNT_16_BIT		2
#define BYTES_COUNT_8_BIT		1

enum pstate_mode {
	PSTATE_DISABLED = 0, /* pstate disabled */
	PSTATE_ENABLED = 1,  /* pstate enabled */
};

#endif /* _MTGPU_DEFS_H_ */
