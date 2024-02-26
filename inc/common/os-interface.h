/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __OS_INTERFACE_H__
#define __OS_INTERFACE_H__

#include "linux-types.h"

#ifndef BIT
#define BIT(nr)		(1ul << (nr))
#endif

#ifndef ALIGN
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define __ALIGN(x, a)		__ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN(x, a)		__ALIGN((x), (a))
#define ALIGN_DOWN(x, a)	__ALIGN((x) - ((a) - 1), (a))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)	(sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef likely
#define likely(x)	(x)
#endif

#ifndef unlikely
#define unlikely(x)	(x)
#endif

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

#ifndef DMA_BIT_MASK
#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ull : ((1ull<<(n))-1))
#endif

#ifndef PATH_MAX
#define PATH_MAX	4096	/* # chars in a path name including nul */
#endif

#ifndef min
#define min(a, b) \
	({__typeof(a) _a = (a); __typeof(b) _b = (b); _a > _b ? _b : _a;})
#endif

#ifndef max
#define max(a, b) \
	({__typeof(a) _max1 = (a); __typeof(b) _max2 = (b); (void)(&_max1 == &_max2);\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#ifndef roundup
#define roundup(x, y) (					\
{							\
	typeof(y) __y = y;				\
	(((x) + (__y - 1)) / __y) * __y;		\
}							\
)
#endif

#define OS_VAL(index)	(os_value[OS_##index])
#define DECLEAR_OS_VALUE \
	X(PCI_IRQ_LEGACY)\
	X(PCI_IRQ_MSI)\
	X(PCI_IRQ_MSIX)\
	X(PCI_INTERRUPT_PIN)\
	X(PCI_INTERRUPT_LINE)\
	X(PCI_MSI_FLAGS)\
	X(PCI_MSI_FLAGS_64BIT)\
	X(PCI_MSI_FLAGS_QMASK)\
	X(PCI_MSI_FLAGS_ENABLE)\
	X(PCI_MSI_FLAGS_QSIZE)\
	X(PCI_MSI_ADDRESS_LO)\
	X(PCI_MSI_ADDRESS_HI)\
	X(PCI_MSI_DATA_64)\
	X(PCI_EXT_CAP_ID_REBAR)\
	X(PCI_REBAR_CTRL)\
	X(PCI_REBAR_CTRL_NBAR_MASK)\
	X(PCI_REBAR_CTRL_NBAR_SHIFT)\
	X(PCI_REBAR_CTRL_BAR_IDX)\
	X(PCI_REBAR_CAP)\
	X(PCI_REBAR_CAP_SIZES)\
	X(PCI_COMMAND)\
	X(PCI_COMMAND_IO)\
	X(PCI_COMMAND_MEMORY)\
	X(PCI_COMMAND_MASTER)\
	X(PCI_COMMAND_INTX_DISABLE)\
	X(PCI_EXT_CAP_ID_SRIOV)\
	X(PCI_SRIOV_VF_DID)\
	X(PCI_D3hot)\
	X(PCI_D0)\
	X(PCI_ANY_ID)\
	X(PCI_EXT_CAP_ID_ACS)\
	X(PCI_ACS_CTRL)\
	X(PCI_ACS_RR)\
	X(PCI_ACS_CR)\
	X(PCI_ACS_EC)\
	X(PCI_VENDOR_ID)\
	X(PCI_DEVICE_ID)\
	X(PCI_SUBSYSTEM_VENDOR_ID)\
	X(PCI_SUBSYSTEM_ID)\
	X(PCI_SUBDEVICE_ID_QEMU)\
	X(PCI_STATUS)\
	X(PCI_STATUS_INTERRUPT)\
	X(PCI_STATUS_66MHZ)\
	X(PCI_STATUS_FAST_BACK)\
	X(PCI_STATUS_CAP_LIST)\
	X(PCI_CLASS_DEVICE)\
	X(PCI_CLASS_DISPLAY_VGA)\
	X(PCI_CLASS_DISPLAY_3D)\
	X(PCI_CLASS_REVISION)\
	X(PCI_CAP_ID_MSI)\
	X(PCI_CAP_ID_EXP)\
	X(PCI_CAP_EXP_ENDPOINT_SIZEOF_V1)\
	X(PCI_CAP_LIST_ID)\
	X(PCI_CAP_LIST_NEXT)\
	X(PCI_CAPABILITY_LIST)\
	X(PCI_CACHE_LINE_SIZE)\
	X(PCI_BASE_ADDRESS_0)\
	X(PCI_BASE_ADDRESS_1)\
	X(PCI_BASE_ADDRESS_2)\
	X(PCI_BASE_ADDRESS_3)\
	X(PCI_BASE_ADDRESS_4)\
	X(PCI_BASE_ADDRESS_5)\
	X(PCI_BASE_ADDRESS_MEM_MASK)\
	X(PCI_BASE_ADDRESS_SPACE_MEMORY)\
	X(PCI_BASE_ADDRESS_MEM_TYPE_32)\
	X(PCI_BASE_ADDRESS_MEM_TYPE_64)\
	X(PCI_BASE_ADDRESS_MEM_PREFETCH)\
	X(PCI_ROM_ADDRESS)\
	X(PCI_EXP_FLAGS)\
	X(PCI_EXP_FLAGS_VERS)\
	X(PCI_EXP_FLAGS_TYPE)\
	X(PCI_EXP_DEVCAP)\
	X(PCI_EXP_DEVCAP_FLR)\
	X(PCI_EXP_DEVCTL)\
	X(PCI_EXP_DEVCTL_BCR_FLR)\
	X(PCI_EXP_TYPE_ENDPOINT)\
	X(PCI_STD_HEADER_SIZEOF)\
	X(IRQF_SHARED)\
	X(MODE_OK)\
	X(DRM_UT_CORE)\
	X(DISPLAY_FLAGS_HSYNC_LOW)\
	X(DISPLAY_FLAGS_HSYNC_HIGH)\
	X(DISPLAY_FLAGS_VSYNC_LOW)\
	X(DISPLAY_FLAGS_VSYNC_HIGH)\
	X(DISPLAY_FLAGS_PIXDATA_NEGEDGE)\
	X(PAGE_SIZE)\
	X(PAGE_SHIFT)\
	X(FOLL_WRITE)\
	X(O_NONBLOCK)\
	X(O_RDWR)\
	X(O_TRUNC)\
	X(O_CREAT)\
	X(EPOLLIN)\
	X(EPOLLHUP)\
	X(GFP_DMA)\
	X(GFP_KERNEL)\
	X(GFP_ATOMIC)\
	X(PIDTYPE_PID)\
	X(VM_EXEC)\
	X(VM_LOCKED)\
	X(VM_SHARED)\
	X(IORESOURCE_MEM)\
	X(IORESOURCE_IRQ)\
	X(IORESOURCE_IO)\
	X(IORESOURCE_UNSET)\
	X(IORES_DESC_NONE)\
	X(IORESOURCE_SYSRAM)\
	X(PLATFORM_DEVID_AUTO)\
	X(PM_HIBERNATION_PREPARE)\
	X(PM_SUSPEND_PREPARE)\
	X(PM_POST_HIBERNATION)\
	X(PM_POST_RESTORE)\
	X(PM_POST_SUSPEND)\
	X(PM_RESTORE_PREPARE)\
	X(EINVAL)\
	X(ENOMEM)\
	X(EIO)\
	X(ETIME)\
	X(EPERM)\
	X(EEXIST)\
	X(ENODEV)\
	X(ENXIO)\
	X(ETIMEDOUT)\
	X(EBUSY)\
	X(EFAULT)\
	X(ENOTSUPP)\
	X(EOPNOTSUPP)\
	X(ENOSPC)\
	X(EACCES)\
	X(EAGAIN)\
	X(ENOENT)\
	X(ENOTTY)\
	X(ERANGE)\
	X(IRQ_HANDLED)\
	X(IRQ_NONE)\
	X(IRQF_TRIGGER_NONE)\
	X(DMA_BIDIRECTIONAL)\
	X(DMA_TO_DEVICE)\
	X(DMA_FROM_DEVICE)\
	X(ION_HEAP_TYPE_CUSTOM)\
	X(WQ_UNBOUND)\
	X(WQ_SYSFS)\
	X(WQ_MEM_RECLAIM)\
	X(WQ_CPU_INTENSIVE)\
	X(WQ_HIGHPRI)\
	X(SIGINT)\
	X(SZ_4K)\
	X(SZ_1M)\
	X(NOTIFY_OK)\
	X(MAX_ORDER)\
	X(__GFP_COMP)\
	X(__GFP_ZERO)\
	X(__GFP_MOVABLE)\
	X(__GFP_RECLAIMABLE)\
	X(SLAB_ACCOUNT)\
	X(IOMMU_WRITE)\
	X(IOMMU_READ)\
	X(IOMMU_DOMAIN_UNMANAGED)

enum {
#define X(VALUE) OS_##VALUE,
	DECLEAR_OS_VALUE
#undef X
	OS_VALUE_MAX,
};

#define __os_round_mask(x, y) ((__typeof__(x))((y) - 1))
#define os_round_up(x, y) ((((x) - 1) | __os_round_mask((x), (y))) + 1)

#define os_dma_mmap_coherent(d, v, c, h, s) os_dma_mmap_attrs(d, v, c, h, s, 0)

/*
 * Used to create numbers.
 */

#ifndef _IOC_NONE

#define _IOC_NONE  0U
#define _IOC_WRITE 1U
#define _IOC_READ  2U

#define _IOC_DIRBITS  2
#define _IOC_NRBITS   8
#define _IOC_TYPEBITS 8
#define _IOC_SIZEBITS 14

#define _IOC_NRMASK  ((1 << _IOC_NRBITS) - 1)
#define _IOC_NRSHIFT 0
#define _IOC_NR(nr)  (((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)

#define _IOC_TYPESHIFT    (_IOC_NRSHIFT + _IOC_NRBITS)
#define _IOC_SIZESHIFT    (_IOC_TYPESHIFT + _IOC_TYPEBITS)
#define _IOC_DIRSHIFT     (_IOC_SIZESHIFT + _IOC_SIZEBITS)
#define _IOC_TYPECHECK(t) (sizeof(t))

#define _IOC(dir, type, nr, size) \
    (((dir) << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | ((nr) << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))

#define __IO(type, nr)        _IOC(_IOC_NONE, (type), (nr), 0)
#define _IOR(type, nr, size)  _IOC(_IOC_READ, (type), (nr), (_IOC_TYPECHECK(size)))
#define _IOW(type, nr, size)  _IOC(_IOC_WRITE, (type), (nr), (_IOC_TYPECHECK(size)))
#define _IOWR(type, nr, size) _IOC(_IOC_READ | _IOC_WRITE, (type), (nr), (_IOC_TYPECHECK(size)))
#endif /* _IOC_NONE */

struct mutex;
struct semaphore;
typedef struct spinlock spinlock_t;
struct kfifo;
struct wait_queue_head;
typedef struct wait_queue_head wait_queue_head_t;
typedef struct wait_queue_entry wait_queue_entry_t;
struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);
struct workqueue_struct;
struct platform_device;
struct platform_device_info;
struct device;
struct msi_msg;
struct msi_desc;
struct pci_bus;
struct pci_dev;
struct pci_saved_state;
struct pci_device_id;
struct resource;
struct file;
struct fd;
struct firmware;
struct timer_list;
struct mt_work;
struct sg_table;
struct dma_buf;
struct dma_buf_attachment;
struct blocking_notifier_head;
struct notifier_block;
struct kref;
struct miscdevice;
struct inode;
struct mm_struct;
struct vm_area_struct;
typedef struct poll_table_struct poll_table;
struct ida;
struct task_struct;
struct pid;
struct path;
struct mtgpu_resource;
struct attribute;
struct bin_attribute;
struct attribute_group;
struct device_attribute;
struct kobject;
struct seq_file;
struct rb_root_cached;
struct interval_tree_node;
struct bus_type;
struct delayed_work;
struct eventfd_ctx;
struct tasklet_struct;
struct mempool_s;
struct radix_tree_iter;
struct dmi_device;
struct dmi_dev_onboard;
#ifdef OS_STRUCT_XARRAY_EXIST
struct xarray;
#define radix_tree_root xarray
#else
struct radix_tree_root;
#endif
struct page;
struct iommu_group;
struct iommu_domain;
struct kmem_cache;

#if defined(SUPPORT_ION)
struct ion_heap;
struct ion_platform_heap;
struct ion_device;
struct ion_heap_data;
#endif

typedef void (*dr_release_t)(struct device *dev, void *res);
typedef int (*notifier_fn_t)(struct notifier_block *nb,
			     unsigned long action, void *data);
typedef void * (mempool_alloc_t)(gfp_t gfp_mask, void *pool_data);
typedef void (mempool_free_t)(void *element, void *pool_data);
typedef int (*wait_queue_func_t)(struct wait_queue_entry *wq_entry,
				 unsigned int mode, int flags, void *key);
typedef void (*poll_queue_proc)(struct file *, wait_queue_head_t *, struct poll_table_struct *);
typedef int mt_kref;
typedef int pci_power_t;
typedef unsigned long kernel_ulong_t;
typedef struct mempool_s mempool_t;

struct mt_file_operations {
	int (*open)(struct inode *, struct file *);
	ssize_t (*read)(struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *);
	long (*unlocked_ioctl)(struct file *, unsigned int, unsigned long);
	int (*mmap)(struct file *, struct vm_area_struct *);
	__poll_t (*poll)(struct file *, struct poll_table_struct *);
	loff_t (*llseek)(struct file *, loff_t, int);
	int (*release)(struct inode *, struct file *);
};

struct mt_tm {
	int  cur_tm_sec;
	int  cur_tm_min;
	int  cur_tm_hour;
	int  cur_tm_mday;
	int  cur_tm_mon;
	long cur_tm_year;
};

#define mt_typeof_member(T, m)	typeof(((T*)0)->m)

#define os_container_of(ptr, type, member)		\
	(type *)((uintptr_t)(ptr) - offsetof(type, member))

#define os_for_each_set_bit(bit, addr, size)		\
	for ((bit) = os_find_first_bit((addr), (size));	\
	     (bit) < (size);				\
	     (bit) = os_find_next_bit((addr), (size), (bit) + 1))

#define os_list_entry(ptr, type, member)		\
	os_container_of(ptr, type, member)

#define os_list_first_entry(ptr, type, member)		\
	os_list_entry((ptr)->next, type, member)

#define os_list_next_entry(pos, member)			\
	os_list_entry((pos)->member.next, typeof(*(pos)), member)

#define os_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = os_list_first_entry(head, typeof(*pos), member),		\
	     n = os_list_next_entry(pos, member);				\
	     &pos->member != (head); 						\
	     pos = n, n = os_list_next_entry(n, member))

