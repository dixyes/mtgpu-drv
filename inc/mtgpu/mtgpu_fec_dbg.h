/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_FEC_DUMP_H__
#define __MTGPU_FEC_DUMP_H__

enum fec_dbg_cmd {
	FEC_REQUEST_COREDUMP_DIS	= 0x00,
	FEC_REQUEST_COREDUMP_EN		= 0x01,
	FEC_REQUEST_MAX,
	FEC_DUMP_START			= 0x80,
	FEC_DUMP_END			= 0x81,
	FEC_DUMP_DATA			= 0x82
};

struct fec_dbg_event {
	u32 sub_cmd;
	int len; //sizeof(data)
	loff_t pos;
	u8 data[];
};

struct fec_dbg_info {
	struct device *dev;
	struct dentry *d_fec_root;
	struct dentry *d_dump_enable;
	struct dentry *d_fw_ctrl;
	bool dump_enable;
	int current_fw_ctrl_state;

	struct file *d_file;
	char *d_name;
	struct file *umd_log_file;
};

int mtgpu_fec_dbg_init(struct device *dev, struct fec_dbg_info **dbginfo);
void mtgpu_fec_dbg_deinit(struct device *dev, struct fec_dbg_info *dbg_info);
void mtgpu_fec_dump_handler(struct device *dev, struct fec_dbg_info *dbg_info, void *data);
int mtgpu_fec_dbg_do_request(struct device *dev, enum fec_dbg_cmd req, void *data, int len);
int mtgpu_fec_warm_reboot(struct device *dev);
int mtgpu_fec_request_standby(struct device *dev);
int mtgpu_fec_request_idle(struct device *dev);
int mtgpu_fec_request_wakeup(struct device *dev, bool idle);
void mtgpu_fec_dbg_write_umd_log(struct device *dev, int size, u8 *data);
struct fec_dbg_info *mtgpu_fec_get_dbg_info_from_mtdev(struct mtgpu_device *mtdev);

#endif //__MTGPU_FEC_DUMP_H__
