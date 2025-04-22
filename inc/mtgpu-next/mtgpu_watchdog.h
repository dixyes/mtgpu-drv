/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_WATCHDOG_H_
#define _MTGPU_WATCHDOG_H_

struct _PVRSRV_RGXDEV_INFO_;

void watchdog_info_reset(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
int mtgpu_watchdog_init(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_watchdog_deinit(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_watchdog_start(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_watchdog_stop(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_watchdog_set_response(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
bool mtgpu_watchdog_is_meta_hang(struct _PVRSRV_RGXDEV_INFO_ *dev_info);
void mtgpu_watchdog_set_fec_coredump(struct _PVRSRV_RGXDEV_INFO_ *dev_info);

#endif /* _MTGPU_WATCHDOG_H_ */
