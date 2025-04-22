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
#define MTGPU_IOCTL_CHECKSUM \
	sizeof(struct drm_mtgpu_ioctl_args) + \
	(offsetof(struct drm_mtgpu_ioctl_args, cmd_type) << 1) + \
	(offsetof(struct drm_mtgpu_ioctl_args, cmd) << 2) +\
	(offsetof(struct drm_mtgpu_ioctl_args, checksum) << 3) + \
	(offsetof(struct drm_mtgpu_ioctl_args, size) << 4) + \
	(offsetof(struct drm_mtgpu_ioctl_args, data) << 5)

#define MTGPU_ABI_DMA_CMD_CHECKSUM \
	sizeof(struct drm_mtgpu_dma_cmd) + \
	(offsetof(struct drm_mtgpu_dma_cmd, abi_version) << 1) + \
	(offsetof(struct drm_mtgpu_dma_cmd, abi_checksum) << 2) + \
	(offsetof(struct drm_mtgpu_dma_cmd, src_type) << 3) + \
	(offsetof(struct drm_mtgpu_dma_cmd, dst_type) << 4) + \
	(offsetof(struct drm_mtgpu_dma_cmd, src_addr) << 5) + \
	(offsetof(struct drm_mtgpu_dma_cmd, dst_addr) << 6) + \
	(offsetof(struct drm_mtgpu_dma_cmd, src_offset) << 7) + \
	(offsetof(struct drm_mtgpu_dma_cmd, dst_offset) << 8) + \
	(offsetof(struct drm_mtgpu_dma_cmd, xfer_size) << 9)

#define MTGPU_CORE_CMD_DEVICE_INIT_CHECKSUM \
	sizeof(struct drm_mtgpu_device_init) + \
	(offsetof(struct drm_mtgpu_device_init, in.api_major_version) << 1) + \
	(offsetof(struct drm_mtgpu_device_init, in.libdrm_version) << 2) + \
	(offsetof(struct drm_mtgpu_device_init, in.shared_inc_version) << 3)

#define MTGPU_CORE_CMD_GET_VERSION_LIST_CHECKSUM \
	sizeof(struct drm_mtgpu_get_version_list) + \
	(offsetof(struct drm_mtgpu_get_version_list, out.supported_api_count) << 1) + \
	(offsetof(struct drm_mtgpu_get_version_list, out.supported_abi_count) << 2) + \
	(offsetof(struct drm_mtgpu_get_version_list, out.supported_fwif_count) << 3) + \
	(offsetof(struct drm_mtgpu_get_version_list, out.api_version_data) << 4) + \
	(offsetof(struct drm_mtgpu_get_version_list, out.abi_version_data) << 5) + \
	(offsetof(struct drm_mtgpu_get_version_list, out.fwif_version_data) << 6) + \
	(sizeof(struct mtgpu_api_version_info) << 7) + \
	(offsetof(struct mtgpu_api_version_info, api_id) << 8) + \
	(offsetof(struct mtgpu_api_version_info, version_min) << 9) + \
	(offsetof(struct mtgpu_api_version_info, version_max) << 10) + \
	(sizeof(struct mtgpu_abi_version_info) << 11) + \
	(offsetof(struct mtgpu_abi_version_info, abi_id) << 12) + \
	(offsetof(struct mtgpu_abi_version_info, version_min) << 13) + \
	(offsetof(struct mtgpu_abi_version_info, version_max) << 14)

#define MTGPU_QUERY_CMD_HEAP_COUNT_CHECKSUM \
	sizeof(struct drm_mtgpu_query_heap_count) + \
	(offsetof(struct drm_mtgpu_query_heap_count, out.heap_count) << 1)

#define MTGPU_QUERY_CMD_HEAP_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_heap_info) + \
	(offsetof(struct drm_mtgpu_query_heap_info, in.index) << 1) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.id) << 2) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.name) << 3) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.base) << 4) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.length) << 5) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.log2_page_size) << 6) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.enable_multi_page_size) << 7) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.page_shift_bit_mask) << 8) + \
	(offsetof(struct drm_mtgpu_query_heap_info, out.log2_import_alignment) << 9)