#define os_plist_for_each_entry_safe(pos, n, head, m)				\
	os_list_for_each_entry_safe(pos, n, &(head)->node_list, m.node_list)

#define os_radix_tree_for_each_slot(slot, root, iter, start)		\
	for (slot = os_radix_tree_iter_init(iter, start) ;		\
	     slot || (slot = os_radix_tree_next_chunk(root, iter, 0)) ;	\
	     slot = os_radix_tree_next_slot(slot, iter, 0))

#define os_kvm_for_each_memslot(memslot, slots)			\
	for (memslot = os_list_first_kvm_memslot(slots);	\
	     os_list_kvm_memslot_check(slots, memslot);		\
	     os_kvm_memslot_inc(&memslot))

/**
 * The macros are defined for os struct.
 * Take encoder for example (type: drm_encoder):
 *      struct mt_drm_encoder {
 *              struct drm_encoder obj;
 *              void *data;
 *      };
 *      void *os_create_drm_encoder(void);
 *      void os_destroy_drm_encoder(struct drm_encoder *obj);
 *      void *os_get_drm_encoder_drvdata(struct drm_encoder *obj);
 *      void os_set_drm_encoder_drvdata(struct drm_encoder *obj, void *data);
 */
/* implementation the os struct functions */
#define IMPLEMENT_OS_STRUCT_COMMON_FUNCS(type)					\
	struct mt_##type {							\
		struct type obj;						\
		void *data;							\
	};									\
	void *os_create_##type(void)						\
	{									\
		return kzalloc(sizeof(struct mt_##type), GFP_KERNEL);		\
	}									\
	void os_destroy_##type(struct type *obj)				\
	{									\
		kfree(obj);							\
	}									\
	void *os_get_##type##_drvdata(struct type *obj)				\
	{									\
		struct mt_##type *mt_obj = (struct mt_##type *)obj;		\
		return mt_obj->data;						\
	}									\
	void os_set_##type##_drvdata(struct type *obj, void *data)		\
	{									\
		struct mt_##type *mt_obj = (struct mt_##type *)obj;		\
		mt_obj->data = data;						\
	}

/* declarations the os struct functions */
#define DECLARE_OS_STRUCT_COMMON_FUNCS(type)					\
	void *os_create_##type(void);						\
	void os_destroy_##type(struct type *obj);				\
	void *os_get_##type##_drvdata(struct type *obj);			\
	void os_set_##type##_drvdata(struct type *obj, void *data)

#define os_for_each_sg(sglist, sg, nr, __i)	\
	for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = os_sg_next(sg))

/* get member of the structure */
#define IMPLEMENT_GET_OS_MEMBER_FUNC(type, member)					\
mt_typeof_member(struct type, member) os_get_##type##_##member(struct type *ptr)	\
{											\
	return ptr->member;								\
}

/* set member of the structure */
#define IMPLEMENT_SET_OS_MEMBER_FUNC(type, member)					\
void os_set_##type##_##member(struct type *ptr, mt_typeof_member(struct type, member) v)\
{											\
	ptr->member = v;								\
}

/* offsetofend of struct member */
#define IMPLEMENT_GET_OS_MEMBER_OFFSETOFEND_FUNC(type, member)	\
size_t os_offsetofend_##type##_##member(void)			\
{								\
	return offsetofend(struct type, member);		\
}

#define DECLARE_GET_OS_MEMBER_OFFSETOFEND_FUNC(type, member)	\
size_t os_offsetofend_##type##_##member(void)

extern const u64 os_value[];

DECLARE_OS_STRUCT_COMMON_FUNCS(interval_tree_node);
DECLARE_OS_STRUCT_COMMON_FUNCS(rb_root_cached);

/**
 * Interface of interval tree and rbtree
 */
unsigned long os_get_interval_tree_node_start(struct interval_tree_node *node);
void os_set_interval_tree_node_value(struct interval_tree_node *node, u64 start, u64 length);
void os_interval_tree_clear_node(struct interval_tree_node *node);
bool os_interval_tree_node_is_empty(struct interval_tree_node *node);
void os_interval_tree_insert(struct interval_tree_node *node, struct rb_root_cached *root);
void os_interval_tree_remove(struct interval_tree_node *node, struct rb_root_cached *root);
struct interval_tree_node *os_interval_tree_iter_next(struct interval_tree_node *node,
						      unsigned long start, unsigned long last);
struct interval_tree_node *os_interval_tree_iter_first(struct rb_root_cached *root,
						       unsigned long start, unsigned long last);
void os_rb_root_init(struct rb_root_cached *root);

#if defined(SUPPORT_ION)
size_t os_ion_query_heaps_kernel(struct ion_device *idev, struct ion_heap_data *hdata,
				 size_t size);
void os_ion_dev_lma_heap_destroy(struct ion_device *dev, const char *name);
void os_ion_platform_heap_init(struct ion_platform_heap *heap, u32 type, unsigned int id,
			       const char *name, phys_addr_t vram_base);
struct ion_platform_heap *os_ion_platform_heap_create(void);
int os_ion_device_add_heap(struct ion_device *idev, struct ion_heap *heap);
struct ion_device *os_ion_device_create(void);
int os_ion_device_destroy(struct ion_device *idev);
const char *os_ion_get_dev_name(struct ion_device *idev);
struct ion_heap_data *os_alloc_ion_heap_data_array(u32 count);
char *os_get_ion_heap_name(struct ion_heap_data *data, int i);
u32 os_get_ion_heap_id(struct ion_heap_data *data, int i);
#endif /*SUPPORT_ION*/

struct shash_desc;
struct crypto_shash;
struct shash_desc *os_create_shash_desc(struct crypto_shash *tfm);
void os_destroy_shash_desc(struct shash_desc *desc);
struct crypto_shash *os_crypto_alloc_shash(const char *alg_name, u32 type, u32 mask);
int os_crypto_shash_digest(struct shash_desc *desc, const u8 *data, unsigned int len, u8 *out);
void os_crypto_free_shash(struct crypto_shash *tfm);

/**
 * Interface for get members of the structure.
 */
struct scatterlist *os_get_sg_table_sgl(struct sg_table *sgt);
unsigned int os_get_sg_table_nents(struct sg_table *sgt);
unsigned int os_get_sg_table_orig_nents(struct sg_table *sgt);

#define OS_SG_TABLE_MEMBER(ptr, member)	(os_get_sg_table_##member(ptr))

/* members of dma_buf */
size_t os_get_dma_buf_size(struct dma_buf *dma_buf);
const struct dma_buf_ops *os_get_dma_buf_ops(struct dma_buf *dma_buf);
void *os_get_dma_buf_priv(struct dma_buf *dma_buf);

#define OS_DMA_BUF_MEMBER(ptr, member)	(os_get_dma_buf_##member(ptr))

struct device *os_get_dma_buf_attachment_device(struct dma_buf_attachment *attach);
struct device *os_get_device_parent(struct device *dev);

bool os_dev_is_pci(struct device *dev);
struct pci_dev *os_to_pci_dev(struct device *dev);
struct kobject *os_get_device_kobj(struct device *dev);
void os_device_lock(struct device *dev);
void os_device_unlock(struct device *dev);
bool os_is_power_of_2(unsigned long n);
void *os_dev_get_drvdata(const struct device *dev);
void os_dev_set_drvdata(struct device *dev, void *data);
struct device *os_get_device(struct device *dev);
void os_put_device(struct device *dev);
int os_device_attach(struct device *dev);

int os_find_first_bit(const unsigned long *p, unsigned int size);
int os_find_next_bit(const unsigned long *p, int size, int offset);
void *os_kmalloc(size_t size);
void *os_kmalloc_atomic(size_t size);
void *os_kzalloc(size_t size);
void os_kfree(const void *ptr);
void *os_devm_kzalloc(struct device *dev, size_t size);
void *os_vmalloc(unsigned long size);
void *os_vzalloc(unsigned long size);
void *os_kvzalloc(size_t size);
void *os_kcalloc(size_t n, size_t size);
void os_kvfree(const void *addr);
void os_vfree(const void *addr);
unsigned long os_page_to_pfn(struct page *page);
struct page *os_pfn_to_page(u64 pfn);
void os_get_page(struct page *page);
int os_page_count(struct page *page);
u64 os_page_to_phys(struct page *page);
struct page *os_vmalloc_to_page(const void *vmalloc_addr);
struct page *os_phys_to_page(phys_addr_t pa);
struct apertures_struct *os_alloc_apertures(unsigned int max_num);
int os_sg_table_create(struct sg_table **sgt);
void os_sg_table_destroy(struct sg_table *sgt);
struct scatterlist *os_sg_next(struct scatterlist *sg);
dma_addr_t os_sg_dma_address(struct scatterlist *sg);
unsigned int os_sg_dma_len(struct scatterlist *sg);
void os_set_sg_dma_address(struct scatterlist *sg, dma_addr_t dev_addr);
void os_set_sg_dma_len(struct scatterlist *sg, unsigned int dma_len);
int os_sg_alloc_table_from_pages(struct sg_table *sgt, struct page **pages,
				 unsigned int n_pages, unsigned int offset,
				 unsigned long size);
int os_sg_alloc_table(struct sg_table *sgt, unsigned int nents);
void os_sg_free_table(struct sg_table *sgt);
void *os_vmap(struct page **pages, unsigned int count);
void os_vunmap(const void *addr);
int os_remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
		       unsigned long pfn, unsigned long size, unsigned long pgprot);
