/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_OSAL_H_
#define _MTVPU_OSAL_H_

#include "os-interface.h"

enum {MIN_LOG_LEVEL=0, ERR, WARN, INFO, MTDEBUG, VDI_TRACE, TRACE, MAX_LOG_LEVEL};

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
int osal_snprintf(char *str, size_t buf_size, const char *format, ...);

#ifndef va_arg
typedef __builtin_va_list va_list;
#define va_start(v, l)		__builtin_va_start(v, l)
#define va_end(v)               __builtin_va_end(v)
#define va_arg(v, l)            __builtin_va_arg(v, l)
#define va_copy(d, s)           __builtin_va_copy(d, s)
#endif
#endif /* _MTVPU_MEM_H_ */
