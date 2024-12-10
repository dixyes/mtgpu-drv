/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/debugfs.h>

#if defined(OS_DRM_DRMP_H_EXIST)
#include <drm/drmP.h>
#endif
#if defined(OS_DRM_DRM_DP_HELPER_H_EXIST)
#include <drm/drm_dp_helper.h>
#elif defined(OS_DRM_DP_DRM_DP_HELPER_H_EXIST)
#include <drm/dp/drm_dp_helper.h>
#elif defined(OS_DRM_DISPLAY_DRM_DP_HELPER_H_EXIST)
#include <drm/display/drm_dp_helper.h>
#endif
#include <drm/drm_crtc.h>
#include <drm/drm_connector.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_print.h>

#include "mtgpu_drv.h"
#include "mtgpu_dp_common.h"
#include "mtgpu_hdmi_common.h"
#include "mtgpu_dispc_common.h"
#include "mtgpu_drm_debugfs.h"

/* link rate */
static int link_rate_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);
	struct mtgpu_dp_debugfs *debugfs = &dp->debugfs;

	seq_printf(m, "link_rate: %d.%dG\n",
		   debugfs->link_rate / 100000,
		   debugfs->link_rate % 100000 / 1000);

	return 0;
}

static int link_rate_open(struct inode *inode, struct file *file)
{
	struct drm_connector *dev = inode->i_private;

	return single_open(file, link_rate_show, dev);
}

static ssize_t link_rate_write(struct file *file, const char __user *ubuf,
			       size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);
	struct mtgpu_dp_debugfs *debugfs = &dp->debugfs;
	char *buf;
	u32 val;

	buf = memdup_user(ubuf, len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	buf[len - 1] = '\0';

	if (!strncmp(buf, "0", 1)) {
		val = 0;
	} else if (!strncmp(buf, "1.62", 4)) {
		val = drm_dp_bw_code_to_link_rate(DP_LINK_BW_1_62);
	} else if (!strncmp(buf, "2.7", 3)) {
		val = drm_dp_bw_code_to_link_rate(DP_LINK_BW_2_7);
	} else if (!strncmp(buf, "5.4", 3)) {
		val = drm_dp_bw_code_to_link_rate(DP_LINK_BW_5_4);
	} else if (!strncmp(buf, "8.1", 3)) {
		val = drm_dp_bw_code_to_link_rate(DP_LINK_BW_8_1);
	} else {
		DRM_DEV_ERROR(dp->dev, "Wrong link rate configuration, buf:'%s'.\n", buf);
		kfree(buf);
		return -EINVAL;
	}

	debugfs->link_rate = val;

	kfree(buf);

	return len;
}

static const struct file_operations mtgpu_link_rate_fops = {
	.owner = THIS_MODULE,
	.open = link_rate_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = link_rate_write
};

/* lane count */
static int lane_cnt_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);
	struct mtgpu_dp_debugfs *debugfs = &dp->debugfs;

	seq_printf(m, "lane_cnt: %d\n", debugfs->lane_cnt);

	return 0;
}

static int lane_cnt_open(struct inode *inode, struct file *file)
{
	struct drm_connector *dev = inode->i_private;

	return single_open(file, lane_cnt_show, dev);
}

static ssize_t lane_cnt_write(struct file *file, const char __user *ubuf,
			      size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);
	struct mtgpu_dp_debugfs *debugfs = &dp->debugfs;
	char *buf;
	int ret;
	u8 val;

	buf = memdup_user(ubuf, len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	buf[len - 1] = '\0';

	ret = kstrtou8(buf, 10, &val);
	if (ret) {
		DRM_DEV_ERROR(dp->dev, "failed kstrtou8: buf:'%s', ret:%d\n", buf, ret);
		kfree(buf);
		return ret;
	}

	if (val == 3 || val > 4) {
		DRM_DEV_ERROR(dp->dev, "Wrong lane count configuration.\n");
		kfree(buf);
		return -EINVAL;
	}

	debugfs->lane_cnt = val;

	kfree(buf);

	return len;
}

static const struct file_operations mtgpu_lane_cnt_fops = {
	.owner = THIS_MODULE,
	.open = lane_cnt_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = lane_cnt_write
};

/* encoder restart */
static int encoder_restart_show(struct seq_file *m, void *data)
{
	seq_puts(m, "You need execute `echo 1 > encoder_restart`\n");

	return 0;
}

static int encoder_restart_open(struct inode *inode, struct file *file)
{
	struct drm_connector *dev = inode->i_private;

	return single_open(file, encoder_restart_show, dev);
}

