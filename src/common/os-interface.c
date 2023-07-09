/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

/*
 * This is common os interfaces.
 */

#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/sysfs.h>
#include <asm/ptrace.h>
#include <drm/drm_modes.h>
#include <drm/drm_print.h>
#include <linux/suspend.h>
#include <linux/aer.h>
#include <linux/delay.h>
#include <linux/dma-buf.h>
#include <linux/fb.h>
#include <linux/firmware.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#if defined(OS_LINUX_MAPLE_TREE_H_EXIST)
#include <linux/maple_tree.h>
#endif
#include <linux/msi.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/mempool.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/log2.h>
#include <linux/poll.h>
#include <linux/kref.h>
#include <linux/sched/task.h>
#include <linux/pid.h>
#include <linux/dcache.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/iommu.h>
#include <linux/random.h>
#include <linux/device.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <video/display_timing.h>

#include "mtgpu_device.h"
#include "os-interface.h"
#if defined(SUPPORT_ION)
#include "ion/ion.h"
#include "ion_lma_heap.h"
#endif
#include "ion/ion_uapi.h"

struct mt_work_struct {
	struct work_struct work;
	void *data;
};

struct mt_delayed_work_struct {
	struct delayed_work dwork;
	void *data;
};

struct mt_miscdevice {
	struct miscdevice miscdev;
	void *data;
};

const u64 os_value[] = {
#define X(VALUE) VALUE,
	DECLEAR_OS_VALUE
#undef X
};

struct shash_desc *os_create_shash_desc(struct crypto_shash *tfm)
{
	struct shash_desc *desc;

	desc = kmalloc(sizeof(*desc) + crypto_shash_descsize(tfm),
		       GFP_KERNEL);
	desc->tfm = tfm;

	return desc;
}

void os_destroy_shash_desc(struct shash_desc *desc)
{
	os_kfree(desc);
}

struct crypto_shash *os_crypto_alloc_shash(const char *alg_name, u32 type, u32 mask)
{
	return crypto_alloc_shash(alg_name, type, mask);
}

int os_crypto_shash_digest(struct shash_desc *desc, const u8 *data, unsigned int len, u8 *out)
{
	return crypto_shash_digest(desc, data, len, out);
}

void os_crypto_free_shash(struct crypto_shash *tfm)
{
	crypto_free_shash(tfm);
}

#if defined(SUPPORT_ION)
size_t os_ion_query_heaps_kernel(struct ion_device *idev, struct ion_heap_data *hdata, size_t size)
{
	return ion_query_heaps_kernel(idev, hdata, size);
}

void os_ion_dev_lma_heap_destroy(struct ion_device *dev, const char *name)
{
	struct ion_heap *this, *next;

	plist_for_each_entry_safe(this, next, &dev->heaps, node) {
		if (!strncmp(this->name, name, strlen(this->name) - 1))
			kfree(this->name);
		ion_lma_heap_destroy(this);
	}
}

struct ion_platform_heap *os_ion_platform_heap_create(void)
{
	return kzalloc(sizeof(struct ion_platform_heap), GFP_KERNEL);
}

void os_ion_platform_heap_init(struct ion_platform_heap *heap, u32 type, unsigned int id,
			       const char *name, phys_addr_t vram_base)
{
	heap->type = type;
	heap->id = id;
	heap->name = name;
	heap->vram_base = vram_base;
}

int os_ion_device_add_heap(struct ion_device *idev, struct ion_heap *heap)
{
	return ion_device_add_heap(idev, heap);
}

struct ion_device *os_ion_device_create(void)
{
	return ion_device_create();
}

int os_ion_device_destroy(struct ion_device *idev)
{
	return ion_device_destroy(idev);
}

const char *os_ion_get_dev_name(struct ion_device *idev)
{
	return idev->dev.name;
}

struct ion_heap_data *os_alloc_ion_heap_data_array(u32 count)
{
	return kzalloc(sizeof(struct ion_heap_data) * count, GFP_KERNEL);
}

char *os_get_ion_heap_name(struct ion_heap_data *data, int i)
{
	return data[i].name;
}

u32 os_get_ion_heap_id(struct ion_heap_data *data, int i)
{
	return data[i].heap_id;
}

#endif /*SUPPORT_ION*/

int os_sysfs_create_file(struct kobject *kobj, const struct attribute *attr)
{
	return sysfs_create_file(kobj, attr);
}

void os_sysfs_remove_file(struct kobject *kobj, const struct attribute *attr)
{
	sysfs_remove_file(kobj, attr);
}

bool os_is_power_of_2(unsigned long n)
{
	return is_power_of_2(n);
}

void *os_dev_get_drvdata(const struct device *dev)
{
	return dev_get_drvdata(dev);
}

void os_dev_set_drvdata(struct device *dev, void *data)
{
	dev_set_drvdata(dev, data);
}

struct device *os_get_device(struct device *dev)
{
	return get_device(dev);
}

void os_put_device(struct device *dev)
{
	return put_device(dev);
}

int os_device_attach(struct device *dev)
{
	return device_attach(dev);
}

struct kobject *os_get_device_kobj(struct device *dev)
{
	return &dev->kobj;
}

int os_device_attr_create(struct device_attribute **dev_attr, const char *name, umode_t mode,
			  ssize_t (*show)(struct device *dev,
					  struct device_attribute *attr,
					  char *buf),
			  ssize_t (*store)(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf,
					   size_t count))
{
	*dev_attr = kzalloc(sizeof(struct device_attribute), GFP_KERNEL);
	if (!(*dev_attr))
		return -ENOMEM;

	(*dev_attr)->attr.name = name;
	(*dev_attr)->attr.mode = mode;
	(*dev_attr)->show = show;
	(*dev_attr)->store = store;

	return 0;
}

void os_device_attr_destroy(struct device_attribute *dev_attr)
{
	kfree(dev_attr);
}

struct attribute *os_get_device_attr_attr(struct device_attribute *dev_attr)
{
	return &dev_attr->attr;
}

bool os_dev_is_pci(struct device *dev)
{
	return dev_is_pci(dev);
}

struct pci_dev *os_to_pci_dev(struct device *dev)
{
	return container_of(dev, struct pci_dev, dev);
}

int os_find_first_bit(const unsigned long *p, unsigned int size)
{
	return find_first_bit(p, size);
}

int os_find_next_bit(const unsigned long *p, int size, int offset)
{
	return find_next_bit(p, size, offset);
}

void *os_kmalloc(size_t size)
{
	return kmalloc(size, GFP_KERNEL);
}

void *os_kmalloc_atomic(size_t size)
{
	return kmalloc(size, GFP_ATOMIC);
}

void *os_kzalloc(size_t size)
{
	return kzalloc(size, GFP_KERNEL);
}

void os_kfree(const void *ptr)
{
	return kfree(ptr);
}

void *os_kcalloc(size_t n, size_t size)
{
	return kcalloc(n, size, GFP_KERNEL);
}

void *os_devm_kzalloc(struct device *dev, size_t size)
{
	return devm_kzalloc(dev, size, GFP_KERNEL);
}

