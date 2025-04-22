/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_HWPERF_H_
#define _MTGPU_HWPERF_H_

enum drm_mtgpu_job_submission_type;
enum mtgpu_hwperf_host_irq_type;

struct drm_device;
struct drm_file;
struct mtgpu_sem_info;
struct _HWPERF_CONNECTION_DATA_;
struct _PVRSRV_DEVICE_NODE_;
struct _PVRSRV_RGXDEV_INFO_;


/* MT HW Performance Data Transport Rountines */
int mtgpu_hwperf_init_on_demand_resources(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_hwperf_deinit_fw_polling_thread(struct _PVRSRV_DEVICE_NODE_ *dev_node);

/* MT HW Performance Profiling API(s) */
int mtgpu_hwperf_control(struct _HWPERF_CONNECTION_DATA_ *hwperf_connection,
			 struct _PVRSRV_DEVICE_NODE_ *dev_node,
			 RGX_HWPERF_STREAM_ID stream_id,
			 bool toggle, u64 mask);

PVRSRV_ERROR mtgpu_hwperf_data_store(struct _PVRSRV_DEVICE_NODE_ *dev_node);

/* MT HW Performace Host Stream APIs */
void mtgpu_hwperf_host_post_submit_start_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					       enum drm_mtgpu_job_submission_type submission_type,
					       u64 submission_id,
					       u32 pid, u32 flag,
					       u32 check_semaphore_count,
					       struct drm_mtgpu_semaphore *check_semaphores,
					       u32 update_semaphore_count,
					       struct drm_mtgpu_semaphore *update_semaphores);

void mtgpu_hwperf_host_post_submit_end_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					     enum drm_mtgpu_job_submission_type submission_type,
					     u64 submission_id,
					     u64 submission_token,
					     u32 pid, s32 result);

void mtgpu_hwperf_host_post_semaphore_wait_begin_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
						       u64 context,
						       u32 pid,
						       u32 sem_count,
						       struct drm_mtgpu_semaphore *semaphores);

void mtgpu_hwperf_host_post_semaphore_wait_succeed_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
							 u64 context,
							 u32 pid,
							 u32 sem_index,
							 u64 sem_handle);

void mtgpu_hwperf_host_post_semaphore_wait_fail_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
						      u64 context,
						      u32 pid,
						      s32 err);

void mtgpu_hwperf_host_post_semaphore_update_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
						   u64 submission_token,
						   u64 irq_ordinal,
						   u32 pid,
						   u32 semaphore_count,
						   struct mtgpu_sem_info *semaphores);
void mtgpu_hwperf_host_post_dma_enq_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					  u64 work_context,
					  u32 dev_id,
					  u32 pid,
					  u64 submission_id,
					  u32 cmd_count,
					  u32 semaphore_count,
					  struct drm_mtgpu_semaphore *semaphores);

void mtgpu_hwperf_host_post_dma_start_p2p_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
						u64 work_context,
						u32 dev_id, u32 peer_dev_id,
						u32 pid,
						u64 submission_id,
						u64 local_dev_addr, u64 peer_dev_addr);

void mtgpu_hwperf_host_post_dma_start_user_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
						 u64 work_context,
						 u32 dev_id,
						 u32 pid,
						 u64 submission_id,
						 u64 src_addr, u64 dst_addr);

void mtgpu_hwperf_host_post_dma_end_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					  u64 work_context,
					  u32 dev_id,
					  u32 pid,
					  u64 submission_id,
					  u32 cmd_index,
					  u8 xfer_dir,
					  u64 xfer_size,
					  u32 xfer_result);

void mtgpu_hwperf_host_post_notify_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					 enum drm_mtgpu_job_submission_type submission_type,
					 u64 submission_token,
					 u32 pid);

void mtgpu_hwperf_host_post_irq_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
				      u64 irq_ordinal,
				      enum mtgpu_hwperf_host_irq_type irq_type);

void mtgpu_hwperf_host_post_drm_add_job_event(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
					      u64 submission_id,
					      u32 pid,
					      u32 priority,
					      u32 submission_type);

#if defined(NO_HARDWARE)

#define MTGPU_HWPERF_SUBMIT_START(D, T, I, P, F, CC, CS, UC, US)
#define MTGPU_HWPERF_SUBMIT_END(D, T, I, TK, P, R)

#else

/**
 * This macro checks if HWPerfHost and the submit start event are enabled and if they are
 * it posts event to the HWPerfHost stream.
 *
 * @param D      Device info
 * @param T      Submission type
 * @param I      Submission ID
 * @param P      Pid of process
 * @param F      Submission Flag
 * @param CC     Check semaphore count
 * @param CS     Check semaphore array
 * @param UC     Update semaphore count
 * @param US     Update semaphore array
 */
#define MTGPU_HWPERF_SUBMIT_START(D, T, I, P, F, CC, CS, UC, US) \
	do { \
		mtgpu_hwperf_host_post_submit_start_event((D), (T), (I), (P), (F),\
                                                          (CC), (CS), (UC), (US)); \
	} while (0)

/**
 * This macro checks if HWPerfHost and the submit end event are enabled and if they are
 * it posts event to the HWPerfHost stream.
 *
 * @param D      Device info
 * @param T      Submission type
 * @param I      Submission ID
 * @param TK     Submission Token
 * @param P      Pid of process
 * @param R      Result
 */
#define MTGPU_HWPERF_SUBMIT_END(D, T, I, TK, P, R) \
	do { \
		mtgpu_hwperf_host_post_submit_end_event((D), (T), (I), (TK), (P), (R)); \
	} while (0)

#endif // NO_HARDWARE

#endif /* _MTGPU_HWPERF_H_ */
