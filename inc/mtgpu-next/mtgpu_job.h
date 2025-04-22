/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_JOB_H_
#define _MTGPU_JOB_H_

#include "mtgpu_drm.h"

/* redefine them to compatible with dkms */
#define MTFW_DEPEND_Q_SEM_NUM	(32)
#define MTFW_FINISH_Q_SEM_NUM	(8)

#define PROCESS_NAME_SIZE	(16)

struct drm_device;
struct drm_file;
struct _PVRSRV_DEVICE_NODE_;
struct _PVRSRV_RGXDEV_INFO_;
struct _CONNECTION_DATA_;
struct SERVER_MMU_CONTEXT_TAG;
union _MTFW_CCB_ITEM_PARA_;
struct mtgpu_fw_info;
struct mtgpu_vm_context;
struct mtgpu_semaphore_fence;
struct _SYNC_PRIMITIVE_BLOCK_;
struct _MEM_CTX_;
union _MTFW_NODE_ITEM_PARA_;

struct common_context_data {
	u32 dm;
	u32 kick_type;
	struct _PVRSRV_DEVICE_NODE_ *dev_node;
	struct _RGX_SERVER_COMMON_CONTEXT_ *server_common_context;
	struct _SYNC_ADDR_LIST_ *update_sync_list;
	struct _SYNC_ADDR_LIST_ *check_sync_list;
	struct mutex * lock;
};

struct mtgpu_job_item_semaphore {
	struct mtgpu_syncobj *check_syncobjs[MTFW_DEPEND_Q_SEM_NUM];
	u64 check_value[MTFW_DEPEND_Q_SEM_NUM];
	u32 check_syncobj_count;
	struct mtgpu_syncobj *update_syncobjs[MTFW_FINISH_Q_SEM_NUM];
	u32 update_syncobj_count;
	/* it used on drm_sched scenario */
	u64 update_value[MTFW_FINISH_Q_SEM_NUM];
	struct mtgpu_syncobj *cpu_signal_syncobj;
};

struct mtgpu_sched_job_data {
	struct _PVRSRV_RGXDEV_INFO_ *dev_info;
	u64 submission_id;
	u32 submission_type;
	u32 submission_flags;
	bool is_legacy;
	bool need_sync;
	u32 priority;
	u32 sub_cmd;
	u32 node_type;
	char process_name[PROCESS_NAME_SIZE];
	u32 process_id;
	union _MTFW_NODE_ITEM_PARA_ *node_item_para;
	struct mtgpu_job_item *job_item;

	/* ccb */
	u32 ccb_type;
	struct mtgpu_job_list *job_list;
	union _MTFW_CCB_ITEM_PARA_ *ccb_item_para;
};

struct mtgpu_sem_info  {
	u64 handle;
	u32 update_val;
	u32 sysmem_val;
	u32 vram_val;
	u32 padding;
};

struct mtgpu_submission_info {
	u32 process_id;
	u32 padding;
	char process_name[PROCESS_NAME_SIZE];
	u64 submission_addr;
	u64 submission_size;
	u64 page_table_root_addr;
	u32 check_sem_count;
	struct mtgpu_sem_info check_sem[MTFW_DEPEND_Q_SEM_NUM];
	u32 update_sem_count;
	struct mtgpu_sem_info update_sem[MTFW_DEPEND_Q_SEM_NUM];
};

struct mtgpu_job_syncprim_info {
	struct _SYNC_PRIMITIVE_BLOCK_ *check_sync_block[MTFW_DEPEND_Q_SEM_NUM];
	u32 check_sync_offset[MTFW_DEPEND_Q_SEM_NUM];
	u64 check_value[MTFW_DEPEND_Q_SEM_NUM]; /* for drm_schedulder */
	u32 check_count;

	struct _SYNC_PRIMITIVE_BLOCK_ *update_sync_block[MTFW_FINISH_Q_SEM_NUM];
	u32 update_sync_offset[MTFW_FINISH_Q_SEM_NUM];
	u64 update_value[MTFW_FINISH_Q_SEM_NUM]; /* for drm_schedulder */
	u32 update_count;
};

