// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/msi.h>
#include <linux/version.h>
#include <linux/debugfs.h>

#include <sound/core.h>
#include <sound/info.h>
#include <sound/jack.h>
#include <sound/initval.h>
#include <sound/hdmi-codec.h>

#include "mtsnd_drv.h"
#include "mtsnd_irq.h"
#include "mtsnd_conf.h"
#include "mtsnd_debug.h"
#include "mtgpu_drv.h"

#define DRVNAME	"mtsnd"

static int disable_audio;
module_param(disable_audio, int, 0444);
MODULE_PARM_DESC(disable_audio,
		 "mtgpu disable audio function enable(0)/disable(others)");

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for MT soundcard.");

static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for MT soundcard.");

unsigned int snd_debug = 0x7;
module_param(snd_debug, int, 0444);
MODULE_PARM_DESC(snd_debug,
		 "mtsnd audio debug print control");

static int mtsnd_dev_free(struct snd_device *device)
{
	struct mtsnd_chip *chip = (struct mtsnd_chip *)device->device_data;

#ifdef INTERRUPT_DEBUG
	free_irq(chip->irq, chip);
#else
	mtgpu_disable_interrupt(chip->mt_dev, chip->irq);
	if (mtgpu_set_interrupt_handler(chip->mt_dev, chip->irq, NULL, NULL))
		dev_warn(chip->card->dev, "free step irq func clean error\n");
#endif

	mtsnd_free_pcm(chip);
	mtsnd_free_compr(chip);
	mtsnd_free_codec(chip);

	iounmap(chip->bar[0].vaddr);
	iounmap(chip->bar[1].vaddr);

	pci_release_regions(chip->pci);
	pci_disable_device(chip->pci);

	if (chip->debug)
		debugfs_remove_recursive(chip->debug);

	kfree(chip->ipc_msg_buffer);
	kfree(chip);

	return 0;
}

#ifdef INTERRUPT_DEBUG
static irqreturn_t mtsnd_irq_handle(int irq, void *dev_id)
{
	struct mtsnd_chip *chip = dev_id;
	unsigned int src = get_pcm_compr_irq(chip);

	if (check_pcm_irq(chip, src))
		mtsnd_handle_pcm(chip);

	if (check_compr_irq(chip, src))
		mtsnd_handle_compr(chip);

	clear_pcm_compr_irq(chip, src);
	return IRQ_HANDLED;
}
#else
static void mtsnd_irq_handle(void *dev_id)
{
	struct mtsnd_chip *chip = dev_id;

	mtsnd_handle_pcm(chip);
}
#endif

static int mtsnd_create(struct snd_card *card, struct pci_dev *pci,
		     struct mtsnd_chip **rchip, int idx)
{
	int err;
	struct mtsnd_chip *chip;
#ifndef INTERRUPT_DEBUG
	struct platform_device *pdev;
	struct resource *res;
	struct device *dev;
#endif
	static struct snd_device_ops ops = {
		.dev_free = mtsnd_dev_free,
	};

	err = pci_enable_device(pci);
	if (err < 0)
		return err;

	err = pci_request_regions(pci, DRVNAME);
	if (err < 0) {
		pci_disable_device(pci);
		return err;
	}

	pci_set_master(pci);

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		err = -ENOMEM;
		goto errmem;
	}

	/* int chip type */
	err = snd_init_conf(pci->device, chip);
	if (err) {
		dev_err(&pci->dev, "error device id:%d\n", pci->device);
		goto errconf;
	}

	chip->card = card;
	chip->pci = pci;
	chip->idx = idx;

	chip->bar[0].paddr = pci_resource_start(pci, 0);
	chip->bar[0].vaddr = pci_ioremap_bar(pci, 0);
	if (!chip->bar[0].vaddr) {
		dev_err(chip->card->dev, "bar 0 ioremap error\n");
		err = -ENXIO;
		goto errconf;
	}

	chip->bar[1].paddr = pci_resource_start(pci, 2);
	chip->bar[1].vaddr = pci_ioremap_bar(pci, 2);
	if (!chip->bar[1].vaddr) {
		dev_err(chip->card->dev, "bar 0 ioremap error\n");
		err = -ENXIO;
		goto errio0;
	}

	mtsnd_reset_pcm(chip);
	mtsnd_reset_compr(chip);

#ifdef INTERRUPT_DEBUG
	chip->irq = pci->irq;
	err = request_irq(chip->irq, mtsnd_irq_handle, IRQF_SHARED, KBUILD_MODNAME, chip);
	if (err < 0) {
		dev_err(chip->card->dev, "Error request_irq\n");
		goto errio1;
	}