int os_dma_map_sg(struct device *dev, struct scatterlist *sg,
		  int nents, u64 dir);
void os_dma_unmap_sg(struct device *dev, struct scatterlist *sg,
		     int nents, u64 dir);
dma_addr_t os_dma_map_page(struct device *dev, struct page *page, size_t offset,
			   size_t size, u64 dir);
void os_dma_unmap_page(struct device *dev, dma_addr_t dma_addr, size_t size, u64 dir);
dma_addr_t os_dma_map_resource(struct device *dev, phys_addr_t phys_addr,
			       size_t size, u64 dir);
void os_dma_unmap_resource(struct device *dev, dma_addr_t addr, size_t size, u64 dir);
int os_dma_mapping_error(struct device *dev, dma_addr_t addr);
int os_dma_direct_map_sg_dummy(struct device *dev, struct scatterlist *sgl,
			       int nents, u64 dir);
void os_dma_sync_sg_for_device(struct device *dev, struct scatterlist *sg,
			       int nelems, u64 dir);
void os_dma_sync_sg_for_cpu(struct device *dev, struct scatterlist *sg,
			    int nelems, u64 dir);

struct pid *os_find_vpid(int nr);
int os_kill_pid(struct pid *pid, int sig, int priv);
struct task_struct *os_pid_task(struct pid *pid, int type);
struct mm_struct *os_get_task_mm(struct task_struct *p);
void os_task_lock(struct task_struct *p);
void os_task_unlock(struct task_struct *p);
char *os_d_path(const struct path *p, char *param, int size);