void *os_vmalloc(unsigned long size)
{
	return vmalloc(size);
}

void *os_vzalloc(unsigned long size)
{
	return vzalloc(size);
}

void *os_kvzalloc(size_t size)
{
	return kvzalloc(size, GFP_KERNEL);
}

void os_kvfree(const void *addr)
{
	kvfree(addr);
}

void os_vfree(const void *addr)
{
	vfree(addr);
}

struct page *os_vmalloc_to_page(const void *vmalloc_addr)
{
	return vmalloc_to_page(vmalloc_addr);
}

struct page *os_phys_to_page(phys_addr_t pa)
{
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	return pfn_to_page(pa >> PAGE_SHIFT);
#else
	return phys_to_page(pa);
#endif
}

struct apertures_struct *os_alloc_apertures(unsigned int max_num)
{
	return alloc_apertures(max_num);
}

int os_sg_table_create(struct sg_table **sgt)
{
	*sgt = kzalloc(sizeof(struct sg_table), GFP_KERNEL);

	if (!(*sgt))
		return -ENOMEM;
	else
		return 0;
}

void os_sg_table_destroy(struct sg_table *sgt)
{
	kfree(sgt);
}

/* get sg_table member */
IMPLEMENT_GET_OS_MEMBER_FUNC(sg_table, sgl);
IMPLEMENT_GET_OS_MEMBER_FUNC(sg_table, nents);
IMPLEMENT_GET_OS_MEMBER_FUNC(sg_table, orig_nents);

struct scatterlist *os_sg_next(struct scatterlist *sg)
{
	return sg_next(sg);
}

dma_addr_t os_sg_dma_address(struct scatterlist *sg)
{
	return sg_dma_address(sg);
}

unsigned int os_sg_dma_len(struct scatterlist *sg)
{
	return sg_dma_len(sg);
}

int os_sg_alloc_table_from_pages(struct sg_table *sgt, struct page **pages,
				 unsigned int n_pages, unsigned int offset,
				 unsigned long size)
{
	return sg_alloc_table_from_pages(sgt, pages, n_pages, offset, size, GFP_KERNEL);
}

void os_sg_free_table(struct sg_table *sgt)
{
	sg_free_table(sgt);
}

int os_dma_map_sg(struct device *dev, struct scatterlist *sg,
		  int nents, u64 dir)
{
	return dma_map_sg_attrs(dev, sg, nents, (enum dma_data_direction)dir, 0);
}

void os_dma_unmap_sg(struct device *dev, struct scatterlist *sg,
		     int nents, u64 dir)
{
	dma_unmap_sg_attrs(dev, sg, nents, (enum dma_data_direction)dir, 0);
}

dma_addr_t os_dma_map_page(struct device *dev, struct page *page, size_t offset,
			   size_t size, u64 dir)
{
	return dma_map_page(dev, page, offset, size, (enum dma_data_direction)dir);
}

void os_dma_unmap_page(struct device *dev, dma_addr_t dma_addr, size_t size, u64 dir)
{
	dma_unmap_page(dev, dma_addr, size, (enum dma_data_direction)dir);
}

dma_addr_t os_dma_map_resource(struct device *dev, phys_addr_t phys_addr,
			       size_t size, u64 dir)
{
	return dma_map_resource(dev, phys_addr, size, (enum dma_data_direction)dir, 0);
}

void os_dma_unmap_resource(struct device *dev, dma_addr_t addr, size_t size, u64 dir)
{
	dma_unmap_resource(dev, addr, size, (enum dma_data_direction)dir, 0);
}

int os_dma_mapping_error(struct device *dev, dma_addr_t dma_addr)
{
	return dma_mapping_error(dev, dma_addr);
}

void os_dma_sync_sg_for_device(struct device *dev, struct scatterlist *sg,
			       int nelems, u64 dir)
{
	dma_sync_sg_for_device(dev, sg, nelems, (enum dma_data_direction)dir);
}

void os_dma_sync_sg_for_cpu(struct device *dev, struct scatterlist *sg,
			    int nelems, u64 dir)
{
	dma_sync_sg_for_cpu(dev, sg, nelems, (enum dma_data_direction)dir);
}

struct pid *os_find_vpid(int nr)
{
	return find_vpid(nr);
}

struct task_struct *os_pid_task(struct pid *pid, int type)
{
	return pid_task(pid, (enum pid_type)type);
}

struct mm_struct *os_get_task_mm(struct task_struct *p)
{
	return p->mm;
}

void os_task_lock(struct task_struct *p)
{
	task_lock(p);
}

void os_task_unlock(struct task_struct *p)
{
	task_unlock(p);
}

char *os_d_path(const struct path *p, char *param, int size)
{
	return d_path(p, param, size);
}

struct path *os_get_exec_vma_file_path(struct mm_struct *mm)
{
	struct vm_area_struct *vma;

#if defined(OS_LINUX_MAPLE_TREE_H_EXIST)
	MA_STATE(mas, &mm->mm_mt, 0, 0);

	mas_for_each(&mas, vma, ULONG_MAX) {
		if ((vma->vm_flags & VM_EXEC) && !vma->vm_file)
			return &vma->vm_file->f_path;
	}
#else
	vma = mm->mmap;

	while (vma) {
		if ((vma->vm_flags & VM_EXEC) && !vma->vm_file)
			return &vma->vm_file->f_path;

		vma = vma->vm_next;
	}
#endif
	return NULL;
}

IMPLEMENT_GET_OS_MEMBER_FUNC(vm_area_struct, vm_start);
IMPLEMENT_GET_OS_MEMBER_FUNC(vm_area_struct, vm_end);
IMPLEMENT_GET_OS_MEMBER_FUNC(vm_area_struct, vm_flags);
IMPLEMENT_GET_OS_MEMBER_FUNC(vm_area_struct, vm_file);

void os_set_vm_area_struct_vm_flags(struct vm_area_struct *vma, unsigned long flag)
{
	vma->vm_flags = flag;
}

void *os_memcpy(void *dst, const void *src, size_t size)
{
	return memcpy(dst, src, size);
}

void os_memcpy_fromio(void *dst, const void __iomem *src, size_t size)
{
	return memcpy_fromio(dst, src, size);
}

void os_memcpy_toio(void __iomem *dst, const void *src, size_t size)
{
	return memcpy_toio(dst, src, size);
}

void os_memset_io(void __iomem *addr, int value, size_t size)
{
	memset_io(addr, value, size);
}

void *os_memset(void *s, int c, size_t count)
{
	return memset(s, c, count);
}

int os_sema_create(struct semaphore **sem, int val)
{
	*sem = kzalloc(sizeof(*(*sem)), GFP_KERNEL);
	if (!(*sem))
		return -ENOMEM;

	sema_init(*sem, val);

	return 0;
}

void os_up(struct semaphore *sem)
{
	up(sem);
}

int os_down_timeout(struct semaphore *sem, long timeout)
{
	return down_timeout(sem, timeout);
}

void os_sema_destroy(struct semaphore *sem)
{
	kfree(sem);
}

