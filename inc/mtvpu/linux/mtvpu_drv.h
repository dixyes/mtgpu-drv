/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_DRV_H_
#define _MTVPU_DRV_H_

#include "mtgpu_defs.h"
#include "vpuapi.h"
#include "jdi.h"
#include "osfunc.h"
#include "mtvpu_conf.h"
#include "mtvpu_mon.h"

#define INST_Q_DEPTH  COMMAND_QUEUE_DEPTH

#define WAV5_MAX_CORE  6 // for QY2

#define MAX_HOST_VPU_GROUPS_GEN1_GEN2 3	//For SUDI and QY1 in VDI case, host only has groups <=3

#define MTVPU_SEGMENT_NUM       16
#define MTVPU_SEGMENT_VM       -1

#define VPU_SMMU_MEM_BASE_MASK 0xFF00000000

/* modify duration with caution! */
#define VPU_UTIL_DURATION 300 /* ms */
#define VPU_STAT_DURATION 1000 /* ms */

/* return 1GHz(meaningless) to prevent a division zero exception when get clock failed */
#define MTVPU_GET_CLK_FAIL_FREQ  (1*1000*1000*1000)

#ifdef SUPPORT_ION
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
struct devfreq;
struct devfreq_simple_ondemand_data;
struct devfreq_dev_profile;

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
	u32 err_reason;
	u32 tick_frame;
	union {
		struct {  // 517 result
			u32 linear_y_addr;
			u32 linear_y_addr_ext;
		};
		struct {  // 627 result
			u32 stream_size;
		};
	};
};

struct mt_vpu_irq_ctx {
	struct mt_chip *chip;
	struct spinlock *irq_lock;
	/* now the FW queue depth is 4, so we can use work_idx&511 to get
	   the ring worker buffer idx.*/
	void *works;
	int work_idx;
	int work_cnt;
};

struct mt_node {
	struct drm_gem_object *obj;
	u64 dev_phys_addr;
	u64 size;
	void *handle;
#ifdef SUPPORT_ION
	struct dma_buf *ion_buf;
#endif
	u32 pool_id;
	u64 private_data;
};

struct umd_alloc_buffer_single {
	u64 dev_addr;
	u64 virt_addr; /* for smmu use */
	u64 size;
	bool used;
};

struct umd_alloc_buffer_union{
	u64 start_pa;
	u64 virt_addr; /* for smmu use */
	u32 total_size;
	u32 used_size;
};

/* the instance mutex can guaranty safe. so we don't use lock. */
struct mt_vpu_cmd {
	u64 sur_addr;
	u64 sem;

	/* Todo remove it later*/
	bool addr_blocked;
	struct wait_queue_head *addr_wait;
	int status;
};

struct mt_vpu_cmd_que {
	struct mt_vpu_cmd cmd[INST_Q_DEPTH];
	int rd_idx;
	int wr_idx;
	int count;
	u64 pid;
	bool inst_blocked;
	struct wait_queue_head *inst_wait;
	VpuHandle handle;
	struct mt_virm *vm;
	struct spinlock *lock;
};

struct mt_core {
	int idx;
	int irq;
	int available;
	int inited;
	int reload_flag;  // bit0 set means hang need reload, bit1 set: means reset ok, can reload fw
	int product_id;

	void *regs;
	const struct firmware *fw;

	struct mt_node *common_node; /* common mem */
	struct vpu_instance_pool pool; /* vdi mem */
	struct jpu_instance_pool jpu_pool;

	struct device *pm_dev;
	bool suspend;

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
	int inst_bs_size_ret[INST_MAX_SIZE];    /* > 0: bs size; < 0: enc pic failed; == 0: needs query get result */
	Uint32 fw_version;

	struct mutex *open_lock;
	struct mutex *regs_lock;

	int drm_ids[INST_MAX_SIZE];
	u32 pool_ids[INST_MAX_SIZE];
	struct mtvpu_mmu_ctx *mmu_ctx[INST_MAX_SIZE];

	struct mt_chip *priv;
	void *bak_addr;

