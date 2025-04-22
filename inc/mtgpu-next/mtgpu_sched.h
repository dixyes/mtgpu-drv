/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_SCHED_H_
#define _MTGPU_SCHED_H_

#include "os-interface.h"
#include "mtgpu_module_param.h"
#include "mtgpu_drm.h"

#define MAX_WAIT_SCHED_ENTITY_Q_EMPTY os_msecs_to_jiffies(1000)

/**
 * MTGPU_SCHED_FENCE_DONT_PIPELINE - Prefent dependency pipelining
 *
 * Setting this flag on a scheduler fence prevents pipelining of jobs depending
 * on this fence. In other words we always insert a full CPU round trip before
 * dependen jobs are pushed to the hw queue.
 */
#define MTGPU_SCHED_FENCE_DONT_PIPELINE	OS_VAL(DMA_FENCE_FLAG_USER_BITS)

enum dma_resv_usage;
struct dma_resv;
struct dma_fence;
struct drm_gem_object;
struct mtgpu_scheduler;
struct mtgpu_sched_rq;
struct drm_file;
struct spinlock;
struct spsc_queue;
struct rb_root_cached;
struct mtgpu_sched_job;
struct mtgpu_vm_context;
struct mtgpu_job_context;
struct mtgpu_job_item;
struct mtgpu_job_list;
struct mtgpu_semaphore_fence;
struct _PVRSRV_RGXDEV_INFO_;
union _MTFW_NODE_ITEM_PARA_;
union _MTFW_CCB_ITEM_PARA_;

/* These are often used as an (initial) index
 * to an array, and as such should start at 0.
 */
enum mtgpu_sched_priority {
	MTGPU_SCHED_PRIORITY_MIN,
	MTGPU_SCHED_PRIORITY_NORMAL,
	MTGPU_SCHED_PRIORITY_HIGH,
	MTGPU_SCHED_PRIORITY_KERNEL,

	MTGPU_SCHED_PRIORITY_COUNT
};

#define MTGPU_SCHED_POLICY_RR    0
#define MTGPU_SCHED_POLICY_FIFO  1

/**
 * struct mtgpu_sched_entity - A wrapper around a job queue (typically
 * attached to the DRM file_priv).
 *
 * Entities will emit jobs in order to their corresponding hardware
 * ring, and the scheduler will alternate between entities based on
 * scheduling policy.
 */
struct mtgpu_sched_entity {
	/**
	 * @list:
	 *
	 * Used to append this struct to the list of entities in the runqueue
	 * @rq under &mtgpu_sched_rq.entities.
	 *
	 * Protected by &mtgpu_sched_rq.lock of @rq.
	 */
	struct list_head		list;

	/**
	 * @rq:
	 *
	 * Runqueue on which this entity is currently scheduled.
	 *
	 * FIXME: Locking is very unclear for this. Writers are protected by
	 * @rq_lock, but readers are generally lockless and seem to just race
	 * with not even a READ_ONCE.
	 */
	struct mtgpu_sched_rq		*rq;

	/**
	 * @sched_list:
	 *
	 * A list of schedulers (struct mtgpu_scheduler).  Jobs from this entity can
	 * be scheduled on any scheduler on this list.
	 *
	 * This can be modified by calling mtgpu_sched_entity_modify_sched().
	 * Locking is entirely up to the driver, see the above function for more
	 * details.
	 *
	 * This will be set to NULL if &num_sched_list equals 1 and @rq has been
	 * set already.
	 *
	 * FIXME: This means priority changes through
	 * mtgpu_sched_entity_set_priority() will be lost henceforth in this case.
	 */
	struct mtgpu_scheduler        **sched_list;

	/**
	 * @num_sched_list:
	 *
	 * Number of mtgpu_scheds in the @sched_list.
	 */
	unsigned int                    num_sched_list;

	/**
	 * @priority:
	 *
	 * Priority of the entity. This can be modified by calling
	 * mtgpu_sched_entity_set_priority(). Protected by &rq_lock.
	 */
	enum mtgpu_sched_priority         priority;

	/**
	 * @rq_lock:
	 *
	 * Lock to modify the runqueue to which this entity belongs.
	 */
	spinlock_t			*rq_lock;

	/**
	 * @job_queue: the list of jobs of this entity.
	 */
	struct spsc_queue		*job_queue;