int os_mutex_create(struct mutex **lock)
{
	*lock = kzalloc(sizeof(struct mutex), GFP_KERNEL);
	if (!(*lock))
		return -ENOMEM;

	mutex_init(*lock);

	return 0;
}

void os_mutex_lock(struct mutex *lock)
{
	return mutex_lock(lock);
}

void os_mutex_unlock(struct mutex *lock)
{
	return mutex_unlock(lock);
}

void os_mutex_destroy(struct mutex *lock)
{
	mutex_destroy(lock);
	kfree(lock);
}

int os_spin_lock_create(spinlock_t **lock)
{
	*lock = kzalloc(sizeof(spinlock_t), GFP_KERNEL);
	if (!(*lock))
		return -ENOMEM;

	spin_lock_init(*lock);

	return 0;
}

void os_spin_lock(spinlock_t *lock)
{
	return spin_lock(lock);
}

void os_spin_unlock(spinlock_t *lock)
{
	return spin_unlock(lock);
}

void os_spin_lock_irqsave(spinlock_t *lock, unsigned long *flags)
{
	spin_lock_irqsave(lock, *flags);
}

void os_spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	return spin_unlock_irqrestore(lock, flags);
}

void os_spin_lock_bh(spinlock_t *lock)
{
	spin_lock_bh(lock);
}

void os_spin_unlock_bh(spinlock_t *lock)
{
	spin_unlock_bh(lock);
}

void os_spin_lock_destroy(spinlock_t *lock)
{
	kfree(lock);
}

unsigned long os_get_jiffies(void)
{
	return jiffies;
}

unsigned long os_msecs_to_jiffies(const unsigned int m)
{
	return msecs_to_jiffies(m);
}

unsigned long os_nsecs_to_jiffies(u64 n)
{
	return nsecs_to_jiffies(n);
}

unsigned long os_get_tick_us(void)
{
	return 1000000 / HZ;
}

void os_get_random_bytes(void *buf, int nbytes)
{
	get_random_bytes(buf, nbytes);
}

struct tasklet_struct *os_create_tasklet(void (*fun)(unsigned long), unsigned long data)
{
	struct tasklet_struct *tasklet = kmalloc(sizeof(*tasklet), GFP_KERNEL);

	if (tasklet)
		tasklet_init(tasklet, fun, data);

	return tasklet;
}

void os_destroy_tasklet(struct tasklet_struct *tasklet)
{
	kfree(tasklet);
}

void os_tasklet_schedule(struct tasklet_struct *tasklet)
{
	tasklet_schedule(tasklet);
}

static struct mt_work_struct *os_get_mt_work(struct work_struct *work)
{
	return container_of(work, struct mt_work_struct, work);
}

void *os_get_work_drvdata(struct work_struct *work)
{
	struct mt_work_struct *mt_work = os_get_mt_work(work);

	return mt_work->data;
}

void os_set_work_drvdata(struct work_struct *work, void *data)
{
	struct mt_work_struct *mt_work = os_get_mt_work(work);

	mt_work->data = data;
}

void *os_create_work(void)
{
	return kzalloc(sizeof(struct mt_work_struct), GFP_KERNEL);
}

void *os_create_work_atomic(void)
{
	return kzalloc(sizeof(struct mt_work_struct), GFP_ATOMIC);
}

void os_destroy_work(struct work_struct *work)
{
	kfree(work);
}

static struct mt_delayed_work_struct *os_get_mt_dwork(struct delayed_work *dwork)
{
	return container_of(dwork, struct mt_delayed_work_struct, dwork);
}

void *os_get_dwork_drvdata(struct work_struct *work)
{
	struct mt_delayed_work_struct *mt_dwork = os_get_mt_dwork((struct delayed_work *)work);

	return mt_dwork->data;
}

void os_set_dwork_drvdata(struct delayed_work *dwork, void *data)
{
	struct mt_delayed_work_struct *mt_dwork = os_get_mt_dwork(dwork);

	mt_dwork->data = data;
}

void *os_create_dwork(void)
{
	return kzalloc(sizeof(struct mt_delayed_work_struct), GFP_KERNEL);
}

void os_destroy_dwork(struct delayed_work *dwork)
{
	kfree(dwork);
}

void os_init_dwork(struct delayed_work *work, work_func_t func)
{
	INIT_DEFERRABLE_WORK(work, func);
}

bool os_queue_delayed_work(struct workqueue_struct *wq,
			   struct delayed_work *dwork,
			   unsigned long delay)
{
	return queue_delayed_work(wq, dwork, delay);
}

bool os_cancel_delayed_work_sync(struct delayed_work *dwork)
{
	return cancel_delayed_work_sync(dwork);
}

void os_poll_wait(struct file *filp, wait_queue_head_t *wait_address, struct poll_table_struct *p)
{
	poll_wait(filp, wait_address, p);
}

void os_seq_puts(struct seq_file *m, const char *s)
{
	seq_puts(m, s);
}

void os_seq_putc(struct seq_file *m, char c)
{
	seq_putc(m, c);
}

void os_seq_put_decimal_ll(struct seq_file *m, const char *delimiter, long long num)
{
	seq_put_decimal_ll(m, delimiter, num);
}

ssize_t os_seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	return seq_read(file, buf, size, ppos);
}

loff_t os_seq_lseek(struct file *file, loff_t offset, int whence)
{
	return seq_lseek(file, offset, whence);
}

int os_single_open(struct file *file, int (*show)(struct seq_file *, void *), void *data)
{
	return single_open(file, show, data);
}

int os_single_release(struct inode *inode, struct file *file)
{
	return single_release(inode, file);
}

void *os_get_seq_file_private(struct seq_file *seq)
{
	return seq->private;
}

struct proc_dir_entry *os_proc_create_data(const char *name, umode_t mode,
					   struct proc_dir_entry *parent,
					   const void *proc_fops, void *data)
{
#if defined(OS_STRUCT_PROC_OPS_EXIST)
	struct proc_ops *new_proc_ops = (struct proc_ops *)proc_fops;
#else
	struct file_operations *new_proc_ops = (struct file_operations *)proc_fops;
#endif
	return proc_create_data(name, mode, parent, new_proc_ops, data);
}

struct proc_dir_entry *os_proc_create_single_data(const char *name, umode_t mode,
						  struct proc_dir_entry *parent,
						  int (*show)(struct seq_file *, void *),
						  void *data)
{
	return proc_create_single_data(name, mode, parent, show, data);
}

struct proc_dir_entry *os_proc_mkdir(const char *name, struct proc_dir_entry *parent)
{
	return proc_mkdir(name, parent);
}

void os_proc_remove(struct proc_dir_entry *de)
{
	proc_remove(de);
}

void os_remove_proc_entry(const char *name, struct proc_dir_entry *parent)
{
	remove_proc_entry(name, parent);
}

int os_atomic_read_this_module_refcnt(void)
{
	return atomic_read(&THIS_MODULE->refcnt);
}

void *os_pde_data(const struct inode *inode)
{
#if defined(OS_FUNC_PDE_DATA_EXIST)
	return PDE_DATA(inode);
#else
	return pde_data(inode);
#endif
}