static ssize_t encoder_restart_write(struct file *file, const char __user *ubuf,
				     size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);
	char *buf;
	u8 val;
	int ret;

	if (!dp->enabled) {
		DRM_DEV_ERROR(dp->dev, "connector is not enabled");
		return -ENODEV;
	}

	buf = memdup_user(ubuf, len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	buf[len - 1] = '\0';

	ret = kstrtou8(buf, 10, &val);
	if (ret) {
		DRM_DEV_ERROR(dp->dev, "failed kstrtou8: buf:'%s', ret:%d\n", buf, ret);
		kfree(buf);
		return ret;
	}

	if (val == 1) {
		mtgpu_dp_encoder_disable(dp->encoder);
		mtgpu_dp_encoder_enable(dp->encoder);
	} else {
		DRM_DEV_ERROR(dp->dev, "Wrong encoder restart configuration.\n");
		kfree(buf);
		return -EINVAL;
	}

	kfree(buf);

	return len;
}

static const struct file_operations mtgpu_encoder_restart_fops = {
	.owner = THIS_MODULE,
	.open = encoder_restart_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = encoder_restart_write
};

/* edid lock */
static int dp_edid_lock_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);

	seq_printf(m, "edid locked: %d\n", dp->fixed_edid);

	return 0;
}

static int dp_edid_lock_open(struct inode *inode, struct file *file)
{
	struct drm_connector *dev = inode->i_private;

	return single_open(file, dp_edid_lock_show, dev);
}

static ssize_t dp_edid_lock_write(struct file *file, const char __user *ubuf,
				  size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_connector *connector = m->private;
	struct mtgpu_dp *dp = connector_to_mtgpu_dp(connector);
	char *buf;
	u8 val;
	int ret;

	if (!dp->enabled) {
		DRM_DEV_ERROR(dp->dev, "connector is not enabled");
		return -ENODEV;
	}

	buf = memdup_user(ubuf, len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	buf[len - 1] = '\0';

	ret = kstrtou8(buf, 10, &val);
	if (ret) {
		DRM_DEV_ERROR(dp->dev, "failed kstrtou8: buf:'%s', ret:%d\n", buf, ret);
		kfree(buf);
		return ret;
	}

	switch (val) {
	case 0:
	case 1:
		if (mtgpu_dp_update_fixed_edid_flag(dp, (bool)val))
			DRM_DEV_ERROR(dp->dev, "set edid lock failed!\n");
		break;
	default:
		DRM_DEV_ERROR(dp->dev, "Wrong edid lock configuration.\n");
		kfree(buf);
		return -EINVAL;
	}

	kfree(buf);
	return len;
}

static const struct file_operations mtgpu_dp_edid_lock_fops = {
	.owner = THIS_MODULE,
	.open = dp_edid_lock_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = dp_edid_lock_write
};

void mtgpu_dp_debugfs_create_files(struct mtgpu_dp *dp)
{
	struct dentry *dentry;
	int idx = dp->connector->dev->primary->index;
	char name[32];

	sprintf(name, "card%d-%s", idx, dp->connector->name);

	dentry = debugfs_create_dir(name, mtgpu_dentry);
	dp->debugfs.dentry = dentry;

	/* link rate */
	debugfs_create_file("link_rate", 0644, dentry, dp->connector,
			    &mtgpu_link_rate_fops);

	/* lane count */
	debugfs_create_file("lane_cnt", 0644, dentry, dp->connector,
			    &mtgpu_lane_cnt_fops);

	/* encoder restart */
	debugfs_create_file("encoder_restart", 0644, dentry, dp->connector,
			    &mtgpu_encoder_restart_fops);

	memset(name, 0, sizeof(name));

	sprintf(name, "edid-lock_card%d-%s", idx, dp->connector->name);

	dentry = debugfs_create_dir(name, mtgpu_dentry);
	dp->debugfs.dentry = dentry;

	/* edid lock */
	debugfs_create_file("edid_lock", 0644, dentry, dp->connector,
			    &mtgpu_dp_edid_lock_fops);
}

void mtgpu_dp_debugfs_remove_files(struct mtgpu_dp *dp)
{
	struct dentry *dentry = dp->debugfs.dentry;

	debugfs_remove_recursive(dentry);
}

/* edid lock */
static int hdmi_edid_lock_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);

	seq_printf(m, "edid locked: %d\n", hdmi->fixed_edid);

	return 0;
}

