/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/time.h>

#include "mtgpu.h"
#include "mtgpu_fec_dbg.h"

#define FEC_FW_CTRL_REBOOT	((__force int __bitwise)0)	/* fec only warm reboot */
#define FEC_FW_CTRL_STANDBY	((__force int __bitwise)1)	/* fec suspend to ram */
#define FEC_FW_CTRL_L_IDLE	((__force int __bitwise)2)	/* fec long idle */
#define FEC_FW_CTRL_WAKEUP	((__force int __bitwise)3)	/* fec wake from suspend/idle */
#define FEC_FW_CTRL_MIN		FEC_FW_CTRL_REBOOT
#define FEC_FW_CTRL_MAX		((__force int __bitwise)4)

struct mtgpu_ipc_info;

static const char * const fec_ctrl_labels[] = {
	[FEC_FW_CTRL_REBOOT] = "reboot",
	[FEC_FW_CTRL_STANDBY] = "standby",
	[FEC_FW_CTRL_L_IDLE] = "idle",
	[FEC_FW_CTRL_WAKEUP] = "wakeup",
};
#define FEC_FW_CTRL_DEFAULT_STATE	FEC_FW_CTRL_WAKEUP

void mtgpu_fec_dump_handler(struct device *dev, struct fec_dbg_info *dbg_info, void *data)
{
	struct fec_dbg_event *event = data;
	loff_t pos;
	char *fname;
	struct file *f;
	int ret, len;
	int open_flags = O_CREAT | O_RDWR | O_NOFOLLOW |
			 O_LARGEFILE | O_EXCL;

	if (dbg_info) {
		fname = dbg_info->d_name;
		f = dbg_info->d_file;
	} else {
		dev_err(dev, "fec-dump failed to get valid info\n");
		goto err_exit;
	}

	switch (event->sub_cmd) {
	case FEC_DUMP_START:
		fname = kasprintf(GFP_KERNEL, "/var/log/mtgpu-fe.%s-%08lld.dump",
				  dev_name(dev), ktime_get_mono_fast_ns());
		if (!fname)
			goto file_err_exit;

		f = filp_open(fname, open_flags, 0644);
		if (IS_ERR_OR_NULL(f))
			goto file_err_exit;
		else
			dev_warn(dev, "fec-dump file %s created\n", fname);

		dbg_info->d_file = f;
		dbg_info->d_name = fname;
		break;
	case FEC_DUMP_END:
		if (IS_ERR_OR_NULL(f) || IS_ERR_OR_NULL(fname))
			goto file_err_exit;

		filp_close(f, 0);
		dev_warn(dev, "fec-dump file %s completed\n", fname);
		kfree(fname);
		dbg_info->d_file = NULL;
		dbg_info->d_name = NULL;
		break;
	case FEC_DUMP_DATA:
		if (IS_ERR_OR_NULL(f))
			goto file_err_exit;

		len = event->len;
		pos = event->pos;
		ret = kernel_write(f, event->data, len, &pos);
		break;
	default:
		dev_warn(dev, "fec dump err cmd [%d]\n", event->sub_cmd);
		break;
	}

	return;

file_err_exit:
	dev_err(dev, "fec-dump file process cmd %d failed\n", event->sub_cmd);
err_exit:
	return;
}

static ssize_t debugfs_fec_dbg_enable_read(struct file *file, char __user *buf,
					   size_t size, loff_t *offset)
{
	struct fec_dbg_info *dbg_info = file->private_data;

	if (!dbg_info)
		return -ENODEV;

	return simple_read_from_buffer(buf, 2, offset, dbg_info->dump_enable ? "1\n" : "0\n", 2);
}

static ssize_t debugfs_fec_dbg_enable_write(struct file *file, const char __user *buf,
					    size_t size, loff_t *offset)
{
	struct fec_dbg_info *dbg_info = file->private_data;
	int ret, req;
	bool enable;

	if (!dbg_info)
		return -ENODEV;

	ret = kstrtobool_from_user(buf, size, &enable);
	if (ret)
		return ret;
	dbg_info->dump_enable = enable;
	req = enable ? FEC_REQUEST_COREDUMP_EN : FEC_REQUEST_COREDUMP_DIS;
	ret = mtgpu_fec_dbg_do_request(dbg_info->dev, req, NULL, 0);

	if (ret)
		return -EFAULT;

	return size;
}

static const struct file_operations debugfs_fec_dump_en_ops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = debugfs_fec_dbg_enable_read,
	.write = debugfs_fec_dbg_enable_write,
};

/* show the supported fec fw ctrl cmd strings */
static ssize_t debugfs_fec_fw_ctrl_read(struct file *file, char __user *buf,
					size_t size, loff_t *offset)
{
	struct fec_dbg_info *dbg_info = file->private_data;
	int __bitwise i;
	char s[64];
	int len;

	/* use "[]" to mark the current fw_ctrl status */
	for (i = FEC_FW_CTRL_MIN, len = 0; i < FEC_FW_CTRL_MAX; i++) {
		if (fec_ctrl_labels[i]) {
			if (i == dbg_info->current_fw_ctrl_state)
				len += snprintf(s + len, sizeof(s) - len, "[%s] ", fec_ctrl_labels[i]);
			else
				len += snprintf(s + len, sizeof(s) - len, "%s ", fec_ctrl_labels[i]);
		}
	}

	if (len <= *offset)
		return 0;

	/* convert last blank space to newline */
	s[len - 1] = '\n';

	if (copy_to_user(buf, s, len))
		return -EFAULT;
	*offset += len;

	return len;
}

