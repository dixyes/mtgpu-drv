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
#include <linux/msi.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
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
#include <video/display_timing.h>
#include <linux/utsname.h>

#include "mtgpu_device.h"
#include "os-interface.h"
#include "ion/ion.h"
#include "ion/ion_lma_heap.h"
#include "ion/ion_uapi.h"

struct mt_work_struct {
	struct work_struct work;
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

struct attribute *os_get_device_attr_attr(struct device_attribute *dev_attr)
{
	return &dev_attr->attr;
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

struct vm_area_struct_compat *os_get_mm_mmap(struct mm_struct *mm)
{
	struct vm_area_struct *first_vma;
	struct vm_area_struct_compat *vma_compat;
	vma_compat = kmalloc(sizeof(*vma_compat), GFP_KERNEL);
	if (!vma_compat) {
		return NULL;
	}
	printk("alloc vma_compat: %p\n", vma_compat);

	vma_iter_init(&vma_compat->iter, mm, 0);
	
	first_vma = vma_next(&vma_compat->iter);
	if (!first_vma) {
		kfree(vma_compat);
		return NULL;
	}

	vma_compat->cur = first_vma;

	return vma_compat;
}
struct vm_area_struct_compat *os_get_vm_area_struct_vm_next(struct vm_area_struct_compat *vma)
{
	struct vm_area_struct *next;
	next = vma_next(&vma->iter);
	if (!next) {
		kfree(vma);
		printk("free vma_compat: %p\n", vma);
		return NULL;
	}

	vma->cur = next;

	return vma;
}

unsigned long os_get_vm_area_struct_vm_start(struct vm_area_struct_compat *vma)
{
	return vma->cur->vm_start;
}
unsigned long os_get_vm_area_struct_vm_end(struct vm_area_struct_compat *vma)
{
	return vma->cur->vm_end;
}
unsigned long os_get_vm_area_struct_vm_flags(struct vm_area_struct_compat *vma)
{
	return vma->cur->vm_flags;
}
struct file *os_get_vm_area_struct_vm_file(struct vm_area_struct_compat *vma)
{
	return vma->cur->vm_file;
}
void os_set_vm_area_struct_vm_flags(struct vm_area_struct_compat *vma, unsigned long flag)
{
	vma->cur->vm_flags = flag;
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

void os_destroy_work(struct work_struct *work)
{
	kfree(work);
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
	misc->minor = MISC_DYNAMIC_MINOR;

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

struct path *os_get_file_path(struct file *file)
{
	return &file->f_path;
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

struct workqueue_struct *os_alloc_workqueue(const char *fmt, int max_active)
{
	return alloc_workqueue(fmt, WQ_UNBOUND | WQ_SYSFS, max_active);
}

void os_wake_up(struct wait_queue_head *wq_head)
{
	return wake_up(wq_head);
}

void os_wake_up_interruptible(struct wait_queue_head *wq_head)
{
	return wake_up_interruptible(wq_head);
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

unsigned short os_get_pci_device_id(struct pci_dev *pdev)
{
	return pdev->device;
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
	return pci_write_config_word(dev, where,  val);
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

void *os_devres_alloc(dr_release_t release, size_t size, gfp_t gfp)
{
	return devres_alloc(release, size, gfp);
}

void *os_devres_find(struct device *dev, void *release, void *match, void *match_data)
{
	return devres_find(dev, release, match, match_data);
}

void *os_devres_open_group(struct device *dev, void *id, gfp_t gfp)
{
	return devres_open_group(dev, id, gfp);
}

int os_devres_release_group(struct device *dev, void *id)
{
	return devres_release_group(dev, id);
}

void os_devres_add(struct device *dev, void *res)
{
	return devres_add(dev, res);
}

void os_devres_remove_group(struct device *dev, void *id)
{
	return devres_remove_group(dev, id);
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

char *os_strstr(const char *cs, const char *ct)
{
	return strstr(cs, ct);
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

char *os_get_utsname_version(void)
{
	return init_uts_ns.name.version;
}