#define MTGPU_QUERY_CMD_MEM_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_mem_info) + \
	(offsetof(struct drm_mtgpu_query_mem_info, out.vram_hw_size) << 1) + \
	(offsetof(struct drm_mtgpu_query_mem_info, out.vram_total_size) << 2) + \
	(offsetof(struct drm_mtgpu_query_mem_info, out.vram_free_size) << 3) + \
	(offsetof(struct drm_mtgpu_query_mem_info, out.sysmem_total_size) << 4) + \
	(offsetof(struct drm_mtgpu_query_mem_info, out.sysmem_free_size) << 5)

#define MTGPU_QUERY_CMD_BO_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_bo_info) + \
	(offsetof(struct drm_mtgpu_query_bo_info, in.bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_query_bo_info, in.metadata_id) << 2) + \
	(offsetof(struct drm_mtgpu_query_bo_info, in.metadata_addr) << 3) + \
	(offsetof(struct drm_mtgpu_query_bo_info, in.metadata_size) << 4) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.size) << 5) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.align) << 6) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.flags) << 7) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.domain) << 8) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.segment_id) << 9) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.has_metadata) << 10) + \
	(offsetof(struct drm_mtgpu_query_bo_info, out.name) << 11)

#define MTGPU_QUERY_CMD_DEV_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_dev_info) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.dev_id) << 1) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.marketing_name) << 2) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.dev_status) << 3) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.dev_clock_speed) << 4) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.mem_clock_speed) << 5) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.mem_max_clock_speed) << 6) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.soc_timer_clock_speed) << 7) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.num_cores) << 8) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.mpx_map) << 9) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.uuid) << 10) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.subvendor_id) << 11) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.subsystem_id) << 12) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.llc_persisting_hw_max_size) << 13) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.llc_size) << 14) + \
	(offsetof(struct drm_mtgpu_query_dev_info, out.is_igpu) << 15)

#define MTGPU_QUERY_CMD_PCI_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_pci_info) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.domain_number) << 1) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.bus_number) << 2) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.device_number) << 3) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.function_number) << 4) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.current_gen_speed) << 5) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.current_width) << 6) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.numa_node_id) << 7) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.total_pci_device_memory_accessible) << 8) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.total_system_memory_accessible) << 9) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.no_snoop) << 10) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.subvendor_id) << 11) + \
	(offsetof(struct drm_mtgpu_query_pci_info, out.subsystem_id) << 12)

#define MTGPU_QUERY_CMD_PLATFORM_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_platform_info) + \
	(offsetof(struct drm_mtgpu_query_platform_info, out.mtlink_enable) << 1) + \
	(offsetof(struct drm_mtgpu_query_platform_info, out.iommu_enable) << 2) + \
	(offsetof(struct drm_mtgpu_query_platform_info, out.is_vps) << 3) + \
	(offsetof(struct drm_mtgpu_query_platform_info, out.platform_type) << 4) + \
	(offsetof(struct drm_mtgpu_query_platform_info, out.direct_cache_access_support) << 5)

#define MTGPU_QUERY_CMD_HW_CAPABILITY_CHECKSUM \
	sizeof(struct drm_mtgpu_query_hw_capability)

#define MTGPU_QUERY_CMD_DRIVER_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_query_driver_info) + \
	(offsetof(struct drm_mtgpu_query_driver_info, out.sched_mode) << 1)

#define MTGPU_QUERY_CMD_P2P_CAPABILITY_CHECKSUM \
	sizeof(struct drm_mtgpu_query_p2p_capability) + \
	(offsetof(struct drm_mtgpu_query_p2p_capability, in.peer_fd) << 1) + \
	(offsetof(struct drm_mtgpu_query_p2p_capability, out.pci_capability) << 2) + \
	(offsetof(struct drm_mtgpu_query_p2p_capability, out.mtlink_capability) << 3) + \
	(offsetof(struct drm_mtgpu_query_p2p_capability, out.mtlink_version) << 4) + \
	(offsetof(struct drm_mtgpu_query_p2p_capability, out.mtlink_bandwidth) << 5) + \
	(offsetof(struct drm_mtgpu_query_p2p_capability, out.mtlink_link_num) << 6)