struct path *os_get_exec_vma_file_path(struct mm_struct *mm);

unsigned long os_get_vm_area_struct_vm_start(struct vm_area_struct *vma);
unsigned long os_get_vm_area_struct_vm_end(struct vm_area_struct *vma);
unsigned long os_get_vm_area_struct_vm_flags(struct vm_area_struct *vma);
unsigned long os_get_vm_area_struct_vm_page_prot(struct vm_area_struct *vma);
unsigned long os_get_vm_area_struct_vm_pgoff(struct vm_area_struct *vma);
struct file *os_get_vm_area_struct_vm_file(struct vm_area_struct *vma);
void os_set_vm_area_struct_vm_flags(struct vm_area_struct *vma, unsigned long flag);

void *os_memset(void *s, int c, size_t count);
void *os_memcpy(void *dst, const void *src, size_t size);
void os_memcpy_fromio(void *dst, const void __iomem *src, size_t size);
void os_memcpy_toio(void __iomem *dst, const void *src, size_t size);
void os_memset_io(void __iomem *addr, int value, size_t size);
void *os_memdup_user(const void __user *, size_t);
unsigned long os_copy_from_user(void *to, const void *from, unsigned long n);
unsigned long os_copy_to_user(void __user *to, const void *from, unsigned long n);

__poll_t os_key_to_poll(void *key);
void os_init_poll_funcptr(poll_table *pt, poll_queue_proc qproc);
__poll_t os_vfs_poll(struct file *file, struct poll_table_struct *pt);
void os_poll_wait(struct file *filp, struct wait_queue_head *wait_address,
		  struct poll_table_struct *p);

void os_seq_puts(struct seq_file *m, const char *s);
void os_seq_putc(struct seq_file *m, char c);
void os_seq_put_decimal_ll(struct seq_file *m, const char *delimiter, long long num);
ssize_t os_seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
loff_t os_seq_lseek(struct file *file, loff_t offset, int whence);
int os_single_open(struct file *file, int (*show)(struct seq_file *, void *), void *data);
int os_single_release(struct inode *inode, struct file *file);
void *os_get_seq_file_private(struct seq_file *seq);
struct proc_dir_entry *os_proc_create_data(const char *name, umode_t mode,
					   struct proc_dir_entry *parent,
					   const void *proc_fops, void *data);
struct proc_dir_entry *os_proc_create_single_data(const char *name, umode_t mode,
						  struct proc_dir_entry *parent,
						  int (*show)(struct seq_file *, void *),
						  void *data);
struct proc_dir_entry *os_proc_mkdir(const char *name, struct proc_dir_entry *parent);
void os_proc_remove(struct proc_dir_entry *de);
void os_remove_proc_entry(const char *name, struct proc_dir_entry *parent);
int os_atomic_read_this_module_refcnt(void);
void *os_pde_data(const struct inode *inode);
void os_seq_printf(struct seq_file *m, const char *f, ...);

void *os_miscdevice_create(void);
void os_miscdevice_destroy(struct miscdevice *misc);
int os_miscdevice_init(struct miscdevice *misc, struct device *parent,
		       const char *name, int mode, const struct mt_file_operations *ops);
void os_miscdevice_deinit(struct miscdevice *misc);
int os_misc_register(struct miscdevice *misc);
void os_misc_deregister(struct miscdevice *misc);
void os_set_miscdevice_minor(struct miscdevice *misc, int minor);
void *os_get_miscdevice_drvdata(struct miscdevice *misc);
void os_set_miscdevice_drvdata(struct miscdevice *misc, void *data);
int os_set_miscdevice_name(struct miscdevice *misc, const char *data);
void os_clear_miscdevice_name(struct miscdevice *misc);
const char *os_get_miscdevice_name(struct miscdevice *misc);
struct device *os_get_miscdevice_parent(struct miscdevice *misc);
int os_get_miscdevice_minor(struct miscdevice *misc);

void os_set_file_private_data(struct file *file, void *private_data);
void *os_get_file_private_data(struct file *file);
void *os_get_file_node_private_data(struct file *file);
unsigned int os_get_file_flags(struct file *file);