static ssize_t debugfs_fec_fw_ctrl_write(struct file *file, const char __user *buf,
					 size_t size, loff_t *offset)
{
	struct fec_dbg_info *dbg_info = file->private_data;
	char s[64] = {0};
	int __bitwise i, event;
	int ret;

	if (size >= sizeof(s) || copy_from_user(s, buf, size))
		return -EFAULT;

	event = FEC_FW_CTRL_MAX;
	for (i = FEC_FW_CTRL_MIN; i < FEC_FW_CTRL_MAX; i++) {
		const char *label = fec_ctrl_labels[i];

		if (label && sysfs_streq(s, label)) {
			event = i;
			break;
		}
	}

	if (event == dbg_info->current_fw_ctrl_state)
		return -EINVAL;

	switch (event) {
	case FEC_FW_CTRL_REBOOT:
		ret = mtgpu_fec_warm_reboot(dbg_info->dev);
		/* If reboot success, put label as wakeup */
		if (!ret)
			event = FEC_FW_CTRL_WAKEUP;
		break;
	case FEC_FW_CTRL_STANDBY:
		ret = mtgpu_fec_request_standby(dbg_info->dev);
		break;
	case FEC_FW_CTRL_L_IDLE:
		ret = mtgpu_fec_request_idle(dbg_info->dev);
		break;
	case FEC_FW_CTRL_WAKEUP:
		if (dbg_info->current_fw_ctrl_state == FEC_FW_CTRL_STANDBY)
			ret = mtgpu_fec_request_wakeup(dbg_info->dev, false);
		else if (dbg_info->current_fw_ctrl_state == FEC_FW_CTRL_L_IDLE)
			ret = mtgpu_fec_request_wakeup(dbg_info->dev, true);
		else
			ret = -EINVAL;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (!ret)
		dbg_info->current_fw_ctrl_state = event;

	return size;
}

static const struct file_operations debugfs_fec_pwr_ctrl_ops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = debugfs_fec_fw_ctrl_read,
	.write = debugfs_fec_fw_ctrl_write,
};

static void mtgpu_fec_dbg_get_timeofday(char *utc_time)
{
	struct tm tm;

	time64_to_tm(ktime_get_real_seconds(), 0, &tm);
	sprintf(utc_time, "%04ld-%02d-%02d-%02d:%02d:%02d",
		tm.tm_year+1900,
		tm.tm_mon+1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec);
}

void mtgpu_fec_dbg_write_umd_log(struct device *dev, int size, u8 *data)
{
	struct mtgpu_device *mtdev = dev_get_drvdata(dev);
	struct fec_dbg_info *dbg_info = mtgpu_fec_get_dbg_info_from_mtdev(mtdev);
	char utc_time[32], fname[128], buffer[512];
	int len, ret;
	loff_t pos;
	int open_flags = O_CREAT | O_RDWR | O_NOFOLLOW | O_LARGEFILE| O_APPEND;

	mtgpu_fec_dbg_get_timeofday(utc_time);

	if (dbg_info->umd_log_file && dbg_info->umd_log_file->f_inode->i_size < 0x100000) {
		len = sprintf(buffer, "[%s]: %s\n", utc_time, data);
		if (len < 0)
			return;

		kernel_write(dbg_info->umd_log_file, buffer, len, &pos);
	} else {
		if (dbg_info->umd_log_file)
			filp_close(dbg_info->umd_log_file, 0);

		ret = sprintf(fname, "/var/log/mtgpu-fec-%s-%s.log",
			      dev_name(dev), utc_time);
		if (ret < 0)
			return;

		dbg_info->umd_log_file = filp_open(fname, open_flags, 0644);
		if (IS_ERR_OR_NULL(dbg_info->umd_log_file)) {
			dbg_info->umd_log_file = NULL;
			dev_err(dev, "open fec log file failed!\n");
			return;
		}
	}
}

int mtgpu_fec_dbg_init(struct device *dev, struct fec_dbg_info **dbg_info)
{
	char name[32];
	struct fec_dbg_info *tmp = *dbg_info;

	tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	tmp->dev = dev;

	sprintf(name, "card%d-fec", 0);
	tmp->d_fec_root = debugfs_create_dir(name, mtgpu_dentry);
	tmp->d_dump_enable = debugfs_create_file("dump_enable", 0644, tmp->d_fec_root,
						 tmp, &debugfs_fec_dump_en_ops);
	tmp->d_fw_ctrl = debugfs_create_file("fw_ctrl", 0644, tmp->d_fec_root,
					     tmp, &debugfs_fec_pwr_ctrl_ops);
	tmp->current_fw_ctrl_state = FEC_FW_CTRL_DEFAULT_STATE;

	*dbg_info = tmp;

	return 0;
}

void mtgpu_fec_dbg_deinit(struct device *dev, struct fec_dbg_info *dbg_info)
{
	if (dbg_info->umd_log_file)
		filp_close(dbg_info->umd_log_file, 0);
}