struct mtgpu_job_buf_sync_info {
	struct mtgpu_syncobj *check_syncobj[MTFW_DEPEND_Q_SEM_NUM];
	u32 check_count;
	struct mtgpu_syncobj *update_syncobj;
};

struct mtgpu_job_item {
	bool is_legacy;

	u64 pool_index;
	int resp_value;
	bool destroyed;
	u8 process_name[PROCESS_NAME_SIZE];
	struct device *dev;

	u32 submission_cmd;
	u64 submission_token;
	u32 submission_flags;

	struct mtgpu_job_item_semaphore job_semaphore;

	/* hwr & hwperf */
	struct mtgpu_submission_info submission_info;

	/*only for resubmit when FW reboot*/
	u32 ccb_type;
	u32 priority;
	u64 item_fw_addr;

	/* node on job_item_list*/
	struct list_head node;
	struct mtgpu_job_list *job_list;

	bool need_sync;
	bool skip_flag;
	bool resubmit_permit_flag;

	/* for drm_scheduler */
	u32 node_type;
	bool is_submited;
	struct mtgpu_sched_job *sched_job;
	/*
	 * In mtgpu_sched_need_skip_job, need vm_ctx state to determine whether job need to be skip.
	 * Can't use job_ctx->vm_ctx, because job_ctx maybe be destroyed.
	 */
	struct mtgpu_vm_context *vm_ctx;

	/* for legacy job*/
	struct _MEM_CTX_ *mem_ctx;
	struct mtgpu_job_syncprim_info syncprim_info;
	struct mtgpu_job_buf_sync_info buf_sync_info;

	/* for get submission error */
	struct mtgpu_job_context *job_ctx;

	struct mtgpu_doorbell *doorbell;
};

struct mtgpu_job_context {
	struct _PVRSRV_DEVICE_NODE_ *dev_node;
	u32 type;
	u32 flags;
	u32 priority;
	int kref;
	struct mutex *lock;
	struct SERVER_MMU_CONTEXT_TAG *server_mmu_context;
	u64 page_table_root_addr;
	struct mtgpu_vm_context *vm_ctx;
	struct mtgpu_job_list *job_list;
	struct mtgpu_buffer_sync_context_v2 *buffer_sync_context;
	struct mtgpu_sched_entity *entity;
	atomic64_t submission_last_error;
	struct mtgpu_doorbell *doorbell;
	atomic_t is_guilty;
	u32 job_ctx_id;
	struct workqueue_struct *work_queue;
	struct work_struct *cleanup_work;
};

/* list used to record job_item that not respond by FW */
struct mtgpu_job_list {
	struct list_head list;
	struct spinlock *lock;
	struct wait_queue_head *wait_queue;
	/* used to send cancel cmd when release PVRSRV_HANDLE_TYPE_MTGPU_JOB_LIST . */
	struct _MEM_CTX_ *mem_ctx;
};