#define MTGPU_QUERY_CMD_MTLINK_PATH_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_mtlink_path_info) + \
	(offsetof(struct drm_mtgpu_mtlink_path_info, in.peer_fd) << 1) + \
	(offsetof(struct drm_mtgpu_mtlink_path_info, out.path_num) << 2) + \
	(sizeof(struct mtgpu_mtlink_path) << 3) + \
	(offsetof(struct mtgpu_mtlink_path, length) << 4) + \
	(offsetof(struct mtgpu_mtlink_path, path_node) << 5)

#define MTGPU_QUERY_CMD_MISC_INFO_CHECKSUM \
	sizeof(struct drm_mtgpu_misc_info) + \
	(offsetof(struct drm_mtgpu_misc_info, out.misc_count) << 1) + \
	(offsetof(struct drm_mtgpu_misc_info, out.misc_id) << 2)

#define MTGPU_BO_CMD_ALLOC_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_create) + \
	(offsetof(struct drm_mtgpu_bo_create, in.size) << 1) + \
	(offsetof(struct drm_mtgpu_bo_create, in.align) << 2) + \
	(offsetof(struct drm_mtgpu_bo_create, in.flags) << 3) + \
	(offsetof(struct drm_mtgpu_bo_create, in.domains) << 4) + \
	(offsetof(struct drm_mtgpu_bo_create, in.group_id) << 5) + \
	(offsetof(struct drm_mtgpu_bo_create, in.name) << 6) + \
	(offsetof(struct drm_mtgpu_bo_create, out.bo_handle) << 7)

#define MTGPU_BO_CMD_FROM_USERPTR_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_from_userptr) + \
	(offsetof(struct drm_mtgpu_bo_from_userptr, in.userptr) << 1) + \
	(offsetof(struct drm_mtgpu_bo_from_userptr, in.size) << 2) + \
	(offsetof(struct drm_mtgpu_bo_from_userptr, in.flags) << 3) + \
	(offsetof(struct drm_mtgpu_bo_from_userptr, out.bo_handle) << 4)

#define MTGPU_BO_CMD_GET_MMAP_OFFSET_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_get_mmap_offset) + \
	(offsetof(struct drm_mtgpu_bo_get_mmap_offset, in.bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_bo_get_mmap_offset, out.offset) << 2)

#define MTGPU_BO_CMD_EXPORT_GLOBAL_HANDLE_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_global_handle_export) + \
	(offsetof(struct drm_mtgpu_bo_global_handle_export, in.bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_bo_global_handle_export, out.global_handle) << 2)

#define MTGPU_BO_CMD_IMPORT_GLOBAL_HANDLE_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_global_handle_import) + \
	(offsetof(struct drm_mtgpu_bo_global_handle_import, in.global_handle) << 1) + \
	(offsetof(struct drm_mtgpu_bo_global_handle_import, out.size) << 2) + \
	(offsetof(struct drm_mtgpu_bo_global_handle_import, out.bo_handle) << 3)

#define MTGPU_BO_CMD_SET_METADATA_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_set_metadata) + \
	(offsetof(struct drm_mtgpu_bo_set_metadata, bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_bo_set_metadata, metadata_addr) << 2) + \
	(offsetof(struct drm_mtgpu_bo_set_metadata, metadata_size) << 3)

#define MTGPU_BO_CMD_ADD_METADATA_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_add_metadata) + \
	(offsetof(struct drm_mtgpu_bo_add_metadata, bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_bo_add_metadata, metadata_addr) << 2) + \
	(offsetof(struct drm_mtgpu_bo_add_metadata, metadata_size) << 3) + \
	(offsetof(struct drm_mtgpu_bo_add_metadata, metadata_id) << 4)

#define MTGPU_BO_CMD_GET_METADATA_CHECKSUM \
	sizeof(struct drm_mtgpu_bo_get_metadata) + \
	(offsetof(struct drm_mtgpu_bo_get_metadata, bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_bo_get_metadata, metadata_addr) << 2) + \
	(offsetof(struct drm_mtgpu_bo_get_metadata, metadata_size) << 3) + \
	(offsetof(struct drm_mtgpu_bo_get_metadata, metadata_id) << 4)

#define MTGPU_VM_CMD_CONTEXT_CREATE_CHECKSUM \
	sizeof(struct drm_mtgpu_vm_context_create) + \
	(offsetof(struct drm_mtgpu_vm_context_create, vm_ctx_handle) << 1)

