// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_DRV_H
#define _MTSND_DRV_H

#include "os-interface.h"

#define PCI_VENDOR_ID_MTSND		(0x1ED5)
#define DEVICE_ID_GEN1_SND		(0x01FF)
#define DEVICE_ID_GEN2_SND		(0x02FF)
#define DEVICE_ID_GEN3_SND		(0x03FF)
#define DEVICE_ID_GEN1_ANY		(0x0100)
#define DEVICE_ID_GEN2_ANY		(0x0200)
#define DEVICE_ID_GEN3_ANY		(0x0300)
#define MSG_BUFFER_SIZE			(512)

struct device;
struct hdmi_codec_pdata;
struct parsed_hdmi_eld;
struct ipc_msg;
struct snd_conf;
struct snd_card;
struct pci_dev;
struct snd_jack;
struct snd_kcontrol;
struct snd_pcm_substream;
struct snd_compr_stream;
struct dentry;
struct mutex;
struct hdmi_codec_params;
struct hdmi_codec_daifmt;
struct snd_ctl_elem_value;
struct mtsnd_timer;

struct mtsnd_bar {
	unsigned long paddr;
	void __iomem *vaddr;
};

struct mtsnd_codec {
	u32 c_state;
	struct device *dev;
	struct hdmi_codec_pdata *hcd;
	struct parsed_hdmi_eld *eld;
	struct hdmi_codec_params *params;
	struct hdmi_codec_daifmt *daifmt;
};

struct mtsnd_pcm {
	u32 index;

	u32 open_pcm: 1;
	u32 pcm_running: 1;
	unsigned long pcm_device_addr;
	u64 iis_clock;

	void *private_data;
	struct snd_pcm_substream *substream;
	/* ipc data buffer */
	struct ipc_msg *ipc_msg_buffer;
	/* pointer to codecs of pcm stream */
	struct mtsnd_codec *codec[4];
};

/* card instance */
struct mtsnd_chip {
	/* I/O resources */
	struct mtsnd_bar bar[2];
	int irq[4];
	int idx;

	u32 open_compr: 1;

	/* dev convert addr */
	unsigned long compr_device_addr;
	int compr_send_idx;
	int compr_resp_idx;

	/* chip config */
	const struct snd_conf *conf;

	struct snd_card *card;
	struct pci_dev *pci;
	struct device *mt_dev;

	/* jack/kcontrol */
	struct snd_jack *jack[4];
	struct snd_kcontrol *kctl[4];
	struct mtsnd_timer *jack_timer[4];

	struct snd_compr_stream *compr_stream;
	struct dentry *debug;

	/* codec */
	struct mtsnd_codec codec[4];
	/* pcm */
	struct mtsnd_pcm pcm[4];

	struct wait_queue_head *compr_sleep;
};

extern int mtsnd_create_pcm(struct mtsnd_chip *chip);
extern void mtsnd_free_pcm(struct mtsnd_chip *chip);
extern void mtsnd_reset_pcm(struct mtsnd_chip *chip);
extern void mtsnd_handle_pcm(struct mtsnd_pcm *pcm);
extern void mtsnd_suspend_pcm(struct mtsnd_chip *chip);
extern void mtsnd_resume_pcm(struct mtsnd_chip *chip);

extern int mtsnd_create_compr(struct mtsnd_chip *chip);
extern void mtsnd_free_compr(struct mtsnd_chip *chip);
extern void mtsnd_reset_compr(struct mtsnd_chip *chip);
extern void mtsnd_handle_compr(struct mtsnd_chip *chip);

extern int mtsnd_create_codec(struct mtsnd_chip *chip);
extern void mtsnd_free_codec(struct mtsnd_chip *chip);
extern int mtsnd_mixer_switch_get(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol);
extern int mtsnd_mixer_switch_put(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol);
extern void mtsnd_codec_hot_plug_dp0(struct device *dev, bool connected);
extern void mtsnd_codec_hot_plug_dp1(struct device *dev, bool connected);
extern void mtsnd_codec_hot_plug_dp2(struct device *dev, bool connected);
extern void mtsnd_codec_hot_plug_dp3(struct device *dev, bool connected);
extern void mtsnd_codec_hot_plug_hdmi(struct device *dev, bool connected);

extern int mtsnd_init_msg(struct mtsnd_chip *chip);
extern int mtsnd_check_eld(struct mtsnd_chip *chip, int codec_index);

extern struct device *find_device_by_name(struct mtsnd_chip *chip, char *name);

#if defined (OS_BUS_FIND_DEVICE_MATCH_USE_CONST_MODIFIER)
extern int pdev_match_name(struct device *dev, const void *data);
#else
extern int pdev_match_name(struct device *dev, void *data);
#endif

extern int mtsnd_init(void);
extern void mtsnd_deinit(void);

#endif /* _MTSND_DRV_H */