	/**
	 * @fence_seq:
	 *
	 * A linearly increasing seqno incremented with each new
	 * &mtgpu_sched_fence which is part of the entity.
	 *
	 * FIXME: Callers of mtgpu_sched_job_arm() need to ensure correct locking,
	 * this doesn't need to be atomic.
	 */
	atomic_t			fence_seq;

	/**
	 * @fence_context:
	 *
	 * A unique context for all the fences which belong to this entity.  The
	 * &mtgpu_sched_fence.scheduled uses the fence_context but
	 * &mtgpu_sched_fence.finished uses fence_context + 1.
	 */
	uint64_t			fence_context;

	/**
	 * @dependency:
	 *
	 * The dependency fence of the job which is on the top of the job queue.
	 */
	struct dma_fence		*dependency;

	/**
	 * @cb:
	 *
	 * Callback for the dependency fence above.
	 */
	struct dma_fence_cb		*cb;

	/**
	 * @guilty:
	 *
	 * Points to entities' guilty.
	 */
	atomic_t			*guilty;

	/**
	 * @last_scheduled:
	 *
	 * Points to the finished fence of the last scheduled job. Only written
	 * by the scheduler thread, can be accessed locklessly from
	 * mtgpu_sched_job_arm() iff the queue is empty.
	 */
	struct dma_fence __rcu		*last_scheduled;

	/**
	 * @last_user: last group leader pushing a job into the entity.
	 */
	struct task_struct		*last_user;

	/**
	 * @stopped:
	 *
	 * Marks the enity as removed from rq and destined for
	 * termination. This is set by calling mtgpu_sched_entity_flush() and by
	 * mtgpu_sched_fini().
	 */
	bool 				stopped;

	/**
	 * @entity_idle:
	 *
	 * Signals when entity is not in use, used to sequence entity cleanup in
	 * mtgpu_sched_entity_fini().
	 */
	struct completion		*entity_idle;

	/**
	 * @oldest_job_waiting:
	 *
	 * Marks earliest job waiting in SW queue
	 */
	ktime_t				oldest_job_waiting;

	/**
	 * @rb_tree_node:
	 *
	 * The node used to insert this entity into time based priority queue
	 */
	struct rb_node			*rb_tree_node;

};

/**
 * struct mtgpu_sched_rq - queue of entities to be scheduled.
 *
 * @lock: to modify the entities list.
 * @sched: the scheduler to which this rq belongs to.
 * @entities: list of the entities to be scheduled.
 * @current_entity: the entity which is to be scheduled.
 * @rb_tree_root: root of time based priory queue of entities for FIFO scheduling
 *
 * Run queue is a set of entities scheduling command submissions for
 * one specific ring. It implements the scheduling policy that selects
 * the next entity to emit commands from.
 */
struct mtgpu_sched_rq {
	spinlock_t			*lock;
	struct mtgpu_scheduler		*sched;
	struct list_head		entities;
	struct mtgpu_sched_entity	*current_entity;
	struct rb_root_cached		*rb_tree_root;
};

/**
 * struct mtgpu_sched_fence - fences corresponding to the scheduling of a job.
 */
struct mtgpu_sched_fence {
        /**
         * @scheduled: this fence is what will be signaled by the scheduler
         * when the job is scheduled.
         */
	struct dma_fence		*scheduled;

        /**
         * @finished: this fence is what will be signaled by the scheduler
         * when the job is completed.
         *
         * When setting up an out fence for the job, you should use
         * this, since it's available immediately upon
	 * mtgpu_sched_job_init(), and the fence returned by the driver
         * from run_job() won't be created until the dependencies have
         * resolved.
         */
	struct dma_fence		*finished;

	/**
	 * @deadline: deadline set on &mtgpu_sched_fence.finished which
	 * potentially needs to be propagated to &mtgpu_sched_fence.parent
	 */
	ktime_t				deadline;

        /**
	 * @parent: the fence returned by mtgpu_sched_job_run()
	 * when scheduling the job on hardware. We signal the
	 * mtgpu_sched_fence.finished fence once parent is signalled.
         */
	struct dma_fence		*parent;
        /**
         * @sched: the scheduler instance to which the job having this struct
         * belongs to.
         */
	struct mtgpu_scheduler		*sched;
        /**
         * @lock: the lock used by the scheduled and the finished fences.
         */
	spinlock_t			*lock;
        /**
         * @owner: job owner for debugging
         */
	void				*owner;
};