#define MTGPU_VM_CMD_CONTEXT_DESTROY_CHECKSUM \
	sizeof(struct drm_mtgpu_vm_context_destroy) + \
	(offsetof(struct drm_mtgpu_vm_context_destroy, vm_ctx_handle) << 1)

#define MTGPU_VM_CMD_MAP_CHECKSUM \
	sizeof(struct drm_mtgpu_vm_map) + \
	(offsetof(struct drm_mtgpu_vm_map, vm_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_vm_map, va) << 2) +\
	(offsetof(struct drm_mtgpu_vm_map, mapping_flags) << 3) +\
	(offsetof(struct drm_mtgpu_vm_map, bo_handle) << 4) +\
	(offsetof(struct drm_mtgpu_vm_map, size) << 5) +\
	(offsetof(struct drm_mtgpu_vm_map, log2_page_size) << 6)

#define MTGPU_VM_CMD_MAP_ASYNC_CHECKSUM \
	sizeof(struct drm_mtgpu_vm_map_async) + \
	(offsetof(struct drm_mtgpu_vm_map_async, vm_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_vm_map_async, va) << 2) + \
	(offsetof(struct drm_mtgpu_vm_map_async, mapping_flags) << 3) + \
	(offsetof(struct drm_mtgpu_vm_map_async, bo_handle) << 4) + \
	(offsetof(struct drm_mtgpu_vm_map_async, size) << 5) + \
	(offsetof(struct drm_mtgpu_vm_map_async, log2_page_size) << 6) + \
	(offsetof(struct drm_mtgpu_vm_map_async, update_semaphore_count) << 7) + \
	(offsetof(struct drm_mtgpu_vm_map_async, update_semaphore) << 8)

#define MTGPU_VM_CMD_UNMAP_CHECKSUM \
	sizeof(struct drm_mtgpu_vm_unmap) + \
	(offsetof(struct drm_mtgpu_vm_unmap, vm_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_vm_unmap, va) << 2)

#define MTGPU_SYNC_CMD_SEMAPHORE_CREATE_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_create) + \
	(offsetof(struct drm_mtgpu_semaphore_create, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_create, in.flag) << 2) + \
	(offsetof(struct drm_mtgpu_semaphore_create, out.handle) << 3) + \
	(offsetof(struct drm_mtgpu_semaphore_create, out.bo_handle) << 4) + \
	(offsetof(struct drm_mtgpu_semaphore_create, out.shadow_bo_handle) << 5) + \
	(offsetof(struct drm_mtgpu_semaphore_create, out.value_offset) << 6) + \
	(offsetof(struct drm_mtgpu_semaphore_create, out.gpu_address) << 7)

#define MTGPU_SYNC_CMD_SEMAPHORE_DESTROY_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_destroy) + \
	(offsetof(struct drm_mtgpu_semaphore_destroy, handle) << 1)

#define MTGPU_SYNC_CMD_SEMAPHORE_CPU_SIGNAL_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_cpu_signal) + \
	(offsetof(struct drm_mtgpu_semaphore_cpu_signal, vm_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_cpu_signal, semaphore) << 2) + \
	(sizeof(struct drm_mtgpu_semaphore) << 3) + \
	(offsetof(struct drm_mtgpu_semaphore, handle) << 4) + \
	(offsetof(struct drm_mtgpu_semaphore, value) << 5)

#define MTGPU_SYNC_CMD_SEMAPHORE_WAIT_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_wait) + \
	(offsetof(struct drm_mtgpu_semaphore_wait, in.semaphores) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_wait, in.count) << 2) + \
	(offsetof(struct drm_mtgpu_semaphore_wait, in.timeout_ns) << 3)

#define MTGPU_SYNC_CMD_SEMAPHORE_EXPORT_GLOBAL_HANDLE_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_export_global_handle) + \
	(offsetof(struct drm_mtgpu_semaphore_export_global_handle, in.handle) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_export_global_handle, out.global_handle) << 2)

#define MTGPU_SYNC_CMD_SEMAPHORE_IMPORT_GLOBAL_HANDLE_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_import_global_handle) + \
	(offsetof(struct drm_mtgpu_semaphore_import_global_handle, in.global_handle) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_import_global_handle, in.flag) << 2) + \
	(offsetof(struct drm_mtgpu_semaphore_import_global_handle, out.handle) << 3) + \
	(offsetof(struct drm_mtgpu_semaphore_import_global_handle, out.bo_handle) << 4) + \
	(offsetof(struct drm_mtgpu_semaphore_import_global_handle, out.shadow_bo_handle) << 5) + \
	(offsetof(struct drm_mtgpu_semaphore_import_global_handle, out.value_offset) << 6)

