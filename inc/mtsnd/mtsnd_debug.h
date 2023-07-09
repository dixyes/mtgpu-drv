// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright(c) 2022 Moore Threads Technologies Ltd. All rights reserved.
 *
 * This file is provided under a dual MIT/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 */

#ifndef _MTSND_DEBUG_H
#define _MTSND_DEBUG_H

#include "os-interface.h"

#if defined(OS_SECTION_ARG_TYPE_IS_NON_STRING)
#define _section(s) __section(s)
#else
#define _section(s) __section(#s)
#endif

extern unsigned int snd_debug;

#define SND_PRINT_TRACE		(1UL << 4)
#define SND_PRINT_DEBUG		(1UL << 3)
#define SND_PRINT_INFO		(1UL << 2)
#define SND_PRINT_WARN		(1UL << 1)
#define SND_PRINT_ERROR		(1UL << 0)

#define SND_TRACE()						\
({								\
	if (snd_debug & SND_PRINT_TRACE)			\
		os_printk("mtsnd: %s", __func__);			\
})

#define SND_DEBUG(fmt, ...)							\
({										\
	if (snd_debug & SND_PRINT_DEBUG)					\
		os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
})
#define SND_INFO(fmt, ...)							\
({										\
	if (snd_debug & SND_PRINT_INFO)						\
		os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
})

#define SND_WARN(fmt, ...)							\
({										\
	if (snd_debug & SND_PRINT_WARN)						\
		os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
})

#define SND_ERROR(fmt, ...)							\
({										\
	if (snd_debug & SND_PRINT_ERROR)					\
		os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
})

#define SND_INFO_ONCE(fmt, ...)								\
({											\
	if (snd_debug & SND_PRINT_INFO) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
		}									\
	}										\
})

#define SND_WARN_ONCE(fmt, ...)								\
({											\
	if (snd_debug & SND_PRINT_WARN) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
		}									\
	}										\
})


#define SND_ERROR_ONCE(fmt, ...)							\
({											\
	if (snd_debug & SND_PRINT_ERROR) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_printk("mtsnd: %s: " fmt, __func__, ##__VA_ARGS__);		\
		}									\
	}										\
})

#endif /* _MTSND_DEBUG_H */
