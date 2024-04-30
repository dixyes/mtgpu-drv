/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MT_LINK_PROC_FS__
#define __MT_LINK_PROC_FS__

struct mtgpu_device;
struct mtlink_port;
struct mtlink_graph;
struct mtlink_device_list;
struct inode;
struct file;

int mtlink_proc_device_dir_create(struct mtgpu_device *mtdev);
void mtlink_proc_device_dir_remove(struct mtgpu_device *mtdev);
int mtlink_proc_link_dir_create(struct mtlink_port *link_port);
void mtlink_proc_link_dir_remove(struct mtlink_port *link_port);
int mtlink_proc_mtlink_dir_create(struct mtlink_graph *graph,
				  struct mtlink_device_list *device_list);
void mtlink_proc_mtlink_dir_remove(struct mtlink_graph *graph);

int mtlink_test_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_test_proc_write(struct file *file, const char __user *user_buf,
			       size_t nbytes, loff_t *ppos);
int mtlink_topo_switch_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_topo_switch_proc_write(struct file *file, const char __user *user_buf,
				      size_t nbytes, loff_t *ppos);
int mtlink_ctrl_err_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_ctrl_err_proc_write(struct file *filp, const char __user *ubuf,
				   size_t count, loff_t *offp);
int mtlink_debug_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_debug_proc_write(struct file *file, const char __user *user_buf,
				size_t nbytes, loff_t *ppos);
int mtlink_trigger_bdl_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_trigger_bdl_proc_write(struct file *file, const char __user *user_buf,
				      size_t nbytes, loff_t *ppos);
int mtlink_irqcounter_eanble_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_irqcounter_eanble_proc_write(struct file *file, const char __user *user_buf,
					    size_t nbytes, loff_t *ppos);
int mtlink_irqcounter_counter_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_irqcounter_counter_proc_write(struct file *file, const char __user *user_buf,
					     size_t nbytes, loff_t *ppos);
int mtlink_monitor_start_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_monitor_start_proc_write(struct file *file, const char __user *user_buf,
					size_t nbytes, loff_t *ppos);
int mtlink_monitor_counter_proc_open(struct inode *inode, struct file *file);
ssize_t mtlink_monitor_counter_proc_write(struct file *file, const char __user *user_buf,
					  size_t nbytes, loff_t *ppos);

#endif /*__MT_LINK_PROC_FS__*/