#define MTGPU_SYNC_CMD_SEMAPHORE_SUBMIT_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_submit) + \
	(offsetof(struct drm_mtgpu_semaphore_submit, ctx_handle) << 1) +\
	(offsetof(struct drm_mtgpu_semaphore_submit, job_type) << 2) + \
	(offsetof(struct drm_mtgpu_semaphore_submit, submit_type) << 3) + \
	(offsetof(struct drm_mtgpu_semaphore_submit, semaphore) << 4) + \
	(sizeof(struct drm_mtgpu_semaphore) << 5) + \
	(offsetof(struct drm_mtgpu_semaphore, handle) << 6) + \
	(offsetof(struct drm_mtgpu_semaphore, value) << 7)

#define MTGPU_SYNC_CMD_SEMAPHORE_EXPORT_FD_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_export_fd) + \
	(offsetof(struct drm_mtgpu_semaphore_export_fd, in.semaphore) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_export_fd, out.fd) << 2) + \
	(sizeof(struct drm_mtgpu_semaphore) << 3) + \
	(offsetof(struct drm_mtgpu_semaphore, handle) << 4) + \
	(offsetof(struct drm_mtgpu_semaphore, value) << 5)

#define MTGPU_SYNC_CMD_SEMAPHORE_IMPORT_FD_CHECKSUM \
	sizeof(struct drm_mtgpu_semaphore_import_fd) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, in.fd) << 1) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, in.vm_ctx_handle) << 2) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, out.semaphore) << 3) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, out.bo_handle) << 4) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, out.shadow_bo_handle) << 5) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, out.value_offset) << 6) + \
	(offsetof(struct drm_mtgpu_semaphore_import_fd, out.type) << 7) + \
	(sizeof(struct drm_mtgpu_semaphore) << 8) + \
	(offsetof(struct drm_mtgpu_semaphore, handle) << 9) + \
	(offsetof(struct drm_mtgpu_semaphore, value) << 10)

#define MTGPU_JOB_CMD_CONTEXT_CREATE_V3_CHECKSUM \
	sizeof(struct drm_mtgpu_job_context_create) + \
	(offsetof(struct drm_mtgpu_job_context_create, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_job_context_create, in.flags) << 2) + \
	(offsetof(struct drm_mtgpu_job_context_create, in.priority) << 3) + \
	(offsetof(struct drm_mtgpu_job_context_create, in.vm_ctx_handle) << 4) + \
	(offsetof(struct drm_mtgpu_job_context_create, out.ctx_handle) << 5)

#define MTGPU_JOB_CMD_CONTEXT_DESTROY_V3_CHECKSUM \
	sizeof(struct drm_mtgpu_job_context_destroy) + \
	(offsetof(struct drm_mtgpu_job_context_destroy, ctx_handle) << 1)

#define MTGPU_JOB_CMD_SUBMIT_V3_CHECKSUM \
	sizeof(struct drm_mtgpu_job_submit_v3) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.job_ctx_handle) << 1)  + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.check_semaphores) << 2) +\
	(offsetof(struct drm_mtgpu_job_submit_v3, in.check_semaphore_count) << 3) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.update_semaphores) << 4) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.update_semaphore_count) << 5) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.buf_sync_fds) << 6) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.buf_sync_flags) << 7) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.buf_sync_count) << 8) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.submission_va) << 9) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.submission_size) << 10) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.submission_flags) << 11) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, in.submission_id) << 12) + \
	(offsetof(struct drm_mtgpu_job_submit_v3, out.data) << 13)

#define MTGPU_JOB_CMD_APPEND_CHECKSUM \
	sizeof(struct drm_mtgpu_job_append) + \
	(offsetof(struct drm_mtgpu_job_append, job_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_job_append, stream_uid) << 2)