struct fd *os_fdget(unsigned int fd);
void os_fdput(struct fd *fd);
struct file *os_get_fd_file(struct fd *fd);

void os_kref_init(mt_kref *kref);
int os_kref_put(mt_kref *kref, void (*release)(mt_kref *kref));
void os_kref_get(mt_kref *kref);

int os_ida_create(struct ida **ida);
void os_ida_destroy(struct ida *ida);
int os_ida_alloc(struct ida *ida);
int os_ida_alloc_range(struct ida *ida, unsigned int min, unsigned int max);
void os_ida_free(struct ida *ida, unsigned long id);
bool os_ida_is_empty(const struct ida *ida);

int os_mutex_create(struct mutex **lock);
void os_mutex_lock(struct mutex *lock);
int os_mutex_trylock(struct mutex *lock);
void os_mutex_unlock(struct mutex *lock);
void os_mutex_destroy(struct mutex *lock);

int os_spin_lock_create(spinlock_t **lock);
void os_spin_lock(spinlock_t *lock);
void os_spin_unlock(spinlock_t *lock);
void os_spin_lock_irqsave(spinlock_t *lock, unsigned long *flags);
void os_spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags);
void os_spin_lock_bh(spinlock_t *lock);
void os_spin_unlock_bh(spinlock_t *lock);
void os_spin_lock_destroy(spinlock_t *lock);

int os_kfifo_create(struct kfifo **fifo);
int os_kfifo_alloc(struct kfifo *fifo, unsigned int size);
void os_kfifo_free(struct kfifo *fifo);
unsigned int os_kfifo_in(struct kfifo *fifo, const void *from, unsigned int len);
unsigned int os_kfifo_out(struct kfifo *fifo, void *to, unsigned int len);
unsigned int os_kfifo_in_locked(struct kfifo *fifo, const void *from, unsigned int len,
				spinlock_t *lock);
unsigned int os_kfifo_out_locked(struct kfifo *fifo, void *to, unsigned int len, spinlock_t *lock);
int os_kfifo_is_empty(struct kfifo *fifo);
int os_kfifo_is_full(struct kfifo *fifo);

int os_sema_create(struct semaphore **sem, int val);
void os_up(struct semaphore *sem);
int os_down_timeout(struct semaphore *sem, long timeout);
void os_sema_destroy(struct semaphore *sem);

unsigned long os_get_jiffies(void);
unsigned long os_msecs_to_jiffies(const unsigned int m);

struct tasklet_struct *os_create_tasklet(void (*fun)(unsigned long), unsigned long data);
void os_destroy_tasklet(struct tasklet_struct *tasklet);
void os_tasklet_schedule(struct tasklet_struct *tasklet);

void *os_get_work_drvdata(struct work_struct *work);
void os_set_work_drvdata(struct work_struct *work, void *data);
void *os_create_work(void);
void *os_create_work_atomic(void);
void os_destroy_work(struct work_struct *work);
bool os_queue_work(struct workqueue_struct *wq, struct work_struct *work);
void *os_get_dwork_drvdata(struct work_struct *work);
void os_set_dwork_drvdata(struct delayed_work *dwork, void *data);
void *os_create_dwork(void);
void os_destroy_dwork(struct delayed_work *dwork);
void os_init_dwork(struct delayed_work *work, work_func_t func);
void os_init_delayed_work(struct delayed_work *dwork, work_func_t func);
bool os_queue_delayed_work(struct workqueue_struct *wq,
			   struct delayed_work *dwork,
			   unsigned long delay);
bool os_cancel_delayed_work_sync(struct delayed_work *dwork);
void os_destroy_workqueue(struct workqueue_struct *wq);
struct workqueue_struct *os_create_freezable_workqueue(char *name);
struct workqueue_struct *os_alloc_workqueue(const char *fmt, unsigned int flags, int max_active);
struct workqueue_struct *os_alloc_ordered_workqueue(const char *fmt);
void os_flush_workqueue(struct workqueue_struct *wq);
void os_wake_up(struct wait_queue_head *wq_head);
void os_wake_up_interruptible(struct wait_queue_head *wq_head);
void os_wake_up_all(struct wait_queue_head *wq_head);
int os_create_waitqueue_head(struct wait_queue_head **wq_head);
void os_destroy_waitqueue_head(struct wait_queue_head *wq_head);
void os_might_sleep(void);
long os_schedule_timeout(long timeout);
void os_init_wait_entry(struct wait_queue_entry *wq_entry, int flags);
void os_finish_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry);
long os_prepare_to_wait_event_uninterruptible(struct wait_queue_head *wq_head,
					      struct wait_queue_entry *wq_entry);
void os_add_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry);
void os_init_waitqueue_func_entry(struct wait_queue_entry *wq_entry, wait_queue_func_t func);

void os_blocking_init_notifier_head(struct blocking_notifier_head *nh);
void os_notifier_block_set_notifier_call(struct notifier_block *nb, notifier_fn_t cb);
int os_blocking_notifier_call_chain(struct blocking_notifier_head *nh,
				    unsigned long val, void *v);
int os_blocking_notifier_chain_register(struct blocking_notifier_head *nh,
					struct notifier_block *n);
int os_blocking_notifier_chain_unregister(struct blocking_notifier_head *nh,
					  struct notifier_block *n);
int os_blocking_notifier_head_create(struct blocking_notifier_head **nh);
void os_blocking_notifier_head_destroy(struct blocking_notifier_head *nh);

int os_device_attr_create(struct device_attribute **dev_attr, const char *name, umode_t mode,
			  ssize_t (*show)(struct device *dev,
					  struct device_attribute *attr,
					  char *buf),
			  ssize_t (*store)(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf,
					   size_t count));
void os_device_attr_destroy(struct device_attribute *dev_attr);
struct attribute *os_get_device_attr_attr(struct device_attribute *dev_attr);
int os_sysfs_create_file(struct kobject *kobj, const struct attribute *attr);
void os_sysfs_remove_file(struct kobject *kobj, const struct attribute *attr);

#define os_wait_cond_timeout(condition, ret)					\
({										\
	bool cond = (condition);						\
	if (cond && !ret)							\
		ret = 1;							\
	cond || !ret;								\
})

#define os_wait_event_timeout(wq_head, condition, timeout)					\
({												\
	long ret1 = timeout;									\
	os_might_sleep();									\
	if (!os_wait_cond_timeout(condition, ret1)) {						\
		ret1 =										\
		({										\
			struct wait_queue_entry *wq_entry = os_create_wait_queue_entry();	\
			long ret2 = timeout;	/* explicit shadow */				\
												\
			os_init_wait_entry(wq_entry, 0);					\
			for (;;) {								\
				os_prepare_to_wait_event_uninterruptible(wq_head, wq_entry);	\
												\
				if (os_wait_cond_timeout(condition, ret2))			\
					break;							\
												\
				ret2 = os_schedule_timeout(ret2);				\
			}									\
			os_finish_wait(wq_head, wq_entry);					\
			os_kfree(wq_entry);							\
			ret2;									\
		});										\
	}											\
	ret1;											\
})

void os_init_work(struct work_struct *work, work_func_t func);
bool os_schedule_work(struct work_struct *work);
bool os_flush_work(struct work_struct *work);
bool os_cancel_work_sync(struct work_struct *work);
bool os_schedule_delayed_work(struct delayed_work *dwork, unsigned long delay);
struct task_struct *os_kthread_create(int (*threadfn)(void *data),
				      void *data, const char *namefmt, ...);
