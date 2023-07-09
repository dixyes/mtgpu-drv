/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_DRV_H_
#define _MTVPU_DRV_H_

#include "mtgpu_defs.h"
#include "vpuapi.h"

#include "mtvpu_conf.h"
#include "mtvpu_mon.h"

#define INST_Q_DEPTH  COMMAND_QUEUE_DEPTH

#define WAV5_MAX_CORE  2 /* only for QUYU1 */

#define SYNC_INTR_SIZE (WAV5_MAX_CORE * INST_MAX_SIZE)
#define SYNC_ADDR_SIZE (WAV5_MAX_CORE * INST_MAX_SIZE * INST_Q_DEPTH)

#define FW_LOG_BUFFER_SIZE	(16*1024)	//Currently 16k, if not enough enlarge the size

#define MTVPU_SEGMENT_NUM       6
#define MTVPU_SEGMENT_VM       -1

#if (defined SUPPORT_ION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
#define ion_phys_addr_t phys_addr_t
#endif

extern u32 gpu_page_size;
extern u32 gpu_page_shift;

struct file_operations;
struct platform_device;
struct drm_device;
struct drm_ioctl_desc;
struct wait_queue_head;
struct spinlock;
struct semaphore;
struct workqueue_struct;
struct mutex;
struct list_head;
struct drm_gem_object;
struct timer_list;
struct drm_file;
struct mtgpu_gem_object;
struct work_struct;
struct dma_buf;

struct mt_chip;

struct mt_pool {
	struct mtgpu_gem_object *mtgpu_obj;
	dma_addr_t offset;
	u32 count;
	size_t size;
};

struct mt_intr_map {
	u32 core_idx;
	u32 intr_inst;
	u32 intr_reason;
};

struct mt_sync {
	int idx;
	struct mt_intr_map map[SYNC_INTR_SIZE];

	struct spinlock *intr_lock;
	struct semaphore *sema;

	VpuHandle handle[CORE_MAX_SIZE][INST_MAX_SIZE];
	struct mt_virm *vm[CORE_MAX_SIZE][INST_MAX_SIZE];

	struct spinlock *sync_lock;
	struct spinlock *core_lock[CORE_MAX_SIZE];

	struct wait_queue_head *addr_wait[SYNC_ADDR_SIZE];
	bool addr_blocked[SYNC_ADDR_SIZE];
	u64 addr_phys[SYNC_ADDR_SIZE];
	int addr_idx;

	struct wait_queue_head *inst_wait[CORE_MAX_SIZE][INST_MAX_SIZE];
	bool inst_blocked[CORE_MAX_SIZE][INST_MAX_SIZE];
	u32 inst_cmds[CORE_MAX_SIZE][INST_MAX_SIZE];
};

struct mt_node {
	struct drm_gem_object *obj;
	struct list_head list;
	u64 dev_addr;
	void *vir_addr;
	void *bak_addr;
	void *handle;
#ifdef SUPPORT_ION
	struct dma_buf *ion_buf;
#endif
	u32 pool_id;
};

struct mt_core {
	int idx;
	int irq;
	int available;
	int inited;
	int product_id;

	void *regs;
	const struct firmware *fw;

	struct mt_node *common_node; /* common mem */
	struct vpu_instance_pool pool; /* vdi mem */

	int mem_group_id;
	u64 mem_group_base;
	struct drm_device *drm_dev;
#ifdef SUPPORT_ION
	int heap_id;
	struct ion_device *ion_dev;
#endif
	bool serve_all;
	struct wait_queue_head *core_wait;
	struct wait_queue_head *inst_wait[INST_MAX_SIZE];

	int core_intr_reason;
	int core_wait_reason;
	int inst_intr_reason[INST_MAX_SIZE];
	int inst_wait_reason[INST_MAX_SIZE];
	Uint32 fw_version;

	struct mutex *open_lock;
	struct mutex *regs_lock;
	struct list_head mm_head[INST_MAX_SIZE]; /* mm list */

	int drm_ids[INST_MAX_SIZE];
	u32 pool_ids[INST_MAX_SIZE];

	struct mt_chip *priv;
	void *bak_addr;

	u64 core_freq;
	/* The number of ticks used by the enc pic on the VPU within one second */
	/* In MPC mode, one core may serve multiple drm nodes; */
	/* Thus we should calculate cycles for each drm node */
	u64 core_drm_cycle[MTGPU_CORE_COUNT_MAX];