struct mtgpu_sched_fence *to_mtgpu_sched_fence(struct dma_fence *f);

/**
 * struct mtgpu_sched_job - A job to be run by an entity.
 *
 * @queue_node: used to append this struct to the queue of jobs in an entity.
 * @list: a job participates in a "pending" and "done" lists.
 * @sched: the scheduler instance on which this job is scheduled.
 * @s_fence: contains the fences for the scheduling of job.
 * @finish_cb: the callback for the finished fence.
 * @work: Helper to reschdeule job kill to different context.
 * @id: a unique id assigned to each job scheduled on the scheduler.
 * @karma: increment on every hang caused by this job. If this exceeds the hang
 *         limit of the scheduler then the job is marked guilty and will not
 *         be scheduled further.
 * @s_priority: the priority of the job.
 * @entity: the entity to which this job belongs.
 * @cb: the callback for the parent fence in s_fence.
 *
 * A job is created by the driver using mtgpu_sched_job_init(), and
 * should call mtgpu_sched_entity_push_job() once it wants the scheduler
 * to schedule the job.
 */
struct mtgpu_sched_job {
	struct spsc_node		*queue_node;
	struct list_head		list;
	struct mtgpu_scheduler		*sched;
	struct mtgpu_sched_fence	*s_fence;

	struct dma_fence_cb		*finish_cb;
	struct work_struct		*work;

	uint64_t			id;
	atomic_t			karma;
	enum mtgpu_sched_priority	s_priority;
	struct mtgpu_sched_entity	*entity;
	struct dma_fence_cb		*cb;
	/**
	 * @dependencies:
	 *
	 * Contains the dependencies as struct dma_fence for this job, see
	 * mtgpu_sched_job_add_dependency() and
	 * mtgpu_sched_job_add_implicit_dependencies().
	 */
	struct xarray			*dependencies;

	/** @last_dependency: tracks @dependencies as they signal */
	unsigned long			last_dependency;

	/**
	 * @submit_ts:
	 *
	 * When the job was pushed into the entity queue.
	 */
	ktime_t                         submit_ts;

	/* data that needs to be submitted to fw */
	struct mtgpu_sched_job_data	*job_data;

	/* fences that the job depend on, from check semaphores */
	struct dma_fence		*dep_fences[32];
	/* dependent fences total count */
	u32				dep_fence_count;
	u32				dep_fence_index;

	/* fence returned by mtgpu_sched_job_run() and signaled by irq */
	struct dma_fence		*done_fence;

	mt_kref kref;
};

enum mtgpu_sched_stat {
	MTGPU_SCHED_STAT_NONE, /* Reserve 0 */
	MTGPU_SCHED_STAT_NOMINAL,
	MTGPU_SCHED_STAT_ENODEV,
};

/**
 * struct mtgpu_scheduler - scheduler instance-specific data
 *
 * @ops: backend operations provided by the driver.
 * @hw_submission_limit: the max size of the hardware queue.
 * @timeout: the time after which a job is removed from the scheduler.
 * @name: name of the ring for which this scheduler is being used.
 * @sched_rq: priority wise array of run queues.
 * @wake_up_worker: the wait queue on which the scheduler sleeps until a job
 *                  is ready to be scheduled.
 * @job_scheduled: once @mtgpu_sched_entity_do_release is called the scheduler
 *                 waits on this wait queue until all the scheduled jobs are
 *                 finished.
 * @hw_rq_count: the number of jobs currently in the hardware queue.
 * @job_id_count: used to assign unique id to the each job.
 * @timeout_wq: workqueue used to queue @work_tdr
 * @work_tdr: schedules a delayed call to @mtgpu_sched_job_timedout after the
 *            timeout interval is over.
 * @thread: the kthread on which the scheduler which run.
 * @pending_list: the list of jobs which are currently in the job queue.
 * @job_list_lock: lock to protect the pending_list.
 * @hang_limit: once the hangs by a job crosses this limit then it is marked
 *              guilty and it will no longer be considered for scheduling.
 * @score: score to help loadbalancer pick a idle sched
 * @_score: score used when the driver doesn't provide one
 * @ready: marks if the underlying HW is ready to work
 * @free_guilty: A hit to time out handler to free the guilty job.
 * @dev: system &struct device
 *
 * One scheduler is implemented for each hardware ring.
 */