int os_kthread_stop(struct task_struct *k);
bool os_kthread_should_stop(void);
int os_wake_up_process(struct task_struct *p);

void os_wmb(void);
void os_mb(void);

int os_arch_io_reserve_memtype_wc(resource_size_t base, resource_size_t size);
void os_arch_io_free_memtype_wc(resource_size_t base, resource_size_t size);
int os_arch_phys_wc_add(unsigned long base, unsigned long size);
void os_arch_phys_wc_del(int handle);

void __iomem *os_ioremap(phys_addr_t phys_addr, size_t size);
void __iomem *os_ioremap_cache(resource_size_t offset, unsigned long size);
void os_iounmap(void __iomem *io_addr);

int os_get_user_pages_fast(unsigned long start, int nr_pages,
			   unsigned int gup_flags, struct page **pages);
void os_put_page(struct page *page);
int os_page_reserved(struct page *page);
void os_set_pages_reserved(struct page *pages, int n);
void os_clear_pages_reserved(struct page *pages, int n);
struct page *os_alloc_pages(gfp_t gfp_mask, unsigned int order);
void os_free_pages(struct page *page, unsigned int order);
int os_get_order(unsigned long size);

unsigned int os_ioread32(void __iomem *addr);
u32 os_readl(const void __iomem *addr);
void os_iowrite16(u16 b, void __iomem *addr);
void os_iowrite32(u32 b, void __iomem *addr);
void os_writel(u32 value, void __iomem *addr);

int os_pci_domain_nr(struct pci_dev *pdev);
int os_request_pci_io_addr(struct pci_dev *pdev, u32 index,
			   resource_size_t offset, resource_size_t length);
unsigned int os_pci_slot(unsigned int devfn);
unsigned int os_pci_func(unsigned int devfn);
unsigned int os_get_pci_dev_virfn(struct pci_dev *pdev);
struct pci_dev *os_get_pci_dev_physfn(struct pci_dev *pdev);
struct device *os_get_pci_device_base(struct pci_dev *pdev);
unsigned short os_get_pci_device_vendor(struct pci_dev *pdev);
unsigned short os_get_pci_subsystem_vendor(struct pci_dev *pdev);
unsigned short os_get_pci_device_id(struct pci_dev *dev);
unsigned short os_get_pci_subsystem_device_id(struct pci_dev *pdev);
struct pci_bus *os_get_pci_bus(struct pci_dev *pdev);
struct resource *os_get_pci_resource(struct pci_dev *pdev);
pci_power_t os_get_pci_current_state(struct pci_dev *pdev);
kernel_ulong_t os_get_pci_device_data(const struct pci_device_id *id);
unsigned int os_get_pci_devfn(struct pci_dev *pdev);
unsigned char os_get_pci_bus_number(struct pci_dev *pdev);
unsigned int os_get_pci_irq(struct pci_dev *pdev);
resource_size_t os_pci_resource_start(struct pci_dev *pdev, int bar);
resource_size_t os_pci_resource_end(struct pci_dev *pdev, int bar);
unsigned long os_pci_resource_flags(struct pci_dev *pdev, int bar);
resource_size_t os_pci_resource_len(struct pci_dev *pdev, int bar);
int os_pci_msix_vec_count(struct pci_dev *dev);
int os_pci_msi_vec_count(struct pci_dev *dev);
bool os_pci_support_msix(struct pci_dev *dev);
bool os_pci_support_msi(struct pci_dev *dev);
int os_pci_irq_vector(struct pci_dev *dev, unsigned int nr);
int os_pci_alloc_irq_vectors(struct pci_dev *dev, unsigned int min_vecs,
			     unsigned int max_vecs, unsigned int flags);
int os_pci_enable_device(struct pci_dev *dev);
void os_pci_disable_device(struct pci_dev *dev);
void os_pci_set_drvdata(struct pci_dev *pdev, void *drvdata);
void *os_pci_get_drvdata(struct pci_dev *pdev);
void os_pci_set_master(struct pci_dev *dev);
void os_pci_clear_master(struct pci_dev *dev);
int os_pci_load_and_free_saved_state(struct pci_dev *dev, struct pci_saved_state **state);
int os_pci_load_saved_state(struct pci_dev *dev, struct pci_saved_state *state);
void os_pci_restore_state(struct pci_dev *dev);
int os_pci_save_state(struct pci_dev *dev);
int os_pci_set_power_state(struct pci_dev *dev, pci_power_t state);
struct pci_saved_state *os_pci_store_saved_state(struct pci_dev *dev);

int os_pci_find_ext_capability(struct pci_dev *dev, int cap);
int os_pci_read_config_byte(const struct pci_dev *dev, int where, u8 *val);
int os_pci_read_config_word(const struct pci_dev *dev, int where, u16 *val);
int os_pci_write_config_word(const struct pci_dev *dev, int where, u16 val);
int os_pci_read_config_dword(const struct pci_dev *dev, int where, u32 *val);
int os_pci_write_config_dword(const struct pci_dev *dev, int where, u32 val);
void os_pci_release_resource(struct pci_dev *dev, int resno);
int os_pci_resize_resource(struct pci_dev *dev, int resno, int size);
void os_pci_assign_unassigned_bus_resources(struct pci_bus *bus);
int os_pci_enable_pcie_error_reporting(struct pci_dev *pdev);
int os_pci_disable_pcie_error_reporting(struct pci_dev *pdev);
struct resource *os_pci_bus_resource_n(const struct pci_bus *bus, int n);

int os_pci_dev_wait(struct pci_dev *dev, char *reset_type, int timeout);

int os_pci_wait_for_pending_transaction(struct pci_dev *dev);

void os_pci_release_related_resources(struct pci_dev *pdev, int DDR_BAR_NUM);
void os_check_root_pcibus(struct pci_bus *root, struct resource **res);
struct pci_dev *os_find_parent_pci_dev(struct device *dev);

struct pci_dev *os_pci_dev_get(struct pci_dev *dev);
void os_pci_dev_put(struct pci_dev *dev);
struct pci_dev *os_pci_get_device(unsigned int vendor, unsigned int device,
				  struct pci_dev *from);

int os_pci_sriov_get_totalvfs(struct pci_dev *dev);
int os_pci_enable_sriov(struct pci_dev *dev, int nr_virtfn);
void os_pci_disable_sriov(struct pci_dev *dev);

struct pci_dev *os_pci_find_upstream_switch(struct pci_dev *pdev);

bool os_pci_has_same_root_port(struct pci_dev *pdev1, struct pci_dev *pdev2);
bool os_pci_is_under_same_switch(struct pci_dev *pdev1,
				 struct pci_dev *pdev2);

int os_pcie_capability_read_dword(struct pci_dev *dev, int where, u32 *val);
int os_pcie_capability_set_word(struct pci_dev *dev, int where, u16 set);

struct resource *os_request_region(resource_size_t start,
				   resource_size_t n,
				   const char *name);
void os_release_region(resource_size_t start, resource_size_t n);
struct resource *os_request_mem_region(resource_size_t start,
				       resource_size_t n,
				       const char *name);
void os_release_mem_region(resource_size_t start, resource_size_t n);
struct resource *os_create_resource(struct mtgpu_resource *mtgpu_res, u32 num_res);
void os_destroy_resource(struct resource *resource);
int os_release_resource(struct resource *new);
resource_size_t os_resource_size(const struct resource *res);
resource_size_t os_resource_start(const struct resource *res);
resource_size_t os_resource_end(const struct resource *res);
int os_walk_iomem_res_desc(unsigned long desc, unsigned long flags, u64 start,
			   u64 end, void *arg, int (*func)(struct resource *, void *));