static int hdmi_edid_lock_open(struct inode *inode, struct file *file)
{
	struct drm_connector *dev = inode->i_private;

	return single_open(file, hdmi_edid_lock_show, dev);
}

static ssize_t hdmi_edid_lock_write(struct file *file, const char __user *ubuf,
				    size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_connector *connector = m->private;
	struct mtgpu_hdmi *hdmi = connector_to_mtgpu_hdmi(connector);
	char *buf;
	u8 val;
	int ret;

	if (!hdmi->enabled) {
		DRM_DEV_ERROR(hdmi->dev, "connector is not enabled");
		return -ENODEV;
	}

	buf = memdup_user(ubuf, len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	buf[len - 1] = '\0';

	ret = kstrtou8(buf, 10, &val);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "failed kstrtou8: buf:'%s', ret:%d\n", buf, ret);
		kfree(buf);
		return ret;
	}

	switch (val) {
	case 0:
	case 1:
		if (mtgpu_hdmi_update_fixed_edid_flag(hdmi, (bool)val))
			DRM_DEV_ERROR(hdmi->dev, "set edid lock failed!\n");
		break;
	default:
		DRM_DEV_ERROR(hdmi->dev, "Wrong edid lock configuration.\n");
		kfree(buf);
		return -EINVAL;
	}

	kfree(buf);

	return len;
}

static const struct file_operations mtgpu_hdmi_edid_lock_fops = {
	.owner = THIS_MODULE,
	.open = hdmi_edid_lock_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = hdmi_edid_lock_write
};

void mtgpu_hdmi_debugfs_create_files(struct mtgpu_hdmi *hdmi)
{
	struct dentry *dentry;
	int idx = hdmi->connector->dev->primary->index;
	char name[32];

	sprintf(name, "edid-lock_card%d-%s", idx, hdmi->connector->name);

	dentry = debugfs_create_dir(name, mtgpu_dentry);
	hdmi->debugfs.dentry = dentry;

	/* edid lock */
	debugfs_create_file("edid_lock", 0644, dentry, hdmi->connector,
			    &mtgpu_hdmi_edid_lock_fops);
}

void mtgpu_hdmi_debugfs_remove_files(struct mtgpu_hdmi *hdmi)
{
	struct dentry *dentry = hdmi->debugfs.dentry;

	debugfs_remove_recursive(dentry);
}

/* underrun count */
static int underrun_cnt_show(struct seq_file *m, void *data)
{
	struct mtgpu_dispc *dispc = m->private;
	struct mtgpu_dispc_debugfs *debugfs = &dispc->debugfs;

	seq_printf(m, "%llu\n", debugfs->underrun_cnt);

	return 0;
}

static int underrun_cnt_open(struct inode *inode, struct file *file)
{
	struct mtgpu_dispc *dispc = inode->i_private;

	return single_open(file, underrun_cnt_show, dispc);
}

static ssize_t underrun_cnt_write(struct file *file, const char __user *ubuf,
				  size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct mtgpu_dispc *dispc = m->private;
	struct mtgpu_dispc_debugfs *debugfs = &dispc->debugfs;
	char *buf;
	u64 val;
	int ret;

	buf = memdup_user(ubuf, len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	buf[len - 1] = '\0';

	ret = kstrtou64(buf, 10, &val);
	if (ret) {
		DRM_DEV_ERROR(dispc->dev, "failed kstrtou64: buf:'%s', ret:%d\n", buf, ret);
		kfree(buf);
		return ret;
	}

	debugfs->underrun_cnt = val;

	kfree(buf);

	return len;
}

static const struct file_operations mtgpu_underrun_cnt_fops = {
	.owner = THIS_MODULE,
	.open = underrun_cnt_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = underrun_cnt_write,
};

void mtgpu_dispc_debugfs_create_files(struct mtgpu_dispc *dispc)
{
	struct dentry *dentry;
	int idx = dispc->crtc->dev->primary->index;
	char name[32];

	sprintf(name, "card%d-%s", idx, dispc->crtc->name);

	dentry = debugfs_create_dir(name, mtgpu_dentry);
	dispc->debugfs.dentry = dentry;

	/* underrun count */
	debugfs_create_file("underrun_cnt", 0644, dentry, dispc,
			    &mtgpu_underrun_cnt_fops);
}

void mtgpu_dispc_debugfs_remove_files(struct mtgpu_dispc *dispc)
{
	struct dentry *dentry = dispc->debugfs.dentry;

	debugfs_remove_recursive(dentry);
}