int mtgpu_job_submit_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_notify_queue_update_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_ctx_get(struct mtgpu_job_context *job_ctx);
int mtgpu_job_ctx_put(struct mtgpu_job_context *job_ctx);
int mtgpu_job_context_create_ioctl_v3(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_context_destroy_ioctl_v3(struct drm_device *drm, void *data, struct drm_file *file_priv);
void mtgpu_job_semaphore_get(struct mtgpu_job_item_semaphore *job_semaphore);
void mtgpu_job_semaphore_put(struct mtgpu_job_item_semaphore *job_semaphore);
int mtgpu_job_submit_ioctl_v3(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_append_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_get_submission_last_error_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_get_device_last_error_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_item_cleanup(struct _PVRSRV_RGXDEV_INFO_ *dev_info, struct mtgpu_job_item *job_item);
int mtgpu_job_item_add_semaphore(struct _CONNECTION_DATA_ *conn,
				 struct drm_mtgpu_semaphore *check_semaphores,
				 u32 check_semaphore_count,
				 struct drm_mtgpu_semaphore *update_semaphores,
				 u32 update_semaphore_count,
				 struct mtgpu_syncobj **buf_sync_check_syncobjs,
				 u32 buf_sync_check_syncobj_count,
				 struct mtgpu_syncobj *buf_sync_update_syncobj,
				 union _MTFW_CCB_ITEM_PARA_ *item_para,
				 struct mtgpu_job_item_semaphore *job_semaphore);
int mtgpu_job_item_semaphore_create_fence(struct _CONNECTION_DATA_ *conn,
					  struct drm_mtgpu_semaphore *check_semaphores,
					  u32 check_semaphore_count,
					  struct drm_mtgpu_semaphore *update_semaphores,
					  u32 update_semaphore_count,
					  struct mtgpu_syncobj **buf_sync_check_syncobjs,
					  u32 buf_sync_check_syncobj_count,
					  struct mtgpu_syncobj *buf_sync_update_syncobj,
					  struct mtgpu_job_item_semaphore *job_semaphore,
					  struct dma_fence **dep_fences,
					  struct dma_fence **buffer_sync_fence_out,
					  struct mtgpu_semaphore_fence **update_sem_fences_out);
int mtgpu_job_ccb_item_assign_update_semaphore(union _MTFW_CCB_ITEM_PARA_ *ccb_item_para,
					       struct mtgpu_job_item_semaphore *job_semaphore);
void mtgpu_job_item_semaphore_wake_up(struct mtgpu_job_item *job_item);
bool mtgpu_job_has_no_user_wait(struct mtgpu_job_item *job_item);
void mtgpu_job_set_semaphore_error(struct mtgpu_job_item *job_item);
void mtgpu_job_set_semaphore_fault(struct mtgpu_job_item *job_item);
bool mtgpu_job_ctx_has_no_skip_flag(struct mtgpu_job_context *job_ctx);
bool mtgpu_job_ctx_set_guilty(struct mtgpu_job_context *job_ctx, bool need_cancel);
bool mtgpu_job_ctx_is_guilty(struct mtgpu_job_context *job_ctx);
void mtgpu_job_semaphore_fence_signal(struct mtgpu_job_item *job_item);
int mtgpu_job_list_create(struct mtgpu_job_list **item_list_out);
void mtgpu_job_list_destroy(struct mtgpu_job_list *job_list);
void mtgpu_job_item_cleanup_wakeup(struct mtgpu_job_list *job_list);
void mtgpu_job_wait_list_empty(struct mtgpu_vm_context *vm_ctx);
int mtgpu_job_list_cleanup(struct mtgpu_vm_context *vm_ctx);
void mtgpu_job_list_add(struct mtgpu_job_list *job_list, struct list_head *node);
void mtgpu_job_list_delete(struct mtgpu_job_list *job_list, struct list_head *node);

int mtgpu_job_item_add_semaphore_legacy(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					int cmd_type,
					u64 *check_syncprim_addr,
					u32 check_syncprim_count,
					u64 *check_value,
					u64 *update_syncprim_addr,
					u32 update_syncprim_count,
					u64 *update_value,
					u32 check_syncobj_count,
					struct mtgpu_syncobj **check_syncobjs,
					struct mtgpu_syncobj *update_syncobj,
					union _MTFW_CCB_ITEM_PARA_ *item_para);
void mtgpu_job_save_submission_info_legacy(u64 pc_root,
					   struct _SYNC_PRIMITIVE_BLOCK_ **check_syncprim_block,
					   u32 *check_syncprim_offset,
					   u64 *check_value,
					   u32 check_syncprim_count,
					   struct _SYNC_PRIMITIVE_BLOCK_ **update_syncprim_block,
					   u32 *update_syncprim_offset,
					   u64 *update_value,
					   u32 update_syncprim_count,
					   struct mtgpu_syncobj **check_syncobjs,
					   u32 check_syncobj_count,
					   struct mtgpu_syncobj *update_syncobj,
					   u64 submission_va,
					   u32 submission_size,
					   struct mtgpu_job_item *job_item);

int mtgpu_job_submit_with_doorbell_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_acquire_doorbell_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);
int mtgpu_job_release_doorbell_ioctl(struct drm_device *drm, void *data, struct drm_file *file_priv);

#endif /* _MTGPU_JOB_H_ */