void *os_miscdevice_create()
{
	return kzalloc(sizeof(struct mt_miscdevice), GFP_KERNEL);
}

int os_miscdevice_init(struct miscdevice *misc, struct device *parent,
		       const char *name, int mode, const struct mt_file_operations *ops)
{
	struct file_operations *fops;

	fops = kzalloc(sizeof(*fops), GFP_KERNEL);
	if (!fops)
		return -ENOMEM;

	misc->name = kstrdup(name, GFP_KERNEL);
	if (!misc->name) {
		kfree(fops);
		return -ENOMEM;
	}

	misc->parent = parent;
	misc->mode = mode;

	fops->owner		= THIS_MODULE;
	fops->open		= ops->open;
	fops->read		= ops->read;
	fops->write		= ops->write;
	fops->unlocked_ioctl	= ops->unlocked_ioctl;
	fops->mmap		= ops->mmap;
	fops->poll		= ops->poll;
	fops->release		= ops->release;

	/* each miscdevice has it own fops */
	misc->fops = fops;
	return 0;
}

void os_miscdevice_deinit(struct miscdevice *misc)
{
	/* each miscdevice has it own fops */
	kfree(misc->name);
	kfree(misc->fops);
}

void os_miscdevice_destroy(struct miscdevice *misc)
{
	kfree(misc);
}

int os_misc_register(struct miscdevice *misc)
{
	return misc_register(misc);
}

void os_misc_deregister(struct miscdevice *misc)
{
	misc_deregister(misc);
}

void os_set_miscdevice_minor(struct miscdevice *misc, int minor)
{
	misc->minor = minor;
}

void *os_get_miscdevice_drvdata(struct miscdevice *misc)
{
	struct mt_miscdevice *mt_misc = (struct mt_miscdevice *)misc;

	return mt_misc->data;
}

void os_set_miscdevice_drvdata(struct miscdevice *misc, void *data)
{
	struct mt_miscdevice *mt_misc = (struct mt_miscdevice *)misc;

	mt_misc->data = data;
}

const char *os_get_miscdevice_name(struct miscdevice *misc)
{
	return misc->name;
}

struct device *os_get_miscdevice_parent(struct miscdevice *misc)
{
	return misc->parent;
}

int os_get_miscdevice_minor(struct miscdevice *misc)
{
	return misc->minor;
}

void os_set_file_private_data(struct file *file, void *private_data)
{
	file->private_data = private_data;
}

void *os_get_file_private_data(struct file *file)
{
	return file->private_data;
}

void *os_get_file_node_private_data(struct file *file)
{
    return file_inode(file)->i_private;
}

unsigned int os_get_file_flags(struct file *file)
{
	return file->f_flags;
}

void os_kref_init(mt_kref *kref)
{
	kref_init((struct kref *)kref);
}

int os_kref_put(mt_kref *kref, void (*release)(mt_kref *kref))
{
	return kref_put((struct kref *)kref, (void *)release);
}

void os_kref_get(mt_kref *kref)
{
	kref_get((struct kref *)kref);
}

int os_ida_create(struct ida **ida)
{
	*ida = kmalloc(sizeof(**ida), GFP_KERNEL);
	if (!(*ida))
		return -ENOMEM;

	ida_init(*ida);

	return 0;
}

void os_ida_destroy(struct ida *ida)
{
	ida_destroy(ida);
	kfree(ida);
}

int os_ida_alloc(struct ida *ida)
{
	return ida_alloc(ida, GFP_KERNEL);
}

int os_ida_alloc_range(struct ida *ida, unsigned int min, unsigned int max)
{
	return ida_alloc_range(ida, min, max, GFP_KERNEL);
}

void os_ida_free(struct ida *ida, unsigned long id)
{
	ida_free(ida, id);
}

bool os_ida_is_empty(const struct ida *ida)
{
	return ida_is_empty(ida);
}

bool os_queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	return queue_work(wq, work);
}

void os_destroy_workqueue(struct workqueue_struct *wq)
{
	destroy_workqueue(wq);
}

struct workqueue_struct *os_create_freezable_workqueue(char *name)
{
	return create_freezable_workqueue(name);
}

struct workqueue_struct *os_alloc_workqueue(const char *fmt, unsigned int flags, int max_active)
{
	return alloc_workqueue(fmt, flags, max_active);
}

struct workqueue_struct *os_alloc_ordered_workqueue(const char *fmt)
{
	return create_singlethread_workqueue(fmt);
}

void os_flush_workqueue(struct workqueue_struct *wq)
{
	flush_workqueue(wq);
}

void os_wake_up(struct wait_queue_head *wq_head)
{
	wake_up(wq_head);
}

void os_wake_up_interruptible(struct wait_queue_head *wq_head)
{
	wake_up_interruptible(wq_head);
}

void os_wake_up_all(struct wait_queue_head *wq_head)
{
	wake_up_all(wq_head);
}

int os_create_waitqueue_head(struct wait_queue_head **wq_head)
{
	*wq_head = kzalloc(sizeof(struct wait_queue_head), GFP_KERNEL);
	if (!(*wq_head))
		return -ENOMEM;

	init_waitqueue_head(*wq_head);

	return 0;
}

void os_destroy_waitqueue_head(struct wait_queue_head *wq_head)
{
	kfree(wq_head);
}

void os_might_sleep(void)
{
	might_sleep();
}

long os_schedule_timeout(long timeout)
{
	return schedule_timeout(timeout);
}

void os_init_wait_entry(struct wait_queue_entry *wq_entry, int flags)
{
	init_wait_entry(wq_entry, flags);
}

void os_finish_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
	finish_wait(wq_head, wq_entry);
}

long os_prepare_to_wait_event_uninterruptible(struct wait_queue_head *wq_head,
					      struct wait_queue_entry *wq_entry)
{
	return prepare_to_wait_event(wq_head, wq_entry, TASK_UNINTERRUPTIBLE);
}

struct wait_queue_entry *os_create_wait_queue_entry(void)
{
	return kmalloc(sizeof(struct wait_queue_entry), GFP_KERNEL);
}

void os_init_work(struct work_struct *work, work_func_t func)
{
	INIT_WORK(work, func);
}

bool os_schedule_work(struct work_struct *work)
{
	return schedule_work(work);
}

bool os_cancel_work_sync(struct work_struct *work)
{
	return cancel_work_sync(work);
}

void os_wmb(void)
{
	wmb(); /* memory barrier */
}

void os_mb(void)
{
	mb(); /* memory barrier */
}

int os_arch_io_reserve_memtype_wc(resource_size_t base, resource_size_t size)
{
	return arch_io_reserve_memtype_wc(base, size);
}

void os_arch_io_free_memtype_wc(resource_size_t base, resource_size_t size)
{
	arch_io_free_memtype_wc(base, size);
}

int os_arch_phys_wc_add(unsigned long base, unsigned long size)
{
	return arch_phys_wc_add(base, size);
}

void os_arch_phys_wc_del(int handle)
{
	arch_phys_wc_del(handle);
}

