/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#if !defined(__SYSINFO_H__)
#define __SYSINFO_H__

#include "mtgpu_defs.h"
/*!< System specific poll/timeout details */
#if defined(VIRTUAL_PLATFORM)
#define MAX_HW_TIME_US                           (240000000)
#define DEVICES_WATCHDOG_POWER_ON_SLEEP_TIMEOUT  (120000)
#else
#define MAX_HW_TIME_US                           (20000000)
#define DEVICES_WATCHDOG_POWER_ON_SLEEP_TIMEOUT  (10000) // (1500)
#endif
#define DEVICES_WATCHDOG_POWER_OFF_SLEEP_TIMEOUT (3600000)
#define WAIT_TRY_COUNT                           (10000)

#define SYS_RGX_DEV_NAME MTGPU_DEVICE_NAME

#endif /* !defined(__SYSINFO_H__) */
