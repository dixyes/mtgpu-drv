/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */
#ifndef __MTGPU_DPCD_DEFS_H__
#define __MTGPU_DPCD_DEFS_H__

/* AUX CH addresses */
/* DPCD */
#define DP_DPCD_REV				0x000
# define DP_DPCD_REV_10				0x10
# define DP_DPCD_REV_11				0x11
# define DP_DPCD_REV_12				0x12
# define DP_DPCD_REV_13				0x13
# define DP_DPCD_REV_14				0x14

#define DP_MAX_LINK_RATE			0x001

#define DP_MAX_LANE_COUNT			0x002
# define DP_MAX_LANE_COUNT_MASK			0x1f
# define DP_TPS3_SUPPORTED			(1 << 6) /* 1.2 */
# define DP_ENHANCED_FRAME_CAP			(1 << 7)

#define DP_MAX_DOWNSPREAD			0x003
# define DP_MAX_DOWNSPREAD_0_5			(1 << 0)
# define DP_NO_AUX_HANDSHAKE_LINK_TRAINING	(1 << 6)
# define DP_TPS4_SUPPORTED			(1 << 7)

#define DP_MAIN_LINK_CHANNEL_CODING		0x006

#define DP_TRAINING_AUX_RD_INTERVAL		0x00e	 /* XXX 1.2? */
# define DP_TRAINING_AUX_RD_MASK		0x7F	 /* DP 1.3 */
# define DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT	(1 << 7) /* DP 1.3 */

/* link configuration */
#define	DP_LINK_BW_SET				0x100
# define DP_LINK_RATE_TABLE			0x00	/* eDP 1.4 */
# define DP_LINK_BW_1_62			0x06
# define DP_LINK_BW_2_7				0x0a
# define DP_LINK_BW_5_4				0x14	/* 1.2 */
# define DP_LINK_BW_8_1				0x1e	/* 1.4 */

#define DP_LANE_COUNT_SET			0x101
# define DP_LANE_COUNT_MASK			0x0f
# define DP_LANE_COUNT_ENHANCED_FRAME_EN	(1 << 7)

#define DP_TRAINING_PATTERN_SET			0x102
# define DP_TRAINING_PATTERN_DISABLE		0
# define DP_TRAINING_PATTERN_1			1
# define DP_TRAINING_PATTERN_2			2
# define DP_TRAINING_PATTERN_3			3	/* 1.2 */
# define DP_TRAINING_PATTERN_4			7	/* 1.4 */
# define DP_TRAINING_PATTERN_MASK		0x3
# define DP_TRAINING_PATTERN_MASK_1_4		0xf

# define DP_RECOVERED_CLOCK_OUT_EN		(1 << 4)
# define DP_LINK_SCRAMBLING_DISABLE		(1 << 5)

#define DP_TRAINING_LANE0_SET			0x103
#define DP_TRAINING_LANE1_SET			0x104
#define DP_TRAINING_LANE2_SET			0x105
#define DP_TRAINING_LANE3_SET			0x106

# define DP_TRAIN_VOLTAGE_SWING_MASK		0x3
# define DP_TRAIN_VOLTAGE_SWING_SHIFT		0
# define DP_TRAIN_MAX_SWING_REACHED		(1 << 2)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_0		(0 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_1		(1 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_2		(2 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_3		(3 << 0)

# define DP_TRAIN_PRE_EMPHASIS_MASK		(3 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_0		(0 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_1		(1 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_2		(2 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_3		(3 << 3)

# define DP_TRAIN_PRE_EMPHASIS_SHIFT		3
# define DP_TRAIN_MAX_PRE_EMPHASIS_REACHED	(1 << 5)

#define DP_DOWNSPREAD_CTRL			0x107
# define DP_SPREAD_AMP_0_5			(1 << 4)
# define DP_MSA_TIMING_PAR_IGNORE_EN		(1 << 7) /* eDP */

#define DP_MAIN_LINK_CHANNEL_CODING_SET		0x108
# define DP_SET_ANSI_8B10B			(1 << 0)

#define DP_LINK_QUAL_LANE0_SET			0x10b	/* DPCD >= 1.2 */
#define DP_LINK_QUAL_LANE1_SET			0x10c
#define DP_LINK_QUAL_LANE2_SET			0x10d
#define DP_LINK_QUAL_LANE3_SET			0x10e
# define DP_LINK_QUAL_PATTERN_DISABLE		0
# define DP_LINK_QUAL_PATTERN_D10_2		1
# define DP_LINK_QUAL_PATTERN_ERROR_RATE	2
# define DP_LINK_QUAL_PATTERN_PRBS7		3
# define DP_LINK_QUAL_PATTERN_80BIT_CUSTOM	4
# define DP_LINK_QUAL_PATTERN_HBR2_EYE		5
# define DP_LINK_QUAL_PATTERN_MASK		7

#define DP_SINK_COUNT				0x200
/* prior to 1.2 bit 7 was reserved mbz */
# define DP_GET_SINK_COUNT(x)			((((x) & 0x80) >> 1) | ((x) & 0x3f))
# define DP_SINK_CP_READY			(1 << 6)