#else
	dev = find_device_by_name(chip, "mtgpu_audio");
	if (!dev) {
		dev_err(chip->card->dev, "Error find_device_by_name\n");
		err = -ENODEV;
		goto errio1;
	}

	pdev = to_platform_device(dev);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		dev_err(chip->card->dev, "Error platform_get_resource\n");
		err = -ENODATA;
		goto errio1;
	};

	err = mtgpu_set_interrupt_handler(pdev->dev.parent, res->start, mtsnd_irq_handle, chip);
	if (err < 0) {
		dev_err(chip->card->dev, "Error mtgpu_set_interrupt_handler\n");
		goto errio1;
	}

	err = mtgpu_enable_interrupt(pdev->dev.parent, res->start);
	if (err < 0) {
		dev_err(chip->card->dev, "Error mtgpu_enable_interrupt\n");
		goto errirq_enable;
	}

	chip->mt_dev = pdev->dev.parent;
	chip->irq = res->start;
#endif

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);
	if (err < 0) {
		dev_err(chip->card->dev, "Error creating device [card]\n");
		goto errsnd_new;
	}

	strcpy(card->driver, "MooreThreads");
	strcpy(card->shortname, "MooreThreads");
	strcpy(card->mixername, card->shortname);
	if (get_chip_type(chip) == CHIP_GEN1)
		strcpy(card->longname, "MooreThreadsGen1");
	else if (get_chip_type(chip) == CHIP_GEN2)
		strcpy(card->longname, "MooreThreadsGen2");

	*rchip = chip;
	return 0;

errsnd_new:
#ifndef INTERRUPT_DEBUG
	mtgpu_disable_interrupt(pdev->dev.parent, res->start);
errirq_enable:
	if (mtgpu_set_interrupt_handler(pdev->dev.parent, res->start, NULL, NULL))
		dev_warn(chip->card->dev, "irq func clean error\n");
#endif
errio1:
	iounmap(chip->bar[1].vaddr);
errio0:
	iounmap(chip->bar[0].vaddr);
errconf:
	kfree(chip);
errmem:
	pci_disable_device(pci);
	pci_release_regions(pci);

	return err;
}

static ssize_t jack_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct mtsnd_chip *chip = file_inode(file)->i_private;
	char str[256] = {0};
	int len;

	if (*ppos > 0)
		return 0;

	switch (get_chip_type(chip)) {
	case CHIP_GEN1:
		len = sprintf(str, "Gen1, log level: %x\ndp0 %x, dp1 %x, hdmi %x\n",
			      snd_debug, chip->codec[0].c_state, chip->codec[1].c_state,
			      chip->codec[2].c_state);
		break;

	case CHIP_GEN2:
		len = sprintf(str, "Gen2, log level: %x\ndp0 %x, dp1 %x, dp2 %x, hdmi %x\n",
			      snd_debug, chip->codec[0].c_state, chip->codec[1].c_state,
			      chip->codec[2].c_state, chip->codec[3].c_state);
		break;

	default:
		len = sprintf(str, "Error config for this card or audio driver crash!\n");
		break;
	}

	if (copy_to_user(buf, str, len))
		return -EFAULT;
	*ppos += len;

	return *ppos;
}

static ssize_t jack_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct mtsnd_chip *chip = file_inode(file)->i_private;
	char str[256] = {0};
	char name[32] = {0};
	int num;

	if (copy_from_user(str, buf, count))
		return -EFAULT;

	if (sscanf(str, "%s %d", name, &num) == 2) {
		if (strcmp(name, "log") == 0) {
			SND_INFO("runtime change the snd log level %x -> %x\n", snd_debug, num);
			snd_debug = num;
		}
		if (strcmp(name, "dp0") == 0)
			snd_jack_report(chip->jack[0], num > 0 ? SND_JACK_AVOUT : 0);
		if (strcmp(name, "dp1") == 0)
			snd_jack_report(chip->jack[1], num > 0 ? SND_JACK_AVOUT : 0);
		switch (get_chip_type(chip)) {
		case CHIP_GEN1:
			if (strcmp(name, "hdmi") == 0)
				snd_jack_report(chip->jack[2], num > 0 ? SND_JACK_AVOUT : 0);
			break;

		case CHIP_GEN2:
			if (strcmp(name, "dp2") == 0)
				snd_jack_report(chip->jack[2], num > 0 ? SND_JACK_AVOUT : 0);
			if (strcmp(name, "hdmi") == 0)
				snd_jack_report(chip->jack[3], num > 0 ? SND_JACK_AVOUT : 0);
		break;

		default:
			break;
		}
	}

	return count;
}