void __iomem *os_ioremap(phys_addr_t phys_addr, size_t size)
{
	return ioremap(phys_addr, size);
}

void __iomem *os_ioremap_cache(resource_size_t offset, unsigned long size)
{
	return ioremap_cache(offset, size);
}

void os_iounmap(void __iomem *io_addr)
{
	iounmap(io_addr);
}

int os_get_user_pages_fast(unsigned long start, int nr_pages,
			   unsigned int gup_flags, struct page **pages)
{
	return get_user_pages_fast(start, nr_pages, gup_flags, pages);
}

void os_put_page(struct page *page)
{
	put_page(page);
}

unsigned int os_ioread32(void __iomem *addr)
{
	return ioread32(addr);
}

u32 os_readl(const void __iomem *addr)
{
	return readl(addr);
}

void os_iowrite32(u32 b, void __iomem *addr)
{
	iowrite32(b, addr);
}

void os_writel(u32 value, void __iomem *addr)
{
	writel(value, addr);
}

unsigned int os_get_pci_dev_virfn(struct pci_dev *pdev)
{
	return pdev->is_virtfn;
}

struct device *os_get_pci_device_base(struct pci_dev *pdev)
{
	return &pdev->dev;
}

unsigned short os_get_pci_device_vendor(struct pci_dev *pdev)
{
	return pdev->vendor;
}

unsigned short os_get_pci_subsystem_vendor(struct pci_dev *pdev)
{
	return pdev->subsystem_vendor;
}

unsigned short os_get_pci_device_id(struct pci_dev *pdev)
{
	return pdev->device;
}

unsigned short os_get_pci_subsystem_device_id(struct pci_dev *pdev)
{
	return pdev->subsystem_device;
}

struct pci_bus *os_get_pci_bus(struct pci_dev *pdev)
{
	return pdev->bus;
}

struct resource *os_get_pci_resource(struct pci_dev *pdev)
{
	return pdev->resource;
}

pci_power_t os_get_pci_current_state(struct pci_dev *pdev)
{
	return pdev->current_state;
}

kernel_ulong_t os_get_pci_device_data(const struct pci_device_id *id)
{
	return id->driver_data;
}

int os_pci_domain_nr(struct pci_dev *pdev)
{
	return pci_domain_nr(pdev->bus);
}

int os_request_pci_io_addr(struct pci_dev *pdev, u32 index,
			   resource_size_t offset, resource_size_t length)
{
	return request_pci_io_addr(pdev, index, offset, length);
}

unsigned int os_pci_slot(unsigned int devfn)
{
	return PCI_SLOT(devfn);
}

unsigned int os_pci_func(unsigned int devfn)
{
	return PCI_FUNC(devfn);
}

unsigned int os_get_pci_devfn(struct pci_dev *pdev)
{
	return pdev->devfn;
}

unsigned char os_get_pci_bus_number(struct pci_dev *pdev)
{
	return pdev->bus->number;
}

unsigned int os_get_pci_irq(struct pci_dev *pdev)
{
	return pdev->irq;
}

resource_size_t os_pci_resource_start(struct pci_dev *pdev, int bar)
{
	return pci_resource_start(pdev, bar);
}

resource_size_t os_pci_resource_end(struct pci_dev *pdev, int bar)
{
	return pci_resource_end(pdev, bar);
}

unsigned long os_pci_resource_flags(struct pci_dev *pdev, int bar)
{
	return pci_resource_flags(pdev, bar);
}

resource_size_t os_pci_resource_len(struct pci_dev *pdev, int bar)
{
	return pci_resource_len(pdev, bar);
}

int os_pci_irq_vector(struct pci_dev *dev, unsigned int nr)
{
	return pci_irq_vector(dev, nr);
}

int os_pci_enable_device(struct pci_dev *dev)
{
	return pci_enable_device(dev);
}

void os_pci_disable_device(struct pci_dev *dev)
{
	return pci_disable_device(dev);
}

void os_pci_set_drvdata(struct pci_dev *pdev, void *drvdata)
{
	pci_set_drvdata(pdev, drvdata);
}

void *os_pci_get_drvdata(struct pci_dev *pdev)
{
	return pci_get_drvdata(pdev);
}

void os_pci_set_master(struct pci_dev *dev)
{
	return pci_set_master(dev);
}

void os_pci_clear_master(struct pci_dev *dev)
{
	return pci_clear_master(dev);
}

int os_pci_load_and_free_saved_state(struct pci_dev *dev, struct pci_saved_state **state)
{
	return pci_load_and_free_saved_state(dev, state);
}

int os_pci_load_saved_state(struct pci_dev *dev, struct pci_saved_state *state)
{
	return pci_load_saved_state(dev, state);
}

void os_pci_restore_state(struct pci_dev *dev)
{
	return pci_restore_state(dev);
}

int os_pci_save_state(struct pci_dev *dev)
{
	return pci_save_state(dev);
}

int os_pci_set_power_state(struct pci_dev *dev, pci_power_t state)
{
	return pci_set_power_state(dev, state);
}

struct pci_saved_state *os_pci_store_saved_state(struct pci_dev *dev)
{
	return pci_store_saved_state(dev);
}

int os_pci_msix_vec_count(struct pci_dev *dev)
{
	return pci_msix_vec_count(dev);
}

int os_pci_msi_vec_count(struct pci_dev *dev)
{
	return pci_msi_vec_count(dev);
}

bool os_pci_support_msix(struct pci_dev *dev)
{
	return !!dev->msix_cap;
}

bool os_pci_support_msi(struct pci_dev *dev)
{
	return !!dev->msi_cap;
}

int os_pci_alloc_irq_vectors(struct pci_dev *dev, unsigned int min_vecs,
			     unsigned int max_vecs, unsigned int flags)
{
	return pci_alloc_irq_vectors(dev, min_vecs, max_vecs, flags);
}

int os_pci_enable_pcie_error_reporting(struct pci_dev *pdev)
{
	return pci_enable_pcie_error_reporting(pdev);
}

int os_pci_disable_pcie_error_reporting(struct pci_dev *pdev)
{
	return pci_disable_pcie_error_reporting(pdev);
}

int os_pci_dev_wait(struct pci_dev *dev, char *reset_type, int timeout)
{
	int delay = 1;
	u32 id;

	pci_read_config_dword(dev, PCI_COMMAND, &id);
	while (id == ~0) {
		if (delay > timeout) {
			pci_warn(dev, "not ready %dms after %s; giving up\n",
				 delay - 1, reset_type);
			return -ENOTTY;
		}

		if (delay > 1000)
			pci_info(dev, "not ready %dms after %s; waiting\n",
				 delay - 1, reset_type);

		msleep(delay);
		delay *= 2;
		pci_read_config_dword(dev, PCI_COMMAND, &id);
	}

	if (delay > 1000)
		pci_info(dev, "ready %dms after %s\n", delay - 1,
			 reset_type);

	return 0;
}

void os_get_cached_msi_msg(unsigned int irq, struct msi_msg *msg)
{
	get_cached_msi_msg(irq, msg);
}