#define MTGPU_JOB_CMD_SUBMIT_WITH_DOORBELL_CHECKSUM \
	sizeof(struct drm_mtgpu_job_submit_with_doorbell) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.job_ctx_handle) << 1)  + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.check_semaphores) << 2) +\
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.check_semaphore_count) << 3) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.update_semaphores) << 4) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.update_semaphore_count) << 5) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.submission_va) << 6) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.submission_size) << 7) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.submission_flags) << 8) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.submission_id) << 9) + \
	(offsetof(struct drm_mtgpu_job_submit_with_doorbell, in.doorbell_handle) << 10)

#define MTGPU_JOB_CMD_ACQUIRE_DOORBELL_CHECKSUM \
	sizeof(struct drm_mtgpu_job_acquire_doorbell) + \
	(offsetof(struct drm_mtgpu_job_acquire_doorbell, in.job_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_job_acquire_doorbell, in.user_va) << 2) + \
	(offsetof(struct drm_mtgpu_job_acquire_doorbell, out.doorbell_handle) << 3) + \
	(offsetof(struct drm_mtgpu_job_acquire_doorbell, out.doorbell_addr_offset) << 4)

#define MTGPU_JOB_CMD_RELEASE_DOORBELL_CHECKSUM \
	sizeof(struct drm_mtgpu_job_release_doorbell) + \
	(offsetof(struct drm_mtgpu_job_release_doorbell, in.job_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_job_release_doorbell, in.doorbell_handle) << 2)

#define MTGPU_JOB_CMD_CODEC_WAIT_CHECKSUM \
	sizeof(struct drm_mtgpu_codec_wait) + \
	(offsetof(struct drm_mtgpu_codec_wait, bo_handle) << 1) + \
	(offsetof(struct drm_mtgpu_codec_wait, offset) << 2) + \
	(offsetof(struct drm_mtgpu_codec_wait, flag) << 3) + \
	(offsetof(struct drm_mtgpu_codec_wait, timeout_ns) << 4)

#define MTGPU_PERF_CMD_HWPERF_CONTROL_CHECKSUM \
	sizeof(struct drm_mtgpu_hwperf_control) + \
	(offsetof(struct drm_mtgpu_hwperf_control, in.toggle) << 1) + \
	(offsetof(struct drm_mtgpu_hwperf_control, in.stream_id) << 2) + \
	(offsetof(struct drm_mtgpu_hwperf_control, in.mask) << 3) + \
	(offsetof(struct drm_mtgpu_hwperf_control, out.data) << 4)

#define MTGPU_PERF_CMD_HWPERF_GET_TIMESTAMPS_CHECKSUM \
	sizeof(struct drm_mtgpu_hwperf_get_timestamps) + \
	(offsetof(struct drm_mtgpu_hwperf_get_timestamps, out.soc_timestamp) << 1) + \
	(offsetof(struct drm_mtgpu_hwperf_get_timestamps, out.os_timestamp) << 2)

#define MTGPU_PERF_CMD_HWPERF_FLUSH_BUFFER_CHECKSUM \
	sizeof(struct drm_mtgpu_hwperf_flush_buffer) + \
	(offsetof(struct drm_mtgpu_hwperf_flush_buffer, out.num_flushed) << 1)

#define MTGPU_PERF_CMD_TL_STREAM_OPEN_CHECKSUM \
	sizeof(struct drm_mtgpu_transport_layer) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.sd_handle) << 2) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.data) << 3) + \
	(offsetof(struct drm_mtgpu_transport_layer, out.data) << 4) + \
	(sizeof(struct drm_mtgpu_stream_open_data_in) << 5) + \
	(offsetof(struct drm_mtgpu_stream_open_data_in, name) << 6) + \
	(offsetof(struct drm_mtgpu_stream_open_data_in, mode) << 7) + \
	(sizeof(struct drm_mtgpu_stream_open_data_out) << 8) + \
	(offsetof(struct drm_mtgpu_stream_open_data_out, sd_handle) << 9) + \
	(offsetof(struct drm_mtgpu_stream_open_data_out, bo_handle) << 10) + \
	(offsetof(struct drm_mtgpu_stream_open_data_out, bo_size) << 11) + \
	(offsetof(struct drm_mtgpu_stream_open_data_out, multi_readers_allowed) << 12) + \
	(offsetof(struct drm_mtgpu_stream_open_data_out, read_offset) << 13) + \
	(offsetof(struct drm_mtgpu_stream_open_data_out, tl_stream_size) << 14)

