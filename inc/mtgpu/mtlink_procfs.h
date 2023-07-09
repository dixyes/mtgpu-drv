/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MT_LINK_PROC_FS__
#define __MT_LINK_PROC_FS__

struct mtgpu_device;
struct mtlink_port;
struct mtlink_graph;
struct inode;
struct file;

int mtlink_proc_device_dir_create(struct mtgpu_device *mtdev);
void mtlink_proc_device_dir_remove(struct mtgpu_device *mtdev);
int mtlink_proc_link_dir_create(struct mtlink_port *link_port);
void mtlink_proc_link_dir_remove(struct mtlink_port *link_port);
int mtlink_proc_mtlink_dir_create(struct mtlink_graph *graph);
void mtlink_proc_mtlink_dir_remove(struct mtlink_graph *graph);

int mtlink_test_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_test_proc_write(struct file *file, const char __user *user_buf,
			       size_t nbytes, loff_t *ppos);

#endif /*__MT_LINK_PROC_FS__*/
