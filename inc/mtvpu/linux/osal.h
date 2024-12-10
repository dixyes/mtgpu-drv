/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_OSAL_H_
#define _MTVPU_OSAL_H_

#include "os-interface.h"

enum {MIN_LOG_LEVEL=0, ERR, WARN, INFO, MTDEBUG, TRACE, MAX_LOG_LEVEL};

extern int mtvpu_log_level;

#define VLOG(level, fmt, ...)						      \
({									                          \
	if (unlikely(level <= mtvpu_log_level))	    \
		os_pr_info("[mtvpu] " fmt, ##__VA_ARGS__);	\
})

u64 osal_gettime(void);
void *osal_malloc(int size);
void osal_free(void *p);
void osal_memcpy(void *dst, const void *src, int count);
void *osal_memset(void *dst, int val, int count);

#endif /* _MTVPU_MEM_H_ */