struct platform_device_info *os_create_platform_device_info(struct device *dev,
							    const char *name,
							    int id,
							    const struct resource *res,
							    unsigned int num_res,
							    const void *data,
							    size_t size_data,
							    u64 dma_mask);
void os_destroy_platform_device_info(struct platform_device_info *pdev_info);
void *os_platform_get_drvdata(struct platform_device *pdev);
void os_platform_set_drvdata(struct platform_device *pdev, void *data);
struct platform_device *
os_platform_device_register_full(const struct platform_device_info *pdevinfo);
void os_platform_device_unregister(struct platform_device *pdev);
struct resource *os_platform_get_resource(struct platform_device *dev,
					  unsigned int type,
					  unsigned int num);
struct device *os_get_platform_device_base(struct platform_device *pdev);
struct platform_device *os_to_platform_device(struct device *dev);

u64 os_roundup_pow_of_two(u64 size);
u32 os_order_base_2(u64 size);
int os_fls(unsigned int x);
int os_fls64(unsigned long x);

resource_size_t os_get_system_available_ram_size(void);
resource_size_t os_get_system_free_ram_size(void);
resource_size_t os_get_system_total_ram_size(void);
void os_get_cached_msi_msg(unsigned int irq, struct msi_msg *msg);
struct msi_desc *os_irq_get_msi_desc(unsigned int irq);
struct msi_msg *os_msi_msg_alloc(void);
int os_get_msi_message(struct msi_msg *msiptr, u32 *addr_lo, u32 *addr_hi, u32 *data);
int os_get_msi_message_from_desc(struct msi_desc *desc, u32 *addr_lo, u32 *addr_hi, u32 *data);
int os_request_irq(unsigned int irq, void *handler, unsigned long flags,
		   const char *name, void *dev);
const void *os_free_irq(unsigned int irq, void *dev_id);
void os_pci_free_irq_vectors(struct pci_dev *pdev);

void os_msleep(unsigned int msecs);
void os_udelay(unsigned long secs);
void os_mdelay(unsigned long secs);
void os_usleep_range(unsigned long min, unsigned long max);

int os_register_pm_notifier(struct notifier_block *nb);
int os_unregister_pm_notifier(struct notifier_block *nb);

void os_dma_buf_put(struct dma_buf *dmabuf);
struct dma_buf *os_dma_buf_get(int fd);
void *os_get_dmabuf_from_attachment(struct dma_buf_attachment *attach);
int os_dma_set_mask_and_coherent(struct device *dev, u64 mask);
void *os_dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t gfp);
void os_dma_free_coherent(struct device *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle);
int os_dma_mmap_attrs(struct device *dev, struct vm_area_struct *vma, void *cpu_addr,
		      dma_addr_t dma_addr, size_t size, unsigned long attrs);

int os_request_firmware(const struct firmware **fw, const char *name, struct device *device);
void os_release_firmware(const struct firmware *fw);
const u8 *os_get_firmware_data(const struct firmware *fw);
size_t os_get_firmware_size(const struct firmware *fw);

unsigned long os_nsecs_to_jiffies(u64 n);
unsigned long os_get_tick_us(void);
void os_get_random_bytes(void *buf, int nbytes);

int os_mod_timer(struct timer_list *timer, unsigned long expires);
int os_del_timer_sync(struct timer_list *timer);
void os_timer_setup(struct timer_list *timer, void (*function)(struct timer_list *),
		    unsigned int flags);
int os_create_timer(struct timer_list **timer);
void os_destroy_timer(struct timer_list *timer);

u64 os_kclock_ns64(void);
u64 os_ktime_get_ns(void);

void os_ktime_get_real_tm(struct mt_tm *mt_time, int offset);

struct inode *os_file_inode(const struct file *f);

void os_list_sort(void *priv, struct list_head *head,
		  int (*cmp)(void *priv, const struct list_head *a,
			     const struct list_head *b));
struct radix_tree_root *os_create_radix_tree(void);
void os_destroy_radix_tree(struct radix_tree_root *root);
void *os_radix_tree_lookup(const struct radix_tree_root *root, unsigned long index);
void *os_radix_tree_delete(struct radix_tree_root *root, unsigned long index);
int os_radix_tree_insert(struct radix_tree_root *root, unsigned long index, void *item);
struct radix_tree_iter *os_create_radix_tree_iter(void);
void os_destroy_radix_tree_iter(struct radix_tree_iter *iter);
unsigned long os_get_radix_tree_iter_index(struct radix_tree_iter *iter);
void **os_radix_tree_iter_init(struct radix_tree_iter *iter, unsigned long start);
void **os_radix_tree_next_chunk(const struct radix_tree_root *root,
				struct radix_tree_iter *iter, unsigned int flags);
void **os_radix_tree_next_slot(void **slot, struct radix_tree_iter *iter, unsigned int flags);

u64 os_eventfd_signal(struct eventfd_ctx *ctx, __u64 n);
void os_eventfd_ctx_put(struct eventfd_ctx *ctx);
struct eventfd_ctx *os_eventfd_ctx_fdget(int fd);

int os_ilog2(u64 n);

mempool_t *os_mempool_create(int min_nr, mempool_alloc_t *alloc_fn,
			     mempool_free_t *free_fn, void *pool_data);
void os_mempool_destroy(mempool_t *pool);
void *os_mempool_alloc(mempool_t *pool, gfp_t gfp_mask);
void os_mempool_free(void *element, mempool_t *pool);

/*About dev print*/
void _os_dev_emerg(const struct device *dev, const char *fmt, ...);
void _os_dev_crit(const struct device *dev, const char *fmt, ...);
void _os_dev_alert(const struct device *dev, const char *fmt, ...);
void _os_dev_err(const struct device *dev, const char *fmt, ...);
void _os_dev_warn(const struct device *dev, const char *fmt, ...);
void _os_dev_notice(const struct device *dev, const char *fmt, ...);
void _os_dev_info(const struct device *dev, const char *fmt, ...);
void _os_dev_dbg(const struct device *dev, const char *fmt, ...);

int os_snprintf(char *buf, size_t size, const char *fmt, ...);
int os_printk(const char *fmt, ...);
void os_printk_ratelimited(const char *fmt, ...);

bool OS_IS_ERR(const void *ptr);
bool OS_IS_ERR_OR_NULL(__force const void *ptr);
long OS_PTR_ERR(__force const void *ptr);
void *OS_ERR_PTR(long error);
int OS_READ_ONCE(int *val);
void OS_WARN_ON(bool condition);
bool OS_WARN_ON_ONCE(bool condition);
void OS_BUG_ON(bool condition);

int os_sscanf(const char *str, const char *fmt, ...);
size_t os_strlen(const char *s);
char *os_strcat(char *dest, const char *src);
int os_strcmp(const char *cs, const char *ct);
int os_strncmp(const char *cs, const char *ct, size_t count);
char *os_strcpy(char *dest, const char *src);
char *os_strncpy(char *dest, const char *src, size_t count);
char *os_strchr(const char *, int);
char *os_strstr(const char *s1, const char *s2);
int os_kstrtol(const char *s, unsigned int base, long *res);
int os_sprintf(char *buf, const char *fmt, ...);
char *os_strtrim(char *src);

int os_iommu_map(struct iommu_domain *domain, unsigned long iova,
		 phys_addr_t paddr, size_t size, int prot);