static struct file_operations jack_fops = {
	.owner = THIS_MODULE,
	.read = jack_read,
	.write = jack_write,
};

static int mtsnd_probe(struct pci_dev *pci, const struct pci_device_id *pci_id)
{
	static int idx;
	struct snd_card *card = NULL;
	struct mtsnd_chip *chip = NULL;
	char name[32];
	int err;

	if (idx >= SNDRV_CARDS)
		return -ENODEV;

	err = snd_card_new(&pci->dev, index[idx], id[idx], THIS_MODULE, 0, &card);
	if (err < 0) {
		dev_err(&pci->dev, "Error snd_card_new\n");
		return err;
	}

	err = mtsnd_create(card, pci, &chip, idx);
	if (err < 0) {
		dev_err(&pci->dev, "Error mtsnd_create\n");
		goto out_free;
	}

	chip->idx = idx;
	card->private_data = chip;
	pci_set_drvdata(pci, card);

	/* init the rb for communicating with smc */
	chip->ipc_msg_buffer = kzalloc(MSG_BUFFER_SIZE, GFP_KERNEL);
	if (!chip->ipc_msg_buffer) {
		err = -ENOMEM;
		goto out_free;
	}

	err = mtsnd_create_compr(chip);
	if (err < 0) {
		dev_err(&pci->dev, "Error mtsnd_create_compr\n");
		goto out_free;
	}

	err = mtsnd_create_pcm(chip);
	if (err < 0) {
		dev_err(&pci->dev, "Error mtsnd_create_pcm\n");
		goto out_free;
	}

	err = mtsnd_create_codec(chip);
	if (err < 0) {
		dev_err(&pci->dev, "Error mtsnd_create_codec\n");
		goto out_free;
	}

	sprintf(name, "mtsnd%d", idx);
	chip->debug = debugfs_create_dir(name, NULL);
	if (chip->debug)
		debugfs_create_file("jack", 0666, chip->debug, chip, &jack_fops);

	err = snd_card_register(card);
	if (err < 0) {
		dev_err(&pci->dev, "Error snd_card_register\n");
		goto out_free;
	}

	idx++;
	return 0;

out_free:
	snd_card_free(card);
	return err;
}

static void mtsnd_remove(struct pci_dev *pci)
{
	snd_card_free(pci_get_drvdata(pci));
}

/* PCI IDs */
static const struct pci_device_id mtsnd_ids[] = {
	{ PCI_VENDOR_ID_MTSND, DEVICE_ID_SUDI_SND, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_MULTIMEDIA_AUDIO << 8, ~0, 0},
	{ PCI_VENDOR_ID_MTSND, DEVICE_ID_QY1_SND, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_MULTIMEDIA_AUDIO << 8, ~0, 0},
	{ PCI_VENDOR_ID_MTSND, DEVICE_ID_SUDI_ANY, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_MULTIMEDIA_AUDIO << 8, ~0, 0},
	{ PCI_VENDOR_ID_MTSND, DEVICE_ID_QY1_ANY, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_MULTIMEDIA_AUDIO << 8, ~0, 0},
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, mtsnd_ids);

#ifdef CONFIG_PM_SLEEP
static int mtsnd_pm_suspend(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct mtsnd_chip *chip = card->private_data;

	mtsnd_suspend_pcm(chip);
	return 0;
}

static int mtsnd_pm_resume(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct mtsnd_chip *chip = card->private_data;

	mtsnd_resume_pcm(chip);
	return 0;
}

static const struct dev_pm_ops mtsnd_pm_ops = {
	.suspend = mtsnd_pm_suspend,
	.resume = mtsnd_pm_resume,
	.freeze = mtsnd_pm_suspend,
	.restore = mtsnd_pm_resume,
};
#endif

/* pci_driver definition */
static struct pci_driver mtsnd_pci_driver = {
	.name = DRVNAME,
	.driver = {
#ifdef CONFIG_PM_SLEEP
		.pm = &mtsnd_pm_ops,
#endif
	},
	.id_table = mtsnd_ids,
	.probe = mtsnd_probe,
	.remove = mtsnd_remove,
};

int mtsnd_init(void)
{
	pr_info("mtsnd build info: %s\n", MT_BUILD_SND);

	if (disable_audio) {
		pr_info("mtsnd: disable audio for mtgpu\n");
	} else {
		int ret;

		ret = pci_register_driver(&mtsnd_pci_driver);
		if (unlikely(ret))
			return ret;
	}

	return 0;
}

void mtsnd_deinit(void)
{
	if (!disable_audio)
		pci_unregister_driver(&mtsnd_pci_driver);
}