struct msi_desc *os_irq_get_msi_desc(unsigned int irq)
{
	return irq_get_msi_desc(irq);
}

struct msi_msg *os_msi_msg_alloc(void)
{
	return kzalloc(sizeof(struct msi_msg), GFP_KERNEL);
}

int os_get_msi_message(struct msi_msg *msiptr, u32 *addr_lo, u32 *addr_hi, u32 *data)
{
	if (!msiptr)
		return -EPERM;

	*addr_lo = msiptr->address_lo;
	*addr_hi = msiptr->address_hi;
	*data = msiptr->data;

	return 0;
}

int os_get_msi_message_from_desc(struct msi_desc *desc, u32 *addr_lo, u32 *addr_hi, u32 *data)
{
	if (!desc)
		return -EPERM;

	*addr_lo = desc->msg.address_lo;
	*addr_hi = desc->msg.address_hi;
	*data = desc->msg.data;

	return 0;
}

int os_request_irq(unsigned int irq, void *handler, unsigned long flags,
		   const char *name, void *dev)
{
	return request_irq(irq, (irq_handler_t)handler, flags, name, dev);
}

const void *os_free_irq(unsigned int irq, void *dev_id)
{
	return free_irq(irq, dev_id);
}

void os_pci_free_irq_vectors(struct pci_dev *pdev)
{
	return pci_free_irq_vectors(pdev);
}

void os_udelay(unsigned long secs)
{
	udelay(secs);
}

void os_mdelay(unsigned long secs)
{
	mdelay(secs);
}

void os_usleep_range(unsigned long min, unsigned long max)
{
	usleep_range(min, max);
}

void os_msleep(unsigned int msecs)
{
	msleep(msecs);
}

struct dma_buf *os_dma_buf_get(int fd)
{
	return dma_buf_get(fd);
}

void os_dma_buf_put(struct dma_buf *dmabuf)
{
	dma_buf_put(dmabuf);
}

int os_dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
	return dma_set_mask_and_coherent(dev, mask);
}

void *os_dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t gfp)
{
	return dma_alloc_coherent(dev, size, dma_handle, gfp);
}

void os_dma_free_coherent(struct device *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
	dma_free_coherent(dev, size, cpu_addr, dma_handle);
}

int os_dma_mmap_attrs(struct device *dev, struct vm_area_struct *vma, void *cpu_addr,
		      dma_addr_t dma_addr, size_t size, unsigned long attrs)
{
	return dma_mmap_attrs(dev, vma, cpu_addr, dma_addr, size, attrs);
}

unsigned long os_copy_from_user(void *to, const void *from, unsigned long n)
{
	return copy_from_user(to, from, n);
}

unsigned long os_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	return copy_to_user(to, from, n);
}

int os_request_firmware(const struct firmware **fw, const char *name, struct device *device)
{
	return request_firmware(fw, name, device);
}

void os_release_firmware(const struct firmware *fw)
{
	release_firmware(fw);
}

struct task_struct *os_kthread_create(int (*threadfn)(void *data),
				      void *data, const char *namefmt, ...)
{
	struct task_struct *task;
	va_list args;
	char name[TASK_COMM_LEN];

	va_start(args, namefmt);
	vsnprintf(name, TASK_COMM_LEN, namefmt, args);
	va_end(args);

	task = kthread_create_on_node(threadfn, data, NUMA_NO_NODE, "%s", name);
	if (IS_ERR(task)) {
		os_printk("Failed to create kernel thread\n");
		return NULL;
	}

	wake_up_process(task);
	return task;
}

int os_kthread_stop(struct task_struct *k)
{
	return kthread_stop(k);
}

const u8 *os_get_firmware_data(const struct firmware *fw)
{
	return fw->data;
}

size_t os_get_firmware_size(const struct firmware *fw)
{
	return fw->size;
}

bool os_kthread_should_stop(void)
{
	return kthread_should_stop();
}

int os_mod_timer(struct timer_list *timer, unsigned long expires)
{
	return mod_timer(timer, expires);
}

int os_del_timer_sync(struct timer_list *timer)
{
	return del_timer_sync(timer);
}

void os_timer_setup(struct timer_list *timer, void (*function)(struct timer_list *),
		    unsigned int flags)
{
	timer_setup(timer, function, flags);
}

int os_create_timer(struct timer_list **timer)
{
	*timer = kzalloc(sizeof(*(*timer)), GFP_KERNEL);
	if (!(*timer))
		return -ENOMEM;

	return 0;
}

void os_destroy_timer(struct timer_list *timer)
{
	kfree(timer);
}

u64 os_kclock_ns64(void)
{
	ktime_t sTime = ktime_get();

#if defined(OS_UNION_KTIME_EXIST)
	return sTime.tv64;
#else
	return sTime;
#endif
}

void os_ktime_get_real_tm(struct mt_tm *mt_time, int offset)
{
	struct tm time;

	time64_to_tm(ktime_get_real_seconds(), offset, &time);
	mt_time->cur_tm_year = time.tm_year + 1900;
	mt_time->cur_tm_mon  = time.tm_mon + 1;
	mt_time->cur_tm_mday = time.tm_mday;
	mt_time->cur_tm_hour = time.tm_hour;
	mt_time->cur_tm_min  = time.tm_min;
	mt_time->cur_tm_sec  = time.tm_sec;
}

struct resource *os_request_region(resource_size_t start,
				   resource_size_t n,
				   const char *name)
{
	return request_region(start, n, name);
}

void os_release_region(resource_size_t start, resource_size_t n)
{
	release_region(start, n);
}

struct resource *os_request_mem_region(resource_size_t start,
				       resource_size_t n,
				       const char *name)
{
	return request_mem_region(start, n, name);
}

void os_release_mem_region(resource_size_t start, resource_size_t n)
{
	release_mem_region(start, n);
}

struct resource *os_create_resource(struct mtgpu_resource *mtgpu_res, u32 num_res)
{
	int i;
	struct resource *resource = kcalloc(num_res, sizeof(*resource), GFP_KERNEL);

	for (i = 0; i < num_res; i++) {
		resource[i].start = mtgpu_res[i].start;
		resource[i].end = mtgpu_res[i].end;
		resource[i].name = mtgpu_res[i].name;
		resource[i].flags = mtgpu_res[i].is_mem ? IORESOURCE_MEM : IORESOURCE_IRQ;
	}

	return resource;
}

void os_destroy_resource(struct resource *resource)
{
	kfree(resource);
}

int os_release_resource(struct resource *new)
{
	return release_resource(new);
}

resource_size_t os_resource_size(const struct resource *res)
{
	return resource_size(res);
}

resource_size_t os_resource_start(const struct resource *res)
{
	return res->start;
}

resource_size_t os_resource_end(const struct resource *res)
{
	return res->end;
}

int os_walk_iomem_res_desc(unsigned long desc, unsigned long flags, u64 start,
			   u64 end, void *arg, int (*func)(struct resource *, void *))
{
	return walk_iomem_res_desc(desc, flags, start, end, arg, func);
}

struct inode *os_file_inode(const struct file *f)
{
	return file_inode(f);
}

