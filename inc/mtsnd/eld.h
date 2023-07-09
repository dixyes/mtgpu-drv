// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * Notice: Some of functions in this file are ported from linux kernel code.
 */

#ifndef _ELD_H
#define _ELD_H

#define ELD_FIXED_BYTES	20
#define ELD_MAX_SIZE    256
#define ELD_MAX_MNL	16
#define ELD_MAX_SAD	16

/*
 * CEA Short Audio Descriptor data
 */
struct cea_snd {
	int	channels;
	int	format;		/* (format == 0) indicates invalid SAD */
	int	rates;
	int	sample_bits;	/* for LPCM */
	int	max_bitrate;	/* for AC3...ATRAC */
	int	profile;	/* for WMAPRO */
};

/*
 * ELD: EDID Like Data
 */
struct parsed_hdmi_eld {
	/*
	 * all fields will be cleared before updating ELD
	 */
	int	baseline_len;
	int	eld_ver;
	int	cea_edid_ver;
	char monitor_name[ELD_MAX_MNL + 1];
	int	manufacture_id;
	int	product_id;
	unsigned long long	port_id;
	int	support_hdcp;
	int	support_ai;
	int	conn_type;
	int	aud_synch_delay;
	int	spk_alloc;
	int	sad_count;
	struct cea_snd sad[ELD_MAX_SAD];
};

#endif /* _ELD_H */
