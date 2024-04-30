/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_DRV_H_
#define _MTVPU_DRV_H_

#ifndef SOC_MODE
#include "mtgpu_defs.h"
#endif
#include "vpuapi.h"
#include "jdi.h"

#include "mtvpu_conf.h"
#include "mtvpu_mon.h"

#define INST_Q_DEPTH  COMMAND_QUEUE_DEPTH

#define WAV5_MAX_CORE  6 // for QY2

#define MAX_HOST_VPU_GROUPS_GEN1_GEN2 3	//For SUDI and QY1 in VDI case, host only has groups <=3

#define SYNC_INTR_SIZE (WAV5_MAX_CORE * INST_MAX_SIZE)
#define SYNC_ADDR_SIZE (WAV5_MAX_CORE * INST_MAX_SIZE * INST_Q_DEPTH)

#define FW_LOG_BUFFER_SIZE	(16 << 10) //Currently 16k, if not enough enlarge the size

#define MTVPU_SEGMENT_NUM       16
#define MTVPU_SEGMENT_VM       -1

#ifdef SUPPORT_ION
#define ion_phys_addr_t phys_addr_t
#endif

#ifdef SOC_MODE
#define APOLLO_VPU_MEM_BASE 0xB08000000
#define APOLLO_VPU_MEM_SIZE 0xc0000000
#define MTGPU_CORE_COUNT_MAX 32
#define gpu_page_size 0x1000
#else
extern u32 gpu_page_size;
extern u32 gpu_page_shift;
#endif

struct file_operations;
struct platform_device;
struct drm_device;
struct drm_ioctl_desc;
struct wait_queue_head;
struct spinlock;
struct semaphore;
struct mutex;
struct list_head;
struct timer_list;
struct drm_file;
struct drm_gem_object;
struct mtgpu_gem_object;
struct vm_area_struct;
struct dma_buf;
struct iommu_group;
struct iommu_domain;
struct iova_domain;

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
	u64 size;
	void *vir_addr;
	void *bak_addr;
	void *handle;
#ifdef SUPPORT_ION
	struct dma_buf *ion_buf;
#endif
	u32 pool_id;
	dma_addr_t iova_addr;
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
	struct jpu_instance_pool jpu_pool;

	int mem_group_id;
	u64 mem_group_base;
	struct drm_device *drm;
#ifdef SUPPORT_ION
	int heap_id;
	struct ion_device *ion;
#endif
	bool serve_all;
	struct wait_queue_head *core_wait;
	struct wait_queue_head *inst_wait[INST_MAX_SIZE];
	char irq_name[32];

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

struct mt_host_pool {
	u32 pool_id;
	bool used;
};

#define HOST_OSID_SIZE 64
#define HOST_POOL_SIZE 8

struct mt_host_mdev {
	u64 mem_group_base;
	int mem_group_id;

	struct mt_host_pool dec_map[HOST_POOL_SIZE];
	struct mt_host_pool enc_map[HOST_POOL_SIZE];
};

struct mt_codec_limit {
    u32 width;
    u32 height;
    u32 max_enc_num;
    u32 max_dec_num;
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

	RenderTargetInfo render;
	struct list_head list;

	struct mt_host_pool dec_map[HOST_POOL_SIZE];
	struct mt_host_pool enc_map[HOST_POOL_SIZE];
};

struct mt_chip {
	int idx;
	struct mt_conf conf;
	struct pci_dev *pdev;
	struct device *parent;
	struct device *dev;

	struct drm_device *drm_host; /* for host use */

	u32 inst_cnt;
	struct mutex *inst_cnt_lock;

#ifdef SOC_MODE
	struct drm_mm *mm;
#endif
	int mem_group_id[MTVPU_SEGMENT_NUM];
	u64 mem_group_base[MTVPU_SEGMENT_NUM];
	u64 mem_group_size[MTVPU_SEGMENT_NUM];
	int mem_group_cnt;

	struct spinlock *shared_mem_lock; /* for linux guest shared memory operations */

	struct spinlock *pool_lock;
	struct mt_pool pool_array[MEM_POOL_MAX_SIZE];
	int pool_idx;

	int mpc_drm_cnt;
#ifdef SUPPORT_ION
	struct ion_device *ions[MTGPU_CORE_COUNT_MAX];
#endif
	struct drm_device *drms[MTGPU_CORE_COUNT_MAX]; /* for native use */
	int mpc_group_ids[MTGPU_CORE_COUNT_MAX];
	struct spinlock *mpc_lock;
	int core_group_inst_cnt[MTVPU_SEGMENT_NUM];

	struct mutex *mm_lock;
	void *vaddr;
	u64 bar_base;

	struct mt_core core[CORE_MAX_SIZE];
	struct semaphore *host_thread_semas[MAX_HOST_VPU_GROUPS_GEN1_GEN2];

	struct mt_sync sync;

	struct task_struct *host_threads[MAX_HOST_VPU_GROUPS_GEN1_GEN2];
	struct task_struct *sync_thread;

	struct list_head vm_heads[MAX_HOST_VPU_GROUPS_GEN1_GEN2]; /* vm list */
	struct mt_host_mdev mdev[HOST_OSID_SIZE];
	int mdev_count;

	struct mt_virm vm;
	struct mutex *vm_locks[MAX_HOST_VPU_GROUPS_GEN1_GEN2];

	struct mt_codec_limit codec_limit;

	struct dentry *debugfs;

	struct timer_list *timer;

	struct semaphore *jpu_core_sema;

	struct iommu_group *io_group;
	struct iommu_domain *io_domain;
	struct iova_domain *iova_domain;
};

struct mt_open {
	int coreIdx;
	int instIdx;
	int isDecoder;
	VpuHandle handle;

	struct list_head list;
};

struct mtvpu_gem_priv {
	FBCInfo fbc;
	struct vm_area_struct *vma;
};

void vpu_ref_core_nolock(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_ref_core_lock(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_unref_core_nolock(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_unref_core_lock(struct mt_chip *chip, int idx, struct mt_virm *vm);

int vpu_init_irq(struct mt_chip *chip, struct platform_device *pdev);
int vpu_init_mpc(struct mt_chip *chip);
void vpu_free_irq(struct mt_chip *chip);
int vpu_load_firmware(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_report_power_state(struct mt_chip *chip, int inst_cnt_change);
int vpu_check_fw_version(struct mt_chip *chip, int idx);
int vpu_host_thread_group1(void *arg);
int vpu_host_thread_group2(void *arg);
int vpu_host_thread_group3(void *arg);
int vpu_sync_thread(void *arg);
int vpu_fill_drm_ioctls(struct drm_ioctl_desc *dst, int num);
int vpu_host_run_ioctl(int ioctl, struct drm_device *drm, void *data, struct mt_virm *vm);
int vpu_sleep_wake(Uint32 core_idx, int is_sleep_wake);
int vpu_reset_core(struct mt_chip *chip, int idx);
int vpu_hw_reset(Uint32 core_idx);
int vpu_hw_deinit(Uint32 core_idx);
int vpu_suspend(struct device *dev);
int vpu_resume(struct device *dev);

s64 vpu_get_clk(struct mt_chip *chip, int idx);
s64 vpu_get_max_clk(struct mt_chip *chip, int idx);

int jpu_request_core(struct drm_device *drm);
void jpu_release_core(struct drm_device *drm, int idx);

extern int enable_pm_mtvpu_backup_device_memory;

#endif /* _MTVPU_DRV_H_ */