	u64 core_freq;      /* for soc_mode use */
	u64 core_max_freq;  /* for none soc_mode use */
	/* The number of ticks used by the enc pic on the VPU within one second */
	/* In MPC mode, one core may serve multiple drm nodes; */
	/* Thus we should calculate cycles for each drm node */
	u64 core_drm_cycle[MTGPU_CORE_COUNT_MAX];

	u32 inst_cnt;
	struct mt_inst_info inst_info[INST_MAX_SIZE];
	struct mt_inst_extra inst_extra[INST_MAX_SIZE];

	struct mutex *inst_lock[INST_MAX_SIZE];
	u32 log_read_pos;
	u32 dump_flag;
	u32 reset_cnt;
	void *fp_log;
	u64 log_offset;
	u32 queue_len;

	struct workqueue_struct *irq_workqueue;
	int (*pfn_irq)(struct mt_chip *chip, struct mt_core *core);
	struct mt_vpu_irq_ctx *work_ctx;
	struct mt_vpu_cmd_que que[INST_MAX_SIZE];
	struct spinlock *que_lock[INST_MAX_SIZE];
	u64 vpu_time_per_sec;
	int inst_failed[INST_MAX_SIZE];
};

struct mt_file {
	struct list_head head; /* opened list */
	struct mutex *file_lock;
	struct mtvpu_mmu_ctx *mmu_ctx;
	struct list_head sema_head;
};

struct mt_host_pool {
	u32 pool_id;
	bool used;
};

#define HOST_OSID_SIZE 64
#define HOST_POOL_SIZE 8

#define VDI_MAX_VM_NUM_GEN1	16
#define VDI_MAX_VM_NUM_GEN2	32
#define SAFE_INST_RESERVE	4
#define BASIC_DEC_CNT		6
#define BASIC_ENC_CNT		6

#define VCORE_INDEX 0
#define VCPU_INDEX  1

struct mt_codec_limit {
	u32 dec_max_width;
	u32 dec_max_height;
	u32 enc_max_width;
	u32 enc_max_height;
	u32 max_enc_num;
	u32 max_dec_num;
};

struct mt_virm {
	int vm_id;
	u32 sched_step;	/* represents the scheduled codec capacity for this VM */
	u32 sched_max;	/* represents the total available schedule codec capacity for this VM */

	u32 dec_index;
	u32 enc_index;

	struct vm_shared_mem *mem;
	bool is_share_mem_vram;
	u32 share_mem_size;

	struct mt_file file;

	RenderTargetInfo render;
	struct list_head list;

	u32 vm_group;

	u64 vcore_base;
	u64 vcpu_base;
	u32 vcpu_multiplier;

	u32 used_dec_cnt;
	u32 used_enc_cnt;
	u32 used_vcpu_buffer;

	uintptr_t dec_close_record;
	uintptr_t enc_close_record;
};

struct mt_chip {
	int idx;
	struct mt_conf conf;
	struct pci_dev *pdev;
	struct device *parent;
	struct device *dev;

	struct drm_device *drm_host; /* for host use */

	int driver_mode;

	u32 inst_cnt;
	struct mutex *inst_cnt_lock;

	bool soc_mode;

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

	void *vaddr;
	u64 bar_base;

	struct mt_core core[CORE_MAX_SIZE];
	int start_core_idx; /* for debug purpose */
	struct semaphore *host_thread_semas[MAX_HOST_VPU_GROUPS_GEN1_GEN2];
	struct task_struct *host_threads[MAX_HOST_VPU_GROUPS_GEN1_GEN2];

	struct list_head vm_heads[MAX_HOST_VPU_GROUPS_GEN1_GEN2]; /* vm list */
	int vm_count;

	bool host_vcpu_inited;
	u32 limit_dec_cnt;
	u32 limit_enc_cnt;
	u32 shared_dec_cnt;
	u32 shared_enc_cnt;
	u64 limit_vcpu_size;
	u64 shared_vcpu_size;

	struct mt_virm vm;
	struct mutex *vm_locks[MAX_HOST_VPU_GROUPS_GEN1_GEN2];

	struct mt_codec_limit codec_limit;

