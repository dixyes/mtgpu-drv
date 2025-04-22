/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_FW_H_
#define _MTGPU_FW_H_

#include "rgxccb.h"
#include "rgxjob.h"
#include "mtfw_fwif.h"
#include "mtfw_fwif_fw_dump.h"
#include "mtgpu_job.h"
#include "mtfw_fwif_version.h"
#include "mtgpu_mdev.h"

#define MTGPU_WAIT_SEM_CHECKED_TIMEOUT_MS		(2000)
#define MTGPU_CCB_POOL_INDEX(ccb_type, index)		((ccb_type) * MTFW_CCB_QUEUE_LENGTH + (index))
#define MTGPU_CCB_LOCK_TYPE_MAX				(MTFW_NODE_TYPE_MAX_NUM)
#define MTGPU_CCB_POOL_TOTAL_LENGTH			(MTFW_CCB_QUEUE_LENGTH * MTFW_CCB_TYPE_MAX_NUM)
#define MTGPU_FW_SHARED_ALIGN_BYTES			(8)

#define MTGPU_RISCV_FW_IS_ENABLED(mtdev)		(DEVICE_IS_PINGHU1S_OR_LATER(mtdev) && mtgpu_riscv_fw_enabled)

#define FIRMWARE_LOAD_PATH(binary_name) "mthreads/" binary_name

struct _RGX_SRVINIT_APPHINTS_;
struct mtgpu_vm_context;
extern MTFW_FWIF_VERSION mtgpu_fw_version_list[MTFW_FWIF_VERSION_ITEM_MAX_NUM];

struct mtgpu_submission_dump_info {
	u32 rd_idx;
	u32 wr_idx;
	struct mtgpu_submission_info submission_info[MTFW_FWIF_FAULT_LOG_TRACE_NUM];
};

struct mtgpu_fw_memory_info {
	u32 code_and_data_size;
	u32 shared_data_size;
	u32 init_mem_total_size;
	u32 ccb_queue_offset;
	u32 node_queue_offset;
	u32 trace_buf_offset;
	u32 perf_event_buf_offset;
	u32 fault_info_buf_offset;
	u32 dump_cfg_array_offset;
	u32 reg_dump_cfg_array_offset;
	u32 reg_dump_buff_offset;
	u32 gpu_util_buf_offset;
	u32 watchdog_buf_offset;
	u32 item_pool_offset;
	u32 vgpu_ext_osid_info_offset;
	u32 vgpu_shared_info_offset;
	u32 reg_dump_buffer_size;
	u32 reg_dump_cfg_array_size;

};

struct mtgpu_fw_info {
	PVRSRV_RGXDEV_INFO *dev_info;
	bool fw_initialised;
	bool meta_t1_enable;

	MTFW_FWIF *fwif;
	MTFW_CCB_QUEUE *ccb_queue_array;
	MTFW_FWIF_TRACEBUF *trace_buf;
	MTFW_FWIF_PERF_EVENT_BUF_CTL *perf_event_buf_ctl;
	MTFW_FWIF_FAULT_INFO_CTRL *fw_fault_info_ctrl;
	MTFW_FWIF_GPU_UTIL_BUF *gpu_util_buf;
	MTFW_FWIF_WATCHDOG *watchdog_buf;
	MTFW_CCB_ITEM *ccb_item_pool;

	MTFW_FWIF_DUMP_CFG *fw_dump_cfg;
	MTFW_FWIF_REG_DUMP_CFG *fw_reg_dump_cfg;
	void *fw_reg_dump_buff_addr;

	u64 ccb_pool_bitmap[MTGPU_CCB_POOL_TOTAL_LENGTH / 64];
	struct mutex *resp_ccb_lock[MTFW_CCB_TYPE_MAX_NUM];
	struct spinlock *ccb_pool_lock[MTGPU_CCB_LOCK_TYPE_MAX];
	struct mutex *ccb_queue_lock[MTGPU_CCB_LOCK_TYPE_MAX];
	struct mtgpu_job_item job_item[MTGPU_CCB_POOL_TOTAL_LENGTH];
	struct kmem_cache *ccb_item_para_cache;
	struct kmem_cache *ccb_item_cache;

	DEVMEM_MEMDESC *init_mem_desc;
	void *init_mem_cpu_va;
	u32 init_mem_meta_addr;
	u64 init_mem_dev_pa;
	struct mtgpu_fw_memory_info fw_mem_info;

	u32 meta_log_group;
	u32 fec_log_group;

	bool disable_hwr;

	struct work_struct *mkdir_work;

	u32 gpu_util_curr_config;
	struct workqueue_struct *gpu_util_workq;
	struct delayed_work *gpu_util_dwork;
	struct mutex *gpu_util_lock;

	struct mutex *fw_reboot_lock;
	bool fw_reboot_result;

	/* Used only for VGPU */
	MTFW_FWIF *fw_heap_vaddr[MTGPU_PER_MPC_USED_MAX_OSID];
	u64 fw_heap_dpa_base[MTGPU_PER_MPC_USED_MAX_OSID];
	MTFW_VGPU_SHARED_INFO *shared_info;
	bool force_hwr;
	bool hot_upgrading;
};

struct mtgpu_fw_skip_data {
	PVRSRV_CLEANUP_THREAD_WORK cleanup_fn;
	PVRSRV_RGXDEV_INFO *dev_info;
	MTFW_SUBMISSION_CMD_TYPE submission_cmd;
	struct mtgpu_job_item *job_item;
};


