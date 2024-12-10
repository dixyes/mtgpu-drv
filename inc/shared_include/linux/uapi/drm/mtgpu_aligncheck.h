/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_ALIGNCHECK_H_
#define _MTGPU_ALIGNCHECK_H_

/*
 *  **********************************************************
 *  *                                                        *
 *  *                    Alignment check                     *
 *  *                                                        *
 *  **********************************************************
 */
#define MTGPU_DRM_ALIGN_CHECKS	\
	sizeof(struct drm_mtgpu_device_init), \
	offsetof(struct drm_mtgpu_device_init, ioctl_version), \
	sizeof(struct drm_mtgpu_heap_info), \
	offsetof(struct drm_mtgpu_heap_info, id), \
	offsetof(struct drm_mtgpu_heap_info, name), \
	offsetof(struct drm_mtgpu_heap_info, base), \
	offsetof(struct drm_mtgpu_heap_info, length), \
	offsetof(struct drm_mtgpu_heap_info, log2_page_size), \
	sizeof(struct mtgpu_heap_detail_in), \
	offsetof(struct mtgpu_heap_detail_in, index), \
	sizeof(struct mtgpu_heap_detail_out), \
	offsetof(struct mtgpu_heap_detail_out, info), \
	sizeof(struct mtgpu_mem_info), \
	offsetof(struct mtgpu_mem_info, system.total_size), \
	offsetof(struct mtgpu_mem_info, system.free_size), \
	offsetof(struct mtgpu_mem_info, vram.total_size), \
	offsetof(struct mtgpu_mem_info, vram.free_size), \
	sizeof(struct drm_mtgpu_device_info), \
	offsetof(struct drm_mtgpu_device_info, device_id), \
	sizeof(struct mtgpu_bo_info_in), \
	offsetof(struct mtgpu_bo_info_in, bo_handle), \
	sizeof(struct mtgpu_bo_info), \
	offsetof(struct mtgpu_bo_info, size), \
	offsetof(struct mtgpu_bo_info, align), \
	offsetof(struct mtgpu_bo_info, flags), \
	offsetof(struct mtgpu_bo_info, domain), \
	offsetof(struct mtgpu_bo_info, segment_id), \
	sizeof(struct mtgpu_bo_info_out), \
	offsetof(struct mtgpu_bo_info_out, info), \
	sizeof(struct mtgpu_dev_info), \
	offsetof(struct mtgpu_dev_info, dev_id), \
	offsetof(struct mtgpu_dev_info, dev_status), \
	offsetof(struct mtgpu_dev_info, dev_clock_speed), \
	offsetof(struct mtgpu_dev_info, mem_clock_speed), \
	offsetof(struct mtgpu_dev_info, mem_max_clock_speed), \
	offsetof(struct mtgpu_dev_info, num_cores), \
	offsetof(struct mtgpu_dev_info, uuid), \
	sizeof(struct mtgpu_pci_info), \
	offsetof(struct mtgpu_pci_info, domain_number), \
	offsetof(struct mtgpu_pci_info, bus_number), \
	offsetof(struct mtgpu_pci_info, device_number), \
	offsetof(struct mtgpu_pci_info, function_number), \
	offsetof(struct mtgpu_pci_info, current_gen_speed), \
	offsetof(struct mtgpu_pci_info, current_width), \
	offsetof(struct mtgpu_pci_info, numa_node_id), \
	offsetof(struct mtgpu_pci_info, total_pci_device_memory_accessible), \
	offsetof(struct mtgpu_pci_info, total_system_memory_accessible), \
	sizeof(struct mtgpu_platform_info), \
	offsetof(struct mtgpu_platform_info, mtlink_enable), \
	offsetof(struct mtgpu_platform_info, iommu_enable), \
	sizeof(struct drm_mtgpu_query_info), \
	offsetof(struct drm_mtgpu_query_info, in.type), \
	offsetof(struct drm_mtgpu_query_info, in.data), \
	offsetof(struct drm_mtgpu_query_info, out.data), \
	sizeof(struct drm_mtgpu_bo_create), \
	offsetof(struct drm_mtgpu_bo_create, in.size), \
	offsetof(struct drm_mtgpu_bo_create, in.align), \
	offsetof(struct drm_mtgpu_bo_create, in.flags), \
	offsetof(struct drm_mtgpu_bo_create, in.domains), \
	offsetof(struct drm_mtgpu_bo_create, in.group_id), \
	offsetof(struct drm_mtgpu_bo_create, out.bo_handle), \
	sizeof(struct drm_mtgpu_bo_from_userptr), \
	offsetof(struct drm_mtgpu_bo_from_userptr, in.userptr), \
	offsetof(struct drm_mtgpu_bo_from_userptr, in.size), \
	offsetof(struct drm_mtgpu_bo_from_userptr, in.flags), \
	offsetof(struct drm_mtgpu_bo_from_userptr, out.bo_handle), \
	sizeof(struct drm_mtgpu_bo_get_mmap_offset), \
	offsetof(struct drm_mtgpu_bo_get_mmap_offset, in.bo_handle), \
	offsetof(struct drm_mtgpu_bo_get_mmap_offset, out.offset), \
	sizeof(struct drm_mtgpu_bo_global_handle_export), \
	offsetof(struct drm_mtgpu_bo_global_handle_export, in.bo_handle), \
	offsetof(struct drm_mtgpu_bo_global_handle_export, out.global_handle), \
	sizeof(struct drm_mtgpu_bo_global_handle_import), \
	offsetof(struct drm_mtgpu_bo_global_handle_import, in.global_handle), \
	offsetof(struct drm_mtgpu_bo_global_handle_import, out.size), \
	offsetof(struct drm_mtgpu_bo_global_handle_import, out.bo_handle), \
	sizeof(struct drm_mtgpu_vm_context_create), \
	offsetof(struct drm_mtgpu_vm_context_create, vm_ctx_handle), \
	sizeof(struct drm_mtgpu_vm_context_destroy), \
	offsetof(struct drm_mtgpu_vm_context_destroy, vm_ctx_handle), \
	sizeof(struct drm_mtgpu_vm_map), \
	offsetof(struct drm_mtgpu_vm_map, vm_ctx_handle), \
	offsetof(struct drm_mtgpu_vm_map, va), \
	offsetof(struct drm_mtgpu_vm_map, mapping_flags), \
	offsetof(struct drm_mtgpu_vm_map, bo_handle), \
	offsetof(struct drm_mtgpu_vm_map, size), \
	sizeof(struct drm_mtgpu_vm_unmap), \
	offsetof(struct drm_mtgpu_vm_unmap, vm_ctx_handle), \
	offsetof(struct drm_mtgpu_vm_unmap, va), \
	sizeof(struct drm_mtgpu_timeline_create), \
	offsetof(struct drm_mtgpu_timeline_create, out.timeline_handle), \
	offsetof(struct drm_mtgpu_timeline_create, out.timeline_bo_handle), \
	offsetof(struct drm_mtgpu_timeline_create, out.timeline_value_offset), \
	sizeof(struct drm_mtgpu_timeline_destroy), \
	offsetof(struct drm_mtgpu_timeline_destroy, in.timeline_handle), \
	sizeof(struct drm_mtgpu_timeline_read), \
	offsetof(struct drm_mtgpu_timeline_read, in.timeline_handle), \
	offsetof(struct drm_mtgpu_timeline_read, out.timeline_value), \
	sizeof(struct drm_mtgpu_fence), \
	offsetof(struct drm_mtgpu_fence, timeline_handle), \
	offsetof(struct drm_mtgpu_fence, seqno), \
	sizeof(struct drm_mtgpu_fence_wait), \
	offsetof(struct drm_mtgpu_fence_wait, in.fences), \
	offsetof(struct drm_mtgpu_fence_wait, in.seqno_count), \
	offsetof(struct drm_mtgpu_fence_wait, in.wait_all), \
	offsetof(struct drm_mtgpu_fence_wait, in.timeout_ns), \
	offsetof(struct drm_mtgpu_fence_wait, out.first_signaled), \
	sizeof(struct drm_mtgpu_context_create), \
	offsetof(struct drm_mtgpu_context_create, in.type), \
	offsetof(struct drm_mtgpu_context_create, in.flags), \
	offsetof(struct drm_mtgpu_context_create, in.ccbsize), \
	offsetof(struct drm_mtgpu_context_create, in.priority), \
	offsetof(struct drm_mtgpu_context_create, in.vm_ctx_handle), \
	offsetof(struct drm_mtgpu_context_create, in.data), \
	offsetof(struct drm_mtgpu_context_create, out.ctx_handle), \
	sizeof(struct drm_mtgpu_context_destroy), \
	offsetof(struct drm_mtgpu_context_destroy, type), \
	offsetof(struct drm_mtgpu_context_destroy, ctx_handle), \
	sizeof(struct drm_mtgpu_job_context_create), \
	offsetof(struct drm_mtgpu_job_context_create, in.type), \
	offsetof(struct drm_mtgpu_job_context_create, in.priority), \
	offsetof(struct drm_mtgpu_job_context_create, in.vm_ctx_handle), \
	offsetof(struct drm_mtgpu_job_context_create, out.ctx_handle), \
	sizeof(struct drm_mtgpu_job_context_destroy), \
	offsetof(struct drm_mtgpu_job_context_destroy, ctx_handle), \
	sizeof(struct drm_mtgpu_tq_context_data), \
	offsetof(struct drm_mtgpu_tq_context_data, robustness_addr), \
	sizeof(struct drm_mtgpu_render_context_data), \
	offsetof(struct drm_mtgpu_render_context_data, robustness_addr), \
	offsetof(struct drm_mtgpu_render_context_data, max_3d_deadline_ms), \
	offsetof(struct drm_mtgpu_render_context_data, max_ta_deadline_ms), \
	sizeof(struct drm_mtgpu_compute_context_data), \
	offsetof(struct drm_mtgpu_compute_context_data, robustness_addr), \
	offsetof(struct drm_mtgpu_compute_context_data, max_deadline_ms), \
	sizeof(struct drm_mtgpu_ce_context_data), \
	offsetof(struct drm_mtgpu_ce_context_data, framework_cmd), \
	offsetof(struct drm_mtgpu_ce_context_data, framework_cmd_size), \
	offsetof(struct drm_mtgpu_ce_context_data, robustness_addr), \
	sizeof(struct drm_mtgpu_dma_context_data), \
	offsetof(struct drm_mtgpu_dma_context_data, robustness_addr), \
	sizeof(struct drm_mtgpu_dma_cmd), \
	offsetof(struct drm_mtgpu_dma_cmd, src_type), \
	offsetof(struct drm_mtgpu_dma_cmd, dst_type), \
	offsetof(struct drm_mtgpu_dma_cmd, src_addr), \
	offsetof(struct drm_mtgpu_dma_cmd, dst_addr), \
	offsetof(struct drm_mtgpu_dma_cmd, src_offset), \
	offsetof(struct drm_mtgpu_dma_cmd, dst_offset), \
	offsetof(struct drm_mtgpu_dma_cmd, xfer_size), \
	sizeof(struct drm_mtgpu_codec_data), \
	offsetof(struct drm_mtgpu_codec_data, type), \
	offsetof(struct drm_mtgpu_codec_data, length), \
	offsetof(struct drm_mtgpu_codec_data, pad0), \
	offsetof(struct drm_mtgpu_codec_data, pad1), \
	offsetof(struct drm_mtgpu_codec_data, data), \
	sizeof(struct drm_mtgpu_compute_job_data), \
	offsetof(struct drm_mtgpu_compute_job_data, num_of_workgroups), \
	offsetof(struct drm_mtgpu_compute_job_data, num_of_workitems), \
	sizeof(struct drm_mtgpu_tq_job_data), \
	offsetof(struct drm_mtgpu_tq_job_data, characteristic1), \
	offsetof(struct drm_mtgpu_tq_job_data, characteristic2), \
	sizeof(struct drm_mtgpu_ce_job_data), \
	offsetof(struct drm_mtgpu_ce_job_data, characteristic1), \
	offsetof(struct drm_mtgpu_ce_job_data, characteristic2), \
	sizeof(struct drm_mtgpu_render_job_data), \
	offsetof(struct drm_mtgpu_render_job_data, frag_check_semaphores), \
	offsetof(struct drm_mtgpu_render_job_data, frag_check_semaphore_count), \
	offsetof(struct drm_mtgpu_render_job_data, frag_update_semaphores), \
	offsetof(struct drm_mtgpu_render_job_data, frag_update_semaphore_count), \
	offsetof(struct drm_mtgpu_render_job_data, frag_foreign_fence_fd), \
	offsetof(struct drm_mtgpu_render_job_data, frag_cmd_array), \
	offsetof(struct drm_mtgpu_render_job_data, frag_cmd_size), \
	offsetof(struct drm_mtgpu_render_job_data, frag_cmd_count), \
	offsetof(struct drm_mtgpu_render_job_data, frag_pr_cmd_array), \
	offsetof(struct drm_mtgpu_render_job_data, frag_pr_cmd_size), \
	offsetof(struct drm_mtgpu_render_job_data, hwrt_dataset_handle), \
	offsetof(struct drm_mtgpu_render_job_data, msaa_scratch_buffer_handle), \
	offsetof(struct drm_mtgpu_render_job_data, zs_buffer_handle), \
	offsetof(struct drm_mtgpu_render_job_data, draw_calls_number), \
	offsetof(struct drm_mtgpu_render_job_data, indices_number), \
	offsetof(struct drm_mtgpu_render_job_data, mrts_number), \
	offsetof(struct drm_mtgpu_render_job_data, render_target_size), \
	offsetof(struct drm_mtgpu_render_job_data, kick_geom), \
	offsetof(struct drm_mtgpu_render_job_data, kick_pr), \
	offsetof(struct drm_mtgpu_render_job_data, kick_frag), \
	offsetof(struct drm_mtgpu_render_job_data, abort), \
	sizeof(struct drm_mtgpu_job_submit), \
	offsetof(struct drm_mtgpu_job_submit, in.type), \
	offsetof(struct drm_mtgpu_job_submit, in.ctx_handle), \
	offsetof(struct drm_mtgpu_job_submit, in.check_semaphores), \
	offsetof(struct drm_mtgpu_job_submit, in.check_semaphore_count), \
	offsetof(struct drm_mtgpu_job_submit, in.update_semaphores), \
	offsetof(struct drm_mtgpu_job_submit, in.update_semaphore_count), \
	offsetof(struct drm_mtgpu_job_submit, in.check_fences), \
	offsetof(struct drm_mtgpu_job_submit, in.check_fence_count), \
	offsetof(struct drm_mtgpu_job_submit, in.foreign_fence_fd), \
	offsetof(struct drm_mtgpu_job_submit, in.sync_buf_fds), \
	offsetof(struct drm_mtgpu_job_submit, in.sync_buf_flags), \
	offsetof(struct drm_mtgpu_job_submit, in.sync_buf_count), \
	offsetof(struct drm_mtgpu_job_submit, in.update_fence), \
	offsetof(struct drm_mtgpu_job_submit, in.update_fence_name), \
	offsetof(struct drm_mtgpu_job_submit, in.dm_cmd_array), \
	offsetof(struct drm_mtgpu_job_submit, in.dm_cmd_size), \
	offsetof(struct drm_mtgpu_job_submit, in.dm_cmd_count), \
	offsetof(struct drm_mtgpu_job_submit, in.deadline_us), \
	offsetof(struct drm_mtgpu_job_submit, in.ext_job_ref), \
	offsetof(struct drm_mtgpu_job_submit, in.pdump_flags), \
	offsetof(struct drm_mtgpu_job_submit, in.data), \
	sizeof(struct drm_mtgpu_dma_transfer), \
	offsetof(struct drm_mtgpu_dma_transfer, in.bo_handle), \
	offsetof(struct drm_mtgpu_dma_transfer, in.transfer_flag), \
	offsetof(struct drm_mtgpu_dma_transfer, in.ext_handle), \
	offsetof(struct drm_mtgpu_dma_transfer, in.offset), \
	offsetof(struct drm_mtgpu_dma_transfer, in.ext_offset), \
	offsetof(struct drm_mtgpu_dma_transfer, in.size), \
	offsetof(struct drm_mtgpu_dma_transfer, in.fence_handle), \
	sizeof(struct drm_mtgpu_object_destroy), \
	offsetof(struct drm_mtgpu_object_destroy, type), \
	offsetof(struct drm_mtgpu_object_destroy, handle), \
	sizeof(struct drm_mtgpu_hwrt_dataset_create_args), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, pm_data_va_array_mcg), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, tail_ptr_va_array_mcg), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, vheap_table_va), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, ppp_multi_sample_ctl), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, pm_data_va_array), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, pm_secure_data_va_array), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, tail_ptr_va_array), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, free_list_handles), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, free_lists_count), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, mcg_core_num), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, isp_merge_lower_x), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, isp_merge_lower_y), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, isp_merge_scale_x), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, isp_merge_scale_y), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, isp_merge_upper_x), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, isp_merge_upper_y), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, ppp_screen), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, rgn_stride), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, teaa), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, temtile1), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, temtile2), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, te_screen), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, tpc_size), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, tpc_stride), \
	offsetof(struct drm_mtgpu_hwrt_dataset_create_args, max_rts), \
	sizeof(struct drm_mtgpu_free_list_create_args), \
	offsetof(struct drm_mtgpu_free_list_create_args, free_list_base_dev_vaddr), \
	offsetof(struct drm_mtgpu_free_list_create_args, free_list_state_dev_vaddr), \
	offsetof(struct drm_mtgpu_free_list_create_args, mem_ctx_handle), \
	offsetof(struct drm_mtgpu_free_list_create_args, free_list_bo_handle), \
	offsetof(struct drm_mtgpu_free_list_create_args, free_list_bo_offset), \
	offsetof(struct drm_mtgpu_free_list_create_args, free_list_state_bo_handle), \
	offsetof(struct drm_mtgpu_free_list_create_args, free_list_state_bo_offset), \
	offsetof(struct drm_mtgpu_free_list_create_args, global_free_list_handle), \
	offsetof(struct drm_mtgpu_free_list_create_args, enable_check_sum), \
	offsetof(struct drm_mtgpu_free_list_create_args, grow_num_pages), \
	offsetof(struct drm_mtgpu_free_list_create_args, grow_threshold), \
	offsetof(struct drm_mtgpu_free_list_create_args, initial_num_pages), \
	offsetof(struct drm_mtgpu_free_list_create_args, max_num_pages), \
	sizeof(struct drm_mtgpu_render_resource_create_args), \
	offsetof(struct drm_mtgpu_render_resource_create_args, mcg_core_num), \
	offsetof(struct drm_mtgpu_render_resource_create_args, free_list_count), \
	offsetof(struct drm_mtgpu_render_resource_create_args, free_list_create_args), \
	sizeof(struct drm_mtgpu_object_create), \
	offsetof(struct drm_mtgpu_object_create, in.type), \
	offsetof(struct drm_mtgpu_object_create, in.data), \
	offsetof(struct drm_mtgpu_object_create, out.handles), \
	sizeof(struct drm_mtgpu_fence_to_fd), \
	offsetof(struct drm_mtgpu_fence_to_fd, in.fence), \
	offsetof(struct drm_mtgpu_fence_to_fd, out.fd), \
	sizeof(struct drm_mtgpu_semaphore), \
	offsetof(struct drm_mtgpu_semaphore, handle), \
	offsetof(struct drm_mtgpu_semaphore, value), \
	sizeof(struct drm_mtgpu_semaphore_create), \
	offsetof(struct drm_mtgpu_semaphore_create, out.handle), \
	offsetof(struct drm_mtgpu_semaphore_create, out.bo_handle), \
	offsetof(struct drm_mtgpu_semaphore_create, out.value_offset), \
	offsetof(struct drm_mtgpu_semaphore_create, out.gpu_address), \
	sizeof(struct drm_mtgpu_semaphore_destroy), \
	offsetof(struct drm_mtgpu_semaphore_destroy, handle), \
	sizeof(struct drm_mtgpu_semaphore_submit), \
	offsetof(struct drm_mtgpu_semaphore_submit, ctx_handle), \
	offsetof(struct drm_mtgpu_semaphore_submit, job_type), \
	offsetof(struct drm_mtgpu_semaphore_submit, sem_type), \
	offsetof(struct drm_mtgpu_semaphore_submit, semaphore), \
	sizeof(struct drm_mtgpu_semaphore_cpu_signal), \
	offsetof(struct drm_mtgpu_semaphore_cpu_signal, vm_ctx_handle), \
	offsetof(struct drm_mtgpu_semaphore_cpu_signal, semaphore), \
	sizeof(struct drm_mtgpu_semaphore_to_fd), \
	offsetof(struct drm_mtgpu_semaphore_to_fd, in.semaphore), \
	offsetof(struct drm_mtgpu_semaphore_to_fd, out.fd), \
	sizeof(struct drm_mtgpu_semaphore_from_fd), \
	offsetof(struct drm_mtgpu_semaphore_from_fd, in.fd), \
	offsetof(struct drm_mtgpu_semaphore_from_fd, in.vm_ctx_handle), \
	offsetof(struct drm_mtgpu_semaphore_from_fd, out.semaphore), \
	offsetof(struct drm_mtgpu_semaphore_from_fd, out.bo_handle), \
	offsetof(struct drm_mtgpu_semaphore_from_fd, out.value_offset), \
	sizeof(struct drm_mtgpu_semaphore_wait), \
	offsetof(struct drm_mtgpu_semaphore_wait, in.semaphores), \
	offsetof(struct drm_mtgpu_semaphore_wait, in.count), \
	offsetof(struct drm_mtgpu_semaphore_wait, in.timeout_ns), \
	sizeof(struct drm_mtgpu_llc_persistence_in), \
	offsetof(struct drm_mtgpu_llc_persistence_in, replace_mode), \
	offsetof(struct drm_mtgpu_llc_persistence_in, max_set_aside_size), \
	sizeof(struct drm_mtgpu_llc_persistence_out), \
	offsetof(struct drm_mtgpu_llc_persistence_out, llc_size), \
	offsetof(struct drm_mtgpu_llc_persistence_out, max_llc_persisting_size), \
	sizeof(struct drm_mtgpu_cache_op), \
	offsetof(struct drm_mtgpu_cache_op, in.type), \
	offsetof(struct drm_mtgpu_cache_op, in.data), \
	offsetof(struct drm_mtgpu_cache_op, out.data), \
	sizeof(struct drm_mtgpu_transport_layer), \
	offsetof(struct drm_mtgpu_transport_layer, in.type), \
	offsetof(struct drm_mtgpu_transport_layer, in.sd_handle), \
	offsetof(struct drm_mtgpu_transport_layer, in.data), \
	offsetof(struct drm_mtgpu_transport_layer, out.data), \
	sizeof(struct drm_mtgpu_stream_open_data_in), \
	offsetof(struct drm_mtgpu_stream_open_data_in, name), \
	offsetof(struct drm_mtgpu_stream_open_data_in, mode), \
	sizeof(struct drm_mtgpu_stream_open_data_out), \
	offsetof(struct drm_mtgpu_stream_open_data_out, sd_handle), \
	offsetof(struct drm_mtgpu_stream_open_data_out, bo_handle), \
	offsetof(struct drm_mtgpu_stream_open_data_out, bo_size), \
	offsetof(struct drm_mtgpu_stream_open_data_out, multi_readers_allowed), \
	offsetof(struct drm_mtgpu_stream_open_data_out, read_offset), \
	sizeof(struct drm_mtgpu_discover_stream_data_in), \
	offsetof(struct drm_mtgpu_discover_stream_data_in, pattern_name), \
	offsetof(struct drm_mtgpu_discover_stream_data_in, size), \
	sizeof(struct drm_mtgpu_discover_stream_data_out), \
	offsetof(struct drm_mtgpu_discover_stream_data_out, found_count), \
	offsetof(struct drm_mtgpu_discover_stream_data_out, stream_name), \
	sizeof(struct drm_mtgpu_acquire_data_in), \
	offsetof(struct drm_mtgpu_acquire_data_in, read_offset), \
	sizeof(struct drm_mtgpu_acquire_data_out), \
	offsetof(struct drm_mtgpu_acquire_data_out, read_len), \
	offsetof(struct drm_mtgpu_acquire_data_out, read_offset), \
	sizeof(struct drm_mtgpu_release_data_in), \
	offsetof(struct drm_mtgpu_release_data_in, read_len), \
	offsetof(struct drm_mtgpu_release_data_in, read_offset), \
	sizeof(struct drm_mtgpu_stream_close_data_in), \
	offsetof(struct drm_mtgpu_stream_close_data_in, bo_handle), \
	sizeof(struct drm_mtgpu_hwperf), \
	offsetof(struct drm_mtgpu_hwperf, in.type), \
	offsetof(struct drm_mtgpu_hwperf, in.toggle), \
	offsetof(struct drm_mtgpu_hwperf, in.stream_id), \
	offsetof(struct drm_mtgpu_hwperf, in.mask), \
	offsetof(struct drm_mtgpu_hwperf, out.data), \
	sizeof(struct mtgpu_hwperf_timestamps), \
	offsetof(struct mtgpu_hwperf_timestamps, soc_timestamp), \
	offsetof(struct mtgpu_hwperf_timestamps, os_timestamp), \
	sizeof(struct drm_mtgpu_notify_queue_update), \
	offsetof(struct drm_mtgpu_notify_queue_update, type), \
	offsetof(struct drm_mtgpu_notify_queue_update, ctx_handle), \
	sizeof(struct drm_mtgpu_codec_wait), \
	offsetof(struct drm_mtgpu_codec_wait, bo_handle), \
	offsetof(struct drm_mtgpu_codec_wait, offset), \
	offsetof(struct drm_mtgpu_codec_wait, flag), \
	offsetof(struct drm_mtgpu_codec_wait, timeout_ns), \
	sizeof(struct drm_mtgpu_align_check), \
	offsetof(struct drm_mtgpu_align_check, check_data), \
	offsetof(struct drm_mtgpu_align_check, check_size), \
	sizeof(struct drm_mtgpu_version_check), \
	offsetof(struct drm_mtgpu_version_check, in.api_major_version), \
	offsetof(struct drm_mtgpu_version_check, in.check_api_count), \
	offsetof(struct drm_mtgpu_version_check, in.check_data), \
	offsetof(struct drm_mtgpu_version_check, out.result_data), \
	offsetof(struct drm_mtgpu_version_check, out.supported_api_count), \
	sizeof(struct mtgpu_api_version), \
	offsetof(struct mtgpu_api_version, name), \
	offsetof(struct mtgpu_api_version, version_range)

#endif /* _MTGPU_ALIGNCHECK_H_ */
