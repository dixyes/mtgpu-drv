/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/namei.h>

#include <drm/drm_crtc.h>
#include <drm/drm_connector.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_print.h>
#include <drm/drm_edid.h>

#include "mtgpu_drm_utils.h"

void mtgpu_drm_get_edid_vendor(const struct edid *edid, char *edid_vendor)
{
	edid_vendor[0] = ((edid->mfg_id[0] & 0x7c) >> 2) + '@';
	edid_vendor[1] = (((edid->mfg_id[0] & 0x3) << 3) |
			  ((edid->mfg_id[1] & 0xe0) >> 5)) + '@';
	edid_vendor[2] = (edid->mfg_id[1] & 0x1f) + '@';
	edid_vendor[3] = '\0';
}

u32 mtgpu_drm_get_edid_productid(const struct edid *edid)
{
	return EDID_PRODUCT_ID(edid);
}

u32 mtgpu_drm_edid_size(struct edid *edid)
{
	return (edid->extensions + 1) * EDID_LENGTH;
}

/* skip some process that run in initramfs
 * which can not read rootfs.
 */
bool mtgpu_drm_get_fixedflag_from_fs(void)
{
	char *comm = current->comm;

	if (!strncmp(comm, "Xorg", strlen("Xorg")) ||
	    !strncmp(comm, "modetest", strlen("modetest")))
		return true;

	return false;
}

bool mtgpu_drm_is_fwpath_exist(const char *filename)
{
	struct path path;

	if (!filename || !*filename)
		return false;

	if (kern_path(filename, LOOKUP_FOLLOW, &path)) {
		DRM_INFO(" %s not exist.\n", filename);
		return false;
	}

	path_put(&path);
	return true;
}

void *mtgpu_drm_get_fixed_edid(struct drm_connector *connector)
{
	char *filename;
	int len;
	ssize_t bytes = 0;
	loff_t pos = 0;
	void *edid = NULL;
	struct file *fp = NULL;
	int idx = connector->dev->primary->index;

	filename = __getname();
	if (!filename)
		return NULL;

	len = snprintf(filename, PATH_MAX, "%s/card%d-%s-edid.bin", EDID_LOCK_PATH,
		       idx, connector->name);
	if (len >= PATH_MAX)
		goto read_exit;

	if (!mtgpu_drm_is_fwpath_exist(filename))
		goto read_exit;

	fp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		fp = NULL;
		goto read_exit;
	}

	edid = kmalloc(512, GFP_KERNEL);
	if (!edid) {
		filp_close(fp, NULL);
		goto read_exit;
	}

	bytes = kernel_read(fp, edid, 512, &pos);
	if (bytes <= 0) {
		kfree(edid);
		edid = NULL;
	}

	filp_close(fp, NULL);
read_exit:
	__putname(filename);
	return edid;
}

void mtgpu_drm_set_fixed_edid(struct drm_connector *connector, void *edid, size_t count)
{
	char *filename;
	int len;
	struct file *fp = NULL;
	loff_t pos = 0;
	int idx = connector->dev->primary->index;

	if (!mtgpu_drm_is_fwpath_exist(EDID_LOCK_PATH))
		return;

	filename = __getname();
	if (!filename)
		return;

	len = snprintf(filename, PATH_MAX, "%s/card%d-%s-edid.bin", EDID_LOCK_PATH,
		       idx, connector->name);
	if (len >= PATH_MAX)
		goto write_exit;

	fp = filp_open(filename, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		fp = NULL;
		goto write_exit;
	}

	if (edid && count)
		kernel_write(fp, edid, count, &pos);

	filp_close(fp, NULL);

write_exit:
	__putname(filename);
}

bool  mtgpu_drm_get_fixed_edid_flag(struct drm_connector *connector)
{
	char *filename;
	int len;
	loff_t pos = 0;
	char op[16];
	struct file *fp = NULL;
	int idx = connector->dev->primary->index;
	bool ret = true;

	filename = __getname();
	if (!filename)
		return false;

	len = snprintf(filename, PATH_MAX, "%s/card%d-%s-fixed.bin", EDID_LOCK_PATH,
		       idx, connector->name);
	if (len >= PATH_MAX)
		goto read_exit;

	if (!mtgpu_drm_is_fwpath_exist(filename)) {
		ret = false;
		goto read_exit;
	}

	fp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		fp = NULL;
		ret = false;
		goto read_exit;
	}

	if (kernel_read(fp, op, 16, &pos) <= 0)
		ret = false;

	filp_close(fp, NULL);

read_exit:
	if (!strncmp(op, "enable", 6))
		ret = true;
	else
		ret = false;

	DRM_INFO("get fixed flag from %s: ret:%d.\n", filename, ret);
	__putname(filename);

	return ret;
}

void mtgpu_drm_set_fixed_edid_flag(struct drm_connector *connector, bool enable)
{
	char *filename;
	int len;
	struct file *fp = NULL;
	loff_t pos = 0;
	int idx = connector->dev->primary->index;
	char *op = enable ? "enable" : "disable";

	if (!mtgpu_drm_is_fwpath_exist(EDID_LOCK_PATH))
		return;

	filename = __getname();
	if (!filename)
		return;

	len = snprintf(filename, PATH_MAX, "%s/card%d-%s-fixed.bin", EDID_LOCK_PATH,
		       idx, connector->name);
	if (len >= PATH_MAX)
		goto write_exit;

	fp = filp_open(filename, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		fp = NULL;
		goto write_exit;
	}

	kernel_write(fp, op, strlen(op), &pos);
	filp_close(fp, NULL);

write_exit:
	__putname(filename);
}