int os_pci_find_ext_capability(struct pci_dev *dev, int cap)
{
	return pci_find_ext_capability(dev, cap);
}

int os_pci_read_config_byte(const struct pci_dev *dev, int where, u8 *val)
{
	return pci_read_config_byte(dev, where, val);
}

int os_pci_read_config_word(const struct pci_dev *dev, int where, u16 *val)
{
	return pci_read_config_word(dev, where, val);
}

int os_pci_write_config_word(const struct pci_dev *dev, int where, u16 val)
{
	return pci_write_config_word(dev, where, val);
}

int os_pci_read_config_dword(const struct pci_dev *dev, int where, u32 *val)
{
	return pci_read_config_dword(dev, where, val);
}

int os_pci_write_config_dword(const struct pci_dev *dev, int where, u32 val)
{
	return pci_write_config_dword(dev, where, val);
}

void os_pci_release_resource(struct pci_dev *dev, int resno)
{
	pci_release_resource(dev,  resno);
}

int os_pci_resize_resource(struct pci_dev *dev, int resno, int size)
{
	return pci_resize_resource(dev,  resno,  size);
}

void os_pci_assign_unassigned_bus_resources(struct pci_bus *bus)
{
	pci_assign_unassigned_bus_resources(bus);
}

struct resource *os_pci_bus_resource_n(const struct pci_bus *bus, int n)
{
	return pci_bus_resource_n(bus, n);
}

/* Check if the root BUS has 64bit memory resources */
void os_check_root_pcibus(struct pci_bus *root, struct resource **res)
{
	int i;

	while (root->parent)
		root = root->parent;

	for (i = 0; (*res = pci_bus_resource_n(root, i)) || i < PCI_BRIDGE_RESOURCE_NUM; i++) {
		if (*res && (*res)->flags & (IORESOURCE_MEM | IORESOURCE_MEM_64) &&
		    (*res)->start > 0x100000000ull)
			break;
	}
}

void os_pci_release_related_resources(struct pci_dev *pdev, int DDR_BAR_NUM)
{
	struct resource *tmp;
	struct resource *res = pdev->resource + DDR_BAR_NUM;

	if (res->parent) {
		tmp = res->parent->child;
		while (tmp) {
			if (!(tmp->flags & IORESOURCE_UNSET)) {
				release_resource(tmp);
				tmp->end = resource_size(tmp) - 1;
				tmp->start = 0;
				tmp->flags |= IORESOURCE_UNSET;
			}
			tmp = tmp->sibling;
		}
	}
}

struct pci_dev *os_find_parent_pci_dev(struct device *dev)
{
	struct device *parent;

	dev = get_device(dev);
	while (dev) {
		if (dev_is_pci(dev))
			return to_pci_dev(dev);

		parent = get_device(dev->parent);
		put_device(dev);
		dev = parent;
	}

	return NULL;
}

struct pci_dev *os_pci_dev_get(struct pci_dev *dev)
{
	return pci_dev_get(dev);
}

void os_pci_dev_put(struct pci_dev *dev)
{
	pci_dev_put(dev);
}

struct pci_dev *os_pci_get_device(unsigned int vendor, unsigned int device,
				  struct pci_dev *from)
{
	return pci_get_device(vendor, device, from);
}

bool os_iommu_present(struct bus_type *bus)
{
	return iommu_present(bus);
}

struct bus_type *os_get_dev_bus_type(struct device *dev)
{
	return dev->bus;
}

static bool os_pci_is_downstream_port(const struct pci_dev *pdev)
{
	int type = pci_pcie_type(pdev);

	return type == PCI_EXP_TYPE_ROOT_PORT ||
	       type == PCI_EXP_TYPE_DOWNSTREAM ||
	       type == PCI_EXP_TYPE_PCIE_BRIDGE;
}

static bool os_pci_is_endpoint(const struct pci_dev *pdev)
{
	int pci_type = pci_pcie_type(pdev);

	return pci_type == PCI_EXP_TYPE_ENDPOINT ||
	       pci_type == PCI_EXP_TYPE_LEG_END;
}

static bool pci_is_sibling_downstream_port(struct pci_dev *pdev1, struct pci_dev *pdev2)
{
	if (os_pci_is_downstream_port(pdev1) && os_pci_is_downstream_port(pdev2)) {
		/* pdev1 and pdev2 have some domain and bus number,
		 * means that they are siblings.
		 */
		if (os_pci_domain_nr(pdev1) == os_pci_domain_nr(pdev2) &&
		    os_get_pci_bus_number(pdev1) == os_get_pci_bus_number(pdev2))
			return true;
	}

	return false;
}

bool os_pci_is_under_same_switch(struct pci_dev *pdev1,
				 struct pci_dev *pdev2)
{
	struct pci_dev *parent1, *parent2;

	if (pdev1 == pdev2)
		return false;

	if (!os_pci_is_endpoint(pdev1))
		return false;

	if (!os_pci_is_endpoint(pdev2))
		return false;

	parent1 = pci_upstream_bridge(pdev1);
	if (!parent1)
		/* parent is NULL, means that dev1 is root port */
		return false;

	parent2 = pci_upstream_bridge(pdev2);
	if (!parent2)
		/* parent is NULL, means that dev2 is root port */
		return false;

	return pci_is_sibling_downstream_port(parent1, parent2);
}

struct pci_dev *os_pci_find_upstream_switch(struct pci_dev *pdev)
{
	struct pci_dev *parent;

	for (parent = pdev->bus->self; parent; parent = parent->bus->self) {
		if (pci_is_pcie(parent)) {
			if (pci_pcie_type(parent) == PCI_EXP_TYPE_ROOT_PORT ||
			    pci_pcie_type(parent) == PCI_EXP_TYPE_DOWNSTREAM) {
				return parent;
			}
		}
	}

	return NULL;
}

struct platform_device_info *os_create_platform_device_info(struct device *dev,
							    const char *name,
							    int id,
							    const struct resource *res,
							    unsigned int num_res,
							    const void *data,
							    size_t size_data,
							    u64 dma_mask)
{
	struct platform_device_info *pdev_info =
			kzalloc(sizeof(*pdev_info), GFP_KERNEL);

	pdev_info->parent = dev;
	pdev_info->name = name;
	pdev_info->id = id;
	pdev_info->res = res;
	pdev_info->num_res = num_res;
	pdev_info->data = data;
	pdev_info->size_data = size_data;
	pdev_info->dma_mask = dma_mask;

	return pdev_info;
}

void os_destroy_platform_device_info(struct platform_device_info *pdev_info)
{
	kfree(pdev_info);
}

struct device *os_get_platform_device_base(struct platform_device *pdev)
{
	return &pdev->dev;
}

struct resource *os_platform_get_resource(struct platform_device *dev,
					  unsigned int type,
					  unsigned int num)
{
	return platform_get_resource(dev, type, num);
}

void os_platform_set_drvdata(struct platform_device *pdev, void *data)
{
	pdev->dev.driver_data = data;
}

void *os_platform_get_drvdata(struct platform_device *pdev)
{
	return platform_get_drvdata(pdev);
}