#define DP_DEVICE_SERVICE_IRQ_VECTOR		0x201
# define DP_REMOTE_CONTROL_COMMAND_PENDING	(1 << 0)
# define DP_AUTOMATED_TEST_REQUEST		(1 << 1)
# define DP_CP_IRQ				(1 << 2)
# define DP_MCCS_IRQ				(1 << 3)
# define DP_DOWN_REP_MSG_RDY			(1 << 4) /* 1.2 MST */
# define DP_UP_REQ_MSG_RDY			(1 << 5) /* 1.2 MST */
# define DP_SINK_SPECIFIC_IRQ			(1 << 6)

#define DP_LANE0_1_STATUS			0x202
#define DP_LANE2_3_STATUS			0x203
# define DP_LANE_CR_DONE			(1 << 0)
# define DP_LANE_CHANNEL_EQ_DONE		(1 << 1)
# define DP_LANE_SYMBOL_LOCKED			(1 << 2)

#define DP_CHANNEL_EQ_BITS	(DP_LANE_CR_DONE |		\
				 DP_LANE_CHANNEL_EQ_DONE |	\
				 DP_LANE_SYMBOL_LOCKED)

#define DP_LANE_ALIGN_STATUS_UPDATED		0x204
#define DP_INTERLANE_ALIGN_DONE			(1 << 0)
#define DP_DOWNSTREAM_PORT_STATUS_CHANGED	(1 << 6)
#define DP_LINK_STATUS_UPDATED			(1 << 7)

#define DP_TEST_REQUEST				0x218
# define DP_TEST_LINK_TRAINING			(1 << 0)
# define DP_TEST_LINK_VIDEO_PATTERN		(1 << 1)
# define DP_TEST_LINK_EDID_READ			(1 << 2)
# define DP_TEST_LINK_PHY_TEST_PATTERN		(1 << 3) /* DPCD >= 1.1 */
# define DP_TEST_LINK_FAUX_PATTERN		(1 << 4) /* DPCD >= 1.2 */
# define DP_TEST_LINK_AUDIO_PATTERN		(1 << 5) /* DPCD >= 1.2 */
# define DP_TEST_LINK_AUDIO_DISABLED_VIDEO	(1 << 6) /* DPCD >= 1.2 */

#define DP_TEST_LINK_RATE			0x219
# define DP_LINK_RATE_162			(0x6)
# define DP_LINK_RATE_27			(0xa)

#define DP_TEST_LANE_COUNT			0x220

#define DP_TEST_PHY_PATTERN			0x248
#define DP_TEST_80BIT_CUSTOM_PATTERN_7_0	0x250
#define	DP_TEST_80BIT_CUSTOM_PATTERN_15_8	0x251
#define	DP_TEST_80BIT_CUSTOM_PATTERN_23_16	0x252
#define	DP_TEST_80BIT_CUSTOM_PATTERN_31_24	0x253
#define	DP_TEST_80BIT_CUSTOM_PATTERN_39_32	0x254
#define	DP_TEST_80BIT_CUSTOM_PATTERN_47_40	0x255
#define	DP_TEST_80BIT_CUSTOM_PATTERN_55_48	0x256
#define	DP_TEST_80BIT_CUSTOM_PATTERN_63_56	0x257
#define	DP_TEST_80BIT_CUSTOM_PATTERN_71_64	0x258
#define	DP_TEST_80BIT_CUSTOM_PATTERN_79_72	0x259

#define DP_TEST_RESPONSE			0x260
# define DP_TEST_ACK				(1 << 0)
# define DP_TEST_NAK				(1 << 1)
# define DP_TEST_EDID_CHECKSUM_WRITE		(1 << 2)

#define DP_SET_POWER				0x600
# define DP_SET_POWER_D0			0x1
# define DP_SET_POWER_D3			0x2
# define DP_SET_POWER_MASK			0x3
# define DP_SET_POWER_D3_AUX_ON			0x5

#define DP_EDP_DPCD_REV				0x700	/* eDP 1.2 */
# define DP_EDP_11				0x00
# define DP_EDP_12				0x01
# define DP_EDP_13				0x02
# define DP_EDP_14				0x03
# define DP_EDP_14a				0x04	/* eDP 1.4a */
# define DP_EDP_14b				0x05	/* eDP 1.4b */

#define DP_DP13_DPCD_REV			0x2200
#define DP_DP13_MAX_LINK_RATE			0x2201

#define DP_LINK_STATUS_SIZE			6

#define DP_BRANCH_OUI_HEADER_SIZE		0xc
#define DP_RECEIVER_CAP_SIZE			0xf
#define DP_DSC_RECEIVER_CAP_SIZE		0xf
#define EDP_PSR_RECEIVER_CAP_SIZE		2
#define EDP_DISPLAY_CTL_CAP_SIZE		3

#endif /* __MTGPU_DPCD_DEFS_H__ */