struct mtgpu_scheduler {
	uint32_t			hw_submission_limit;
	long				timeout;
	const char			*name;
	struct mtgpu_sched_rq		sched_rq[MTGPU_SCHED_PRIORITY_COUNT];
	wait_queue_head_t		*wake_up_worker;
	wait_queue_head_t		*job_scheduled;
	atomic_t			hw_rq_count;
	atomic64_t			job_id_count;
	struct workqueue_struct		*timeout_wq;
	struct delayed_work		*work_tdr;
	struct task_struct		*thread;
	struct list_head		pending_list;
	spinlock_t			*job_list_lock;
	int				hang_limit;
	atomic_t                        *score;
	atomic_t                        _score;
	bool				ready;
	bool				free_guilty;
	struct device			*dev;
};

int mtgpu_sched_job_get(struct mtgpu_sched_job *job);
int mtgpu_sched_job_put(struct mtgpu_sched_job *job);

int mtgpu_sched_init(struct mtgpu_scheduler *sched,
		     u32 hw_submission, u32 hang_limit,
		     long timeout, struct workqueue_struct *timeout_wq,
		     atomic_t *score, const char *name, struct device *dev);

void mtgpu_sched_fini(struct mtgpu_scheduler *sched);
int mtgpu_sched_job_init(struct mtgpu_sched_job *job,
			 struct mtgpu_sched_entity *entity,
			 void *owner);
int mtgpu_sched_job_arm(struct mtgpu_sched_job *job);
int mtgpu_sched_job_add_dependency(struct mtgpu_sched_job *job,
				   struct dma_fence *fence);
int mtgpu_sched_job_add_syncobj_dependency(struct mtgpu_sched_job *job,
					   struct drm_file *file,
					   u32 handle,
					   u32 point);
int mtgpu_sched_job_add_resv_dependencies(struct mtgpu_sched_job *job,
					  struct dma_resv *resv,
					  u32 usage);
int mtgpu_sched_job_add_implicit_dependencies(struct mtgpu_sched_job *job,
					      struct drm_gem_object *obj,
					      bool write);

void mtgpu_sched_entity_modify_sched(struct mtgpu_sched_entity *entity,
				     struct mtgpu_scheduler **sched_list,
				     unsigned int num_sched_list);

void mtgpu_sched_job_cleanup(struct mtgpu_sched_job *job);
void mtgpu_sched_wakeup_if_can_queue(struct mtgpu_scheduler *sched);
void mtgpu_sched_stop(struct mtgpu_scheduler *sched, struct mtgpu_sched_job *bad);
void mtgpu_sched_start(struct mtgpu_scheduler *sched, bool full_recovery);
void mtgpu_sched_resubmit_jobs(struct mtgpu_scheduler *sched, bool skip_all);
void mtgpu_sched_increase_karma(struct mtgpu_sched_job *bad);
void mtgpu_sched_reset_karma(struct mtgpu_sched_job *bad);
void mtgpu_sched_increase_karma_ext(struct mtgpu_sched_job *bad, int type);
bool mtgpu_sched_dependency_optimized(struct dma_fence *fence,
				      struct mtgpu_sched_entity *entity);
void mtgpu_sched_fault(struct mtgpu_scheduler *sched);

void mtgpu_sched_job_set_guilty(struct mtgpu_sched_job *bad);
void mtgpu_sched_rq_add_entity(struct mtgpu_sched_rq *rq,
			       struct mtgpu_sched_entity *entity);
void mtgpu_sched_rq_remove_entity(struct mtgpu_sched_rq *rq,
				  struct mtgpu_sched_entity *entity);

void mtgpu_sched_rq_update_fifo(struct mtgpu_sched_entity *entity, ktime_t ts);

int mtgpu_sched_entity_init(struct mtgpu_sched_entity *entity,
			    enum mtgpu_sched_priority priority,
			    struct mtgpu_scheduler **sched_list,
			    unsigned int num_sched_list,
			    atomic_t *guilty);
long mtgpu_sched_entity_flush(struct mtgpu_sched_entity *entity, long timeout);
void mtgpu_sched_entity_fini(struct mtgpu_sched_entity *entity);
void mtgpu_sched_entity_select_rq(struct mtgpu_sched_entity *entity);
struct mtgpu_sched_job *mtgpu_sched_entity_pop_job(struct mtgpu_sched_entity *entity);
void mtgpu_sched_entity_push_job(struct mtgpu_sched_job *sched_job);
void mtgpu_sched_entity_set_priority(struct mtgpu_sched_entity *entity,
				     enum mtgpu_sched_priority priority);