struct platform_device *
os_platform_device_register_full(const struct platform_device_info *pdevinfo)
{
	return platform_device_register_full(pdevinfo);
}

void os_platform_device_unregister(struct platform_device *pdev)
{
	platform_device_unregister(pdev);
}

struct platform_device *os_to_platform_device(struct device *dev)
{
	return to_platform_device(dev);
}

u64 os_roundup_pow_of_two(u64 size)
{
	return roundup_pow_of_two(size);
}

u32 os_order_base_2(u64 size)
{
	return roundup_pow_of_two(size);
}

int os_fls(unsigned int x)
{
	return fls(x);
}

int os_fls64(unsigned long x)
{
	return fls64(x);
}

resource_size_t os_get_system_available_ram_size(void)
{
	resource_size_t available_ram_size;
	struct sysinfo mem_info;

	si_meminfo(&mem_info);
	available_ram_size = si_mem_available() * mem_info.mem_unit;

	return available_ram_size;
}

int os_register_pm_notifier(struct notifier_block *nb)
{
	return register_pm_notifier(nb);
}

int os_unregister_pm_notifier(struct notifier_block *nb)
{
	return unregister_pm_notifier(nb);
}

int os_ilog2(u64 n)
{
	return ilog2(n);
}

mempool_t *os_mempool_create(int min_nr, mempool_alloc_t *alloc_fn,
			     mempool_free_t *free_fn, void *pool_data)
{
	return mempool_create(min_nr, alloc_fn, free_fn, pool_data);
}

void os_mempool_destroy(mempool_t *pool)
{
	mempool_destroy(pool);
}

void *os_mempool_alloc(mempool_t *pool, gfp_t gfp_mask)
{
	return mempool_alloc(pool, gfp_mask);
}

void os_mempool_free(void *element, mempool_t *pool)
{
	mempool_free(element, pool);
}

int os_snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return i;
}

int os_printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	return r;
}

void os_printk_ratelimited(const char *fmt, ...)
{
	va_list args;
	static DEFINE_RATELIMIT_STATE(_rs, DEFAULT_RATELIMIT_INTERVAL, DEFAULT_RATELIMIT_BURST);

	if (__ratelimit(&_rs)) {
		va_start(args, fmt);
		vprintk(fmt, args);
		va_end(args);
	}
}

void os_blocking_init_notifier_head(struct blocking_notifier_head *nh)
{
	BLOCKING_INIT_NOTIFIER_HEAD(nh);
}

void os_notifier_block_set_notifier_call(struct notifier_block *nb, notifier_fn_t cb)
{
	nb->notifier_call = cb;
}

int os_blocking_notifier_call_chain(struct blocking_notifier_head *nh,
				    unsigned long val, void *v)
{
	return blocking_notifier_call_chain(nh, val, v);
}

int os_blocking_notifier_chain_register(struct blocking_notifier_head *nh,
					struct notifier_block *n)
{
	return blocking_notifier_chain_register(nh, n);
}

int os_blocking_notifier_chain_unregister(struct blocking_notifier_head *nh,
					  struct notifier_block *n)
{
	return blocking_notifier_chain_unregister(nh,  n);
}

int os_blocking_notifier_head_create(struct blocking_notifier_head **nh)
{
	*nh = kzalloc(sizeof(struct blocking_notifier_head), GFP_KERNEL);
	if (!(*nh))
		return -ENOMEM;

	return 0;
}

void os_blocking_notifier_head_destroy(struct blocking_notifier_head *nh)
{
	kfree(nh);
}

int os_notifier_block_create(struct notifier_block **nb)
{
	*nb = kzalloc(sizeof(struct notifier_block), GFP_KERNEL);
	if (!(*nb))
		return -ENOMEM;

	return 0;
}

void os_notifier_block_destroy(struct notifier_block *nb)
{
	kfree(nb);
}

struct device *os_get_dev_parent_parent(struct device *dev)
{
	return dev->parent->parent;
}

void *os_get_device_driver_data(struct device *dev)
{
	return dev->driver_data;
}

char *os_get_current_comm(void)
{
	return current->comm;
}

u64 os_get_current_pid(void)
{
	return current->pid;
}

/*About dev print*/
static void __os_dev_printk(const char *level, const struct device *dev,
			    struct va_format *vaf)
{
	if (dev)
		dev_printk_emit(level[1] - '0', dev, "%s %s: %pV",
				dev_driver_string(dev), dev_name(dev), vaf);
	else
		printk("%s(NULL device *): %pV", level, vaf);
}

void *OS_ERR_PTR(long error)
{
	return ERR_PTR(error);
}

bool OS_IS_ERR(const void *ptr)
{
	return IS_ERR(ptr);
}

bool OS_IS_ERR_OR_NULL(__force const void *ptr)
{
	return IS_ERR_OR_NULL(ptr);
}

long OS_PTR_ERR(__force const void *ptr)
{
	return PTR_ERR(ptr);
}

int OS_READ_ONCE(int *val)
{
	return READ_ONCE(*val);
}

int os_sscanf(const char *str, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsscanf(str, fmt, args);
	va_end(args);

	return i;
}

char *os_strcat(char *dest, const char *src)
{
	return strcat(dest, src);
}

size_t os_strlen(const char *s)
{
	return strlen(s);
}

int os_strcmp(const char *cs, const char *ct)
{
	return strcmp(cs, ct);
}

int os_strncmp(const char *cs, const char *ct, size_t count)
{
	return strncmp(cs, ct, count);
}

char *os_strcpy(char *dest, const char *src)
{
	return strcpy(dest, src);
}

char *os_strncpy(char *dest, const char *src, size_t count)
{
	return strncpy(dest, src, count);
}

int os_sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, INT_MAX, fmt, args);
	va_end(args);

	return i;
}

void os_seq_printf(struct seq_file *m, const char *f, ...)
{
	va_list args;

	va_start(args, f);
	seq_vprintf(m, f, args);
	va_end(args);
}

#define define_os_dev_printk_level(func, kern_level)		\
void func(const struct device *dev, const char *fmt, ...)	\
{								\
	struct va_format vaf;					\
	va_list args;						\
								\
	va_start(args, fmt);					\
								\
	vaf.fmt = fmt;						\
	vaf.va = &args;						\
								\
	__os_dev_printk(kern_level, dev, &vaf);			\
								\
	va_end(args);						\
}								\

define_os_dev_printk_level(_os_dev_emerg, KERN_EMERG);
define_os_dev_printk_level(_os_dev_alert, KERN_ALERT);
define_os_dev_printk_level(_os_dev_crit, KERN_CRIT);
define_os_dev_printk_level(_os_dev_err, KERN_ERR);
define_os_dev_printk_level(_os_dev_warn, KERN_WARNING);
define_os_dev_printk_level(_os_dev_notice, KERN_NOTICE);
define_os_dev_printk_level(_os_dev_info, KERN_INFO);
define_os_dev_printk_level(_os_dev_dbg, KERN_DEBUG);

IMPLEMENT_OS_STRUCT_COMMON_FUNCS(notifier_block);