#define MTGPU_PERF_CMD_TL_STREAM_CLOSE_CHECKSUM \
	sizeof(struct drm_mtgpu_transport_layer) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.sd_handle) << 2) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.data) << 3) + \
	(offsetof(struct drm_mtgpu_transport_layer, out.data) << 4) + \
	(sizeof(struct drm_mtgpu_stream_close_data_in) << 5) + \
	(offsetof(struct drm_mtgpu_stream_close_data_in, bo_handle) << 6)

#define MTGPU_PERF_CMD_TL_STREAM_DISCOVER_CHECKSUM \
	sizeof(struct drm_mtgpu_transport_layer) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.sd_handle) << 2) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.data) << 3) + \
	(offsetof(struct drm_mtgpu_transport_layer, out.data) << 4) + \
	(sizeof(struct drm_mtgpu_discover_stream_data_in) << 5) + \
	(offsetof(struct drm_mtgpu_discover_stream_data_in, pattern_name) << 6) + \
	(offsetof(struct drm_mtgpu_discover_stream_data_in, size) << 7) + \
	(sizeof(struct drm_mtgpu_discover_stream_data_out) << 8) + \
	(offsetof(struct drm_mtgpu_discover_stream_data_out, found_count) << 9) + \
	(offsetof(struct drm_mtgpu_discover_stream_data_out, stream_name) << 10)

#define MTGPU_PERF_CMD_TL_STREAM_ACQUIRE_CHECKSUM \
	sizeof(struct drm_mtgpu_transport_layer) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.sd_handle) << 2) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.data) << 3) + \
	(offsetof(struct drm_mtgpu_transport_layer, out.data) << 4) + \
	(sizeof(struct drm_mtgpu_acquire_data_in) << 5) + \
	(offsetof(struct drm_mtgpu_acquire_data_in, read_offset) << 6) + \
	(sizeof(struct drm_mtgpu_acquire_data_out) << 7) + \
	(offsetof(struct drm_mtgpu_acquire_data_out, read_len) << 8) + \
	(offsetof(struct drm_mtgpu_acquire_data_out, read_offset) << 9)

#define MTGPU_PERF_CMD_TL_STREAM_RELEASE_CHECKSUM \
	sizeof(struct drm_mtgpu_transport_layer) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.type) << 1) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.sd_handle) << 2) + \
	(offsetof(struct drm_mtgpu_transport_layer, in.data) << 3) + \
	(offsetof(struct drm_mtgpu_transport_layer, out.data) << 4) + \
	(sizeof(struct drm_mtgpu_release_data_in) << 5) + \
	(offsetof(struct drm_mtgpu_release_data_in, read_len) << 6) + \
	(offsetof(struct drm_mtgpu_release_data_in, read_offset) << 7)

#define MTGPU_PERF_CMD_MSS_PFM_CONFIG_CHECKSUM \
	sizeof(struct drm_mtgpu_mss_pfm_config) + \
	(offsetof(struct drm_mtgpu_mss_pfm_config, job_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_mss_pfm_config, size) << 2) + \
	(offsetof(struct drm_mtgpu_mss_pfm_config, data) << 3)

#define MTGPU_PERF_CMD_GET_CONTAINER_PID_CHECKSUM \
	sizeof(struct drm_mtgpu_get_container_pid) + \
	(offsetof(struct drm_mtgpu_get_container_pid, in.host_pid) << 1) + \
	(offsetof(struct drm_mtgpu_get_container_pid, out.container_pid) << 2)

#define MTGPU_JOB_CMD_GET_SUBMISSION_LAST_ERROR_CHECKSUM \
	sizeof(struct drm_mtgpu_get_submission_last_error) + \
	(offsetof(struct drm_mtgpu_get_submission_last_error, in.job_ctx_handle) << 1) + \
	(offsetof(struct drm_mtgpu_get_submission_last_error, out.submission_last_error) << 2)

#define MTGPU_JOB_CMD_GET_DEVICE_LAST_ERROR_CHECKSUM \
	sizeof(struct drm_mtgpu_get_device_last_error) + \
	(offsetof(struct drm_mtgpu_get_device_last_error, out.device_last_error) << 1)

#endif /* _MTGPU_ALIGNCHECK_H_ */