size_t os_iommu_unmap(struct iommu_domain *domain, unsigned long iova, size_t size);
void os_iommu_group_put(struct iommu_group *group);
int os_iommu_group_id(struct iommu_group *group);
struct iommu_group *os_iommu_group_get(struct device *dev);
int os_iommu_attach_group(struct iommu_domain *domain, struct iommu_group *group);
void os_iommu_detach_group(struct iommu_domain *domain, struct iommu_group *group);
struct iommu_domain *os_iommu_domain_alloc(struct bus_type *bus);
void os_iommu_domain_free(struct iommu_domain *domain);
struct iommu_domain *os_iommu_get_domain_for_dev(struct device *dev);
unsigned int os_get_iommu_domain_type(struct iommu_domain *domain);
bool os_iommu_present(struct bus_type *bus);
phys_addr_t os_virt_to_phys(void *address);

struct device *os_get_dev_parent_parent(struct device *dev);
struct device *os_get_dev_parent(struct device *dev);
void *os_get_device_driver_data(struct device *dev);

char *os_get_current_comm(void);
u64 os_get_current_pid(void);
u64 os_get_current_tgid(void);

char *os_get_utsname_version(void);

struct bus_type *os_get_dev_bus_type(struct device *dev);

/**
 * Interfaces for querying slot information in DMI devices.
 */
const struct dmi_device *os_dmi_find_slot_device(const char *name, const struct dmi_device *dev);
const struct dmi_dev_onboard *os_get_dmi_device_data(const struct dmi_device *dev);
int os_get_dmi_device_board_segment(const struct dmi_dev_onboard *dev_onboard);
int os_get_dmi_device_board_bus(const struct dmi_dev_onboard *dev_onboard);
int os_get_dmi_device_board_devfn(const struct dmi_dev_onboard *dev_onboard);
int os_get_dmi_device_board_instance(const struct dmi_dev_onboard *dev_onboard);
const char *os_get_dmi_device_board_name(const struct dmi_dev_onboard *dev_onboard);

void os_bitmap_set(unsigned long *map, unsigned int start, unsigned int nbits);
void os_bitmap_clear(unsigned long *map, unsigned int start, unsigned int nbits);
int os_bitmap_empty(const unsigned long *src, unsigned int nbits);
int os_bitmap_full(const unsigned long *src, unsigned int nbits);
void os_bitmap_free(const unsigned long *bitmap);
unsigned long *os_bitmap_zalloc(unsigned int nbits, gfp_t flags);
unsigned long os_bitmap_find_next_zero_area(unsigned long *map,
					    unsigned long size,
					    unsigned long start,
					    unsigned int nr,
					    unsigned long align_mask);

void *os_kmem_cache_alloc(struct kmem_cache *cachep, gfp_t flags);
void os_kmem_cache_free(struct kmem_cache *cachep, void *objp);
struct kmem_cache *os_kmem_cache_create(const char *name, unsigned int size,
					unsigned int align, slab_flags_t flags,
					void (*ctor)(void *));
void os_kmem_cache_destroy(struct kmem_cache *s);

int os_atomic_xchg(atomic_t *v, int val);
void os_atomic_set(atomic_t *v, int val);
void os_atomic_inc(atomic_t *v);
bool os_atomic_dec_and_test(atomic_t *v);
int os_atomic_read(atomic_t *v);

struct file *os_filp_open(const char *filename, int flags, umode_t mode);
int os_filp_close(struct file *filp);
ssize_t os_kernel_write(void *filp, const char __user *buf, size_t count, loff_t *pos);

DECLARE_OS_STRUCT_COMMON_FUNCS(notifier_block);
DECLARE_OS_STRUCT_COMMON_FUNCS(poll_table_struct);
DECLARE_OS_STRUCT_COMMON_FUNCS(wait_queue_entry);
DECLARE_OS_STRUCT_COMMON_FUNCS(timer_list);

#ifndef KERN_SOH
#define KERN_SOH	"\001"		/* ASCII Start Of Header */
#define KERN_SOH_ASCII	'\001'

#define KERN_EMERG	KERN_SOH "0"	/* system is unusable */
#define KERN_ALERT	KERN_SOH "1"	/* action must be taken immediately */
#define KERN_CRIT	KERN_SOH "2"	/* critical conditions */
#define KERN_ERR	KERN_SOH "3"	/* error conditions */
#define KERN_WARNING	KERN_SOH "4"	/* warning conditions */
#define KERN_NOTICE	KERN_SOH "5"	/* normal but significant condition */
#define KERN_INFO	KERN_SOH "6"	/* informational */
#define KERN_DEBUG	KERN_SOH "7"	/* debug-level messages */

#define KERN_DEFAULT	""		/* the default kernel loglevel */
#endif

#define os_pr_fmt(fmt) fmt

#define os_pr_emerg(fmt, ...)							\
	os_printk(KERN_EMERG os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_alert(fmt, ...)							\
	os_printk(KERN_ALERT os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_crit(fmt, ...)							\
	os_printk(KERN_CRIT os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_err(fmt, ...)							\
	os_printk(KERN_ERR os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_warning(fmt, ...)							\
	os_printk(KERN_WARNING os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_warn os_pr_warning
#define os_pr_notice(fmt, ...)							\
	os_printk(KERN_NOTICE os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_info(fmt, ...)							\
	os_printk(KERN_INFO os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_cont(fmt, ...)							\
	os_printk(KERN_CONT os_pr_fmt(fmt), ##__VA_ARGS__)

#ifdef DEBUG
#define os_pr_debug(fmt, ...)							\
	os_printk(KERN_DEBUG os_pr_fmt(fmt), ##__VA_ARGS__)
#else
#define os_pr_debug(fmt, ...)	({})
#endif

#define os_pr_emerg_ratelimited(fmt, ...)					\
	os_printk_ratelimited(KERN_EMERG os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_alert_ratelimited(fmt, ...)					\
	os_printk_ratelimited(KERN_ALERT os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_crit_ratelimited(fmt, ...)					\
	os_printk_ratelimited(KERN_CRIT os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_err_ratelimited(fmt, ...)						\
	os_printk_ratelimited(KERN_ERR os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_warn_ratelimited(fmt, ...)					\
	os_printk_ratelimited(KERN_WARNING os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_notice_ratelimited(fmt, ...)					\
	os_printk_ratelimited(KERN_NOTICE os_pr_fmt(fmt), ##__VA_ARGS__)
#define os_pr_info_ratelimited(fmt, ...)					\
	os_printk_ratelimited(KERN_INFO os_pr_fmt(fmt), ##__VA_ARGS__)

#define os_dev_fmt(fmt) fmt

#define os_dev_emerg(dev, fmt, ...)						\
	_os_dev_emerg(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#define os_dev_crit(dev, fmt, ...)						\
	_os_dev_crit(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#define os_dev_alert(dev, fmt, ...)						\
	_os_dev_alert(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#define os_dev_err(dev, fmt, ...)						\
	_os_dev_err(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#define os_dev_warn(dev, fmt, ...)						\
	_os_dev_warn(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#define os_dev_notice(dev, fmt, ...)						\
	_os_dev_notice(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#define os_dev_info(dev, fmt, ...)						\
	_os_dev_info(dev, os_dev_fmt(fmt), ##__VA_ARGS__)

#ifdef DEBUG
#define os_dev_dbg(dev, fmt, ...)						\
	_os_dev_dbg(dev, os_dev_fmt(fmt), ##__VA_ARGS__)
#else
#define os_dev_dbg(dev, fmt, ...)	({})
#endif

#endif /* __OS_INTERFACE_H__ */