	u32 inst_cnt;
	struct mt_inst_info inst_info[INST_MAX_SIZE];
	struct mt_inst_extra inst_extra[INST_MAX_SIZE];

	struct mt_node *fwlog_node;
	struct mutex *inst_lock[INST_MAX_SIZE];
};

struct mt_file {
	struct list_head head; /* opened list */
	struct mutex *file_lock;
};

struct mt_dec_work_data {
	RenderTargetInfo render;
	DecInitialInfo info;
	VpuHandle handle;
};


#define VDI_MAX_INSTANCE	10

struct mt_inst_pool_map {
	u32 pool_id;
	u32 inst;
	bool used;
};

struct mt_virm {
	int osid;
	int sched;

	u64 mem_group_base;
	int mem_group_id;

	u32 dec_index;
	u32 enc_index;

	struct vm_shared_mem *mem;
	struct mt_file file;

	struct mt_dec_work_data dec_data;

	struct work_struct *dec_work;

	struct list_head list;

	struct mt_inst_pool_map dec_map[VDI_MAX_INSTANCE];
	struct mt_inst_pool_map enc_map[VDI_MAX_INSTANCE];
};

struct mt_chip {
	int idx;
	struct mt_conf conf;
	struct pci_dev *pdev;
	struct device *parent;
	struct device *dev;
	struct drm_device *drm; /* for host use */

	int mem_group_id[MTVPU_SEGMENT_NUM];
	u64 mem_group_base[MTVPU_SEGMENT_NUM];
	int mem_group_cnt;
	struct spinlock *mem_lock;

	struct spinlock *pool_lock;
	struct mt_pool pool_array[MEM_POOL_MAX_SIZE];
	int pool_idx;

	int drm_dev_cnt; /* for mpc use */
#ifdef SUPPORT_ION
	struct ion_device *ion_dev[MTGPU_CORE_COUNT_MAX];
#endif
	struct drm_device *drm_dev[MTGPU_CORE_COUNT_MAX]; /* for native use */
	int drm_video_group_idx[MTGPU_CORE_COUNT_MAX]; /* for mpc use */

	struct mutex *mm_lock;
	void *vaddr;
	u64 bar_base;

	struct mt_core core[CORE_MAX_SIZE];
	u32 enc_idx; /* for request group */
	u32 dec_idx;
	struct semaphore *host_thread_sema;

	struct mt_sync sync;

	struct task_struct *host_thread;
	struct task_struct *group3_thread;
	struct task_struct *sync_thread;

	struct list_head vm_head; /* vm list */
	struct list_head vm_head_group3; /* vm list for group3 */

	struct mt_virm vm;
	struct mutex *vm_lock;
	struct mutex *vm_lock_group3;
	struct semaphore *vm_sema;

	struct dentry *debugfs;
	struct workqueue_struct *workqueue;

	struct timer_list *timer;
};

struct mt_open {
	int coreIdx;
	int instIdx;
	int isDecoder;
	VpuHandle handle;

	struct list_head list;
};

struct mt_fbc
{
	int fbc_type;
	int width;
	int height;
	int sync_flag;
	int header_size[2];
	int data_size[2];
	int reserved[4];
};

int vpu_node_find(struct mt_chip *chip, struct drm_device *drm);
int vpu_init_irq(struct mt_chip *chip, struct platform_device *pdev);
void vpu_free_irq(struct mt_chip *chip);
int vpu_load_firmware(struct mt_chip *chip, int idx, struct mt_virm *vm);
int vpu_check_fw_version(struct mt_chip *chip, int core_idx);
int vpu_host_thread(void *arg);
int vpu_host_thread_group3(void *arg);
int vpu_sync_thread(void *arg);
int vpu_fill_drm_ioctls(struct drm_ioctl_desc *dst, int num);
int vpu_run_ioctl(int ioctl, struct drm_device *drm, void *data, struct mt_virm *vm);
int mtvpu_sleep_wake(Uint32 core_idx, int is_sleep_wake);
int mtvpu_hw_reset(Uint32 core_idx);
int mtvpu_suspend(struct device *dev);
int mtvpu_resume(struct device *dev);

#endif /* _MTVPU_DRV_H_ */