bool mtgpu_sched_entity_is_ready(struct mtgpu_sched_entity *entity);
int mtgpu_sched_entity_error(struct mtgpu_sched_entity *entity);

struct mtgpu_sched_fence *mtgpu_sched_fence_alloc(struct mtgpu_sched_entity *s_entity,
						  void *owner);
int mtgpu_sched_fence_init(struct mtgpu_sched_fence *fence,
			   struct mtgpu_sched_entity *entity);
void mtgpu_sched_fence_free(struct mtgpu_sched_fence *fence);

void mtgpu_sched_fence_scheduled(struct mtgpu_sched_fence *fence,
				 struct dma_fence *parent);
void mtgpu_sched_fence_finished(struct mtgpu_sched_fence *fence, int result);

unsigned long mtgpu_sched_suspend_timeout(struct mtgpu_scheduler *sched);
void mtgpu_sched_resume_timeout(struct mtgpu_scheduler *sched,
				unsigned long remaining);
struct mtgpu_scheduler *
mtgpu_sched_pick_best(struct mtgpu_scheduler **sched_list,
		      unsigned int num_sched_list);
int mtgpu_sched_fence_slab_init(void);
void mtgpu_sched_fence_slab_fini(void);

static inline bool mtgpu_sched_meta_only(void)
{
	return mtgpu_sched_mode == MTGPU_SCHED_MODE_META_ONLY;
}

static inline bool mtgpu_sched_on_nodeq(void)
{
	return mtgpu_sched_mode == MTGPU_SCHED_MODE_DRM_NODEQ;
}

static inline bool mtgpu_sched_on_ccbq(void)
{
	return mtgpu_sched_mode == MTGPU_SCHED_MODE_DRM_CCBQ;
}

static inline bool mtgpu_sched_on_host(void)
{
	return mtgpu_sched_on_nodeq() || mtgpu_sched_on_ccbq();
}

struct mtgpu_sched_job_data *mtgpu_sched_get_job_data(struct mtgpu_sched_job *job);
void mtgpu_sched_job_response_done(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
				   struct mtgpu_job_item *job_item);
void mtgpu_sched_process_error_job(struct mtgpu_sched_job *job);
int mtgpu_scheduler_create(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_scheduler_destroy(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
int mtgpu_sched_entity_create(struct device *dev,
			      struct _PVRSRV_RGXDEV_INFO_ *dev_info,
			      u32 type,
			      struct mtgpu_sched_entity **entity_out);
void mtgpu_sched_entity_destroy(struct mtgpu_sched_entity *entity);
int mtgpu_sched_job_create_and_push(struct device *dev,
				    struct mtgpu_sched_job_data *job_data,
				    struct mtgpu_sched_entity *entity,
				    struct dma_fence **dep_fences,
				    u32 dep_count,
				    struct mtgpu_semaphore_fence **update_sem_fences,
				    u32 update_count,
				    struct dma_fence *buffer_sync_update_fence);
void mtgpu_sched_done_fence_signal(struct mtgpu_sched_job *job);
int mtgpu_sched_hardware_recovery(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
				  bool skip_all);
int mtgpu_sched_add_node_gp_cmd(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
				u32 sub_cmd,
				union _MTFW_NODE_ITEM_PARA_ *item_para);
int mtgpu_sched_add_ccbq_gp_cmd(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
				u32 sub_cmd,
				union _MTFW_CCB_ITEM_PARA_ *item_para,
				void *vm_ctx,
				struct mtgpu_job_context *job_ctx,
				struct mtgpu_job_list *job_list,
				bool need_sync,
				bool is_legacy);

struct dma_fence *mtgpu_sched_job_run(struct mtgpu_sched_job *job);
enum mtgpu_sched_stat mtgpu_sched_job_handle_timedout(struct mtgpu_sched_job *job);
void mtgpu_sched_job_free(struct mtgpu_sched_job *job);
struct dma_fence *mtgpu_sched_job_prepare_dependency(struct mtgpu_sched_job *job,
						     struct mtgpu_sched_entity *s_entity);
void mtgpu_sched_suspend_block(void);
void mtgpu_sched_suspend_unblock(void);

#endif