	struct dentry *debugfs;

	struct timer_list *util_timer; /* for utilization calculation */
	struct timer_list *stat_timer; /* for frame_rate/bitrate statistics */

	struct semaphore *jpu_core_sema;

	struct mtvpu_mmu_ctx *mmu_ctx;	/* mmu context for PH codec cores, all cores share the same context */

	struct iommu_group *io_group;
	struct iommu_domain *io_domain;
	struct iova_domain *iova_domain[CORE_MAX_SIZE];

	/* devfs for soc */
	struct devfreq *dev_freq;
	struct devfreq_simple_ondemand_data *devfreq_data;
	u64 curr_freq;
	u32 freq_min;		/* MHz */
	u32 freq_max;		/* MHz */
	void __iomem *freq_share_mem; /* Write frequency into shared memory,
				 * actual configuration will be completed
				 * by scp-firmware.
				 */
	atomic_t perf_mode;	   /* perf mode indicator */

	u32 fixed_decode_core; /* for soc transcode use */
	int auto_reset_en;     /* Internal testing scenarios do not allow automatic reset, which may result in missing critical issues */
	struct mutex *reset_lock;
};

struct mt_open {
	int coreIdx;
	int instIdx;
	int isDecoder;
	VpuHandle handle;

	struct list_head list;
};

struct mtvpu_gem_priv {
	struct vm_area_struct *vma;
	struct mtvpu_mmu_ctx *mmu_ctx;
	u64 dev_phy_addr;
	void *pmr;
	u64 priv_data; /* for vmap use in smmu */
	void *virt_addr;
};

struct mtvpu_sema_list
{
	struct list_head list;
	struct semaphore *sema;
};

int get_mtvpu_log_level(void);
void set_mtvpu_log_level(int log_level);

void vpu_ref_core_nolock(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_ref_core_lock(struct mt_chip *chip, int idx, struct mt_virm *vm, bool is_enc);
void vpu_unref_core_nolock(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_unref_core_lock(struct mt_chip *chip, int idx, struct mt_virm *vm, bool is_enc);

int vpu_init_irq(struct mt_chip *chip, struct platform_device *pdev);
int vpu_init_mpc(struct mt_chip *chip);
void vpu_free_irq(struct mt_chip *chip);
int vpu_load_firmware(struct mt_chip *chip, int idx, struct mt_virm *vm);
void vpu_report_power_state(struct mt_chip *chip, int inst_cnt_change);
int vpu_check_fw_version(struct mt_chip *chip, int idx);
int vpu_host_thread1(void *arg);
int vpu_host_thread2(void *arg);
int vpu_host_thread3(void *arg);
void vpu_irq_work(struct work_struct *work);
int vpu_reset_thread(void *arg);
int vpu_fill_drm_ioctls(struct drm_ioctl_desc *dst, int num);
int vpu_host_run_ioctl(int ioctl, struct drm_device *drm, void *data, struct mt_virm *vm);
int vpu_sleep_wake(Uint32 core_idx, int is_sleep_wake);
int vpu_reset_core(struct mt_chip *chip, int idx);
int vpu_hw_reset(Uint32 core_idx);
int vpu_hw_deinit(Uint32 core_idx);
int vpu_suspend(struct device *dev);
int vpu_resume(struct device *dev);
int vpu_dvfs_init(struct mt_chip *chip);
int vpu_dvfs_deinit(struct mt_chip *chip);
int vpu_rpm_init(struct device *dev);
void vpu_rpm_exit(struct device *dev);
int vpu_rpm_core_suspend(struct device *dev);
int vpu_rpm_core_resume(struct device *dev);

s64 vpu_get_clk(struct mt_chip *chip, int idx);
s64 vpu_get_max_clk(struct mt_chip *chip, int idx);

int jpu_request_core(struct drm_device *drm);
void jpu_release_core(struct drm_device *drm, int idx);

extern int enable_pm_mtvpu_backup_device_memory;
extern int is_vcore_mem(u32 chip_gen, u32 mem_type);

#endif /* _MTVPU_DRV_H_ */