struct mtgpu_fw_ops {
	void (*get_binary_name) (struct mtgpu_device *mtdev, u8 *binary_name);
	int (*load_data) (struct mtgpu_fw_info *fw_info, const u8 *fw_data);
	int (*startup) (struct mtgpu_fw_info *fw_info);
	bool (*loaded_poll) (struct mtgpu_fw_info *fw_info);
	int (*reload_data) (struct mtgpu_fw_info *fw_info);
	int (*parse_data) (struct mtgpu_fw_info *fw_info, const struct firmware *fw);
	void (*soft_reset) (void __iomem *base);
	void (*unload) (struct mtgpu_fw_info *fw_info);
};

void mtgpu_fw_kick(PVRSRV_RGXDEV_INFO *dev_info, u32 type);
int mtgpu_fw_get_fec_va_uncached(struct device *dev, u64 dev_pa, fw_va *fec_user_va_out);
int mtgpu_fw_get_ccb_woff(MTFW_RING_CTRL *ring_ctrl, u32 *new_woff);
int mtgpu_fw_job_item_ccb_release(struct mtgpu_fw_info *fw_info,
			          MTFW_FWIF_CCB_TYPE ccb_type,
			          u32 pool_index);
int mtgpu_fw_job_item_node_release(struct mtgpu_fw_info *fw_info,
				   MTFW_FWIF_NODE_TYPE node_type,
				   u32 pool_index);
int mtgpu_fw_job_item_ccb_acquire(PVRSRV_RGXDEV_INFO *dev_info,
				  int ccb_type,
				  struct mtgpu_job_item **job_item_out);
int mtgpu_fw_job_item_node_acquire(PVRSRV_RGXDEV_INFO *dev_info,
				   int node_type,
				   struct mtgpu_job_item **job_item_out);
int mtgpu_fw_cmd_submit(PVRSRV_RGXDEV_INFO *dev_info,
			MTFW_FWIF_PRIORITY priority,
			MTFW_FWIF_CCB_TYPE ccb_type,
			MTFW_SUBMISSION_CMD_TYPE sub_cmd,
			MTFW_CCB_ITEM_PARA *ccb_item_para,
			struct mtgpu_job_list *job_list,
			struct mtgpu_job_context *job_ctx,
			u32 submission_flags,
			const char *process_name,
			u32 process_id,
			bool is_legacy,
			bool need_sync,
			u32 sync_timeout_ms,
			struct mtgpu_job_item *job_item);
int mtgpu_fw_node_cmd_submit(struct mtgpu_sched_job *job);
void mtgpu_fw_check_resp_queue(PVRSRV_RGXDEV_INFO *dev_info, u64 irq_ordinal);
void mtgpu_fw_check_node_resp_queue(PVRSRV_RGXDEV_INFO *dev_info, u64 irq_ordinal);
void mtgpu_fw_ccb_blocking_wakeup(PVRSRV_RGXDEV_INFO *dev_info);
void mtgpu_fw_sync_cmd_wakeup(PVRSRV_RGXDEV_INFO *dev_info);
void mtgpu_fw_irq_work_func(void *pvData);
int mtgpu_fw_cmd_resubmit(PVRSRV_RGXDEV_INFO *dev_info, struct mtgpu_job_item *job_item);
int mtgpu_fw_resubmit_jobs(PVRSRV_DEVICE_NODE *dev_node, struct mtgpu_vm_context *vm_ctx);
int mtgpu_fw_resubmit_memctx_jobs(PVRSRV_DEVICE_NODE *dev_node, MEM_CTX *vm_ctx);
int mtgpu_fw_resubmit_unfinished_jobs(PVRSRV_DEVICE_NODE *dev_node);
int mtgpu_fw_reboot(PVRSRV_RGXDEV_INFO *dev_info);
int mtgpu_fw_mmu_invalid_cmd_submit(PVRSRV_DEVICE_NODE *dev_node,
				    struct mtgpu_vm_context *vm_ctx,
				    struct mtgpu_syncobj **update_syncobjs,
				    u64 *update_values,
				    u32 update_semaphore_count,
				    u64 vaddr,
				    u64 size);
int mtgpu_fw_cancel_cmd_submit(struct mtgpu_vm_context *vm_ctx,
			       struct mtgpu_job_context *job_ctx,
			       MTFW_SUBMISSION_CMD_TYPE sub_cmd);
void mtgpu_fw_ccb_item_dump(MTFW_CCB_ITEM *item, fw_va item_fwaddr);

int mtgpu_fw_load(PVRSRV_DEVICE_NODE *dev_node, struct _RGX_SRVINIT_APPHINTS_ *hints);
void mtgpu_fw_unload(PVRSRV_RGXDEV_INFO *dev_info);
void mtgpu_fw_ops_init(struct mtgpu_device_node *mt_dev_node);
void mtgpu_fw_schedule_hardware_recovery(PVRSRV_RGXDEV_INFO *dev_info);
void mtgpu_all_job_item_cleanup(PVRSRV_RGXDEV_INFO *dev_info);
int mtgpu_fw_halt_cmd_submit(PVRSRV_RGXDEV_INFO *dev_info, u64 reason);
bool mtgpu_try_to_get_available_ccb_index(unsigned long *bitmap, struct spinlock *lock, u64 *pool_index);
int mtgpu_fw_global_data_init(void);
void mtgpu_fw_image_cache_free(void);
void mtgpu_fw_global_data_deinit(void);

#endif /* _MTGPU_FW_H_ */
