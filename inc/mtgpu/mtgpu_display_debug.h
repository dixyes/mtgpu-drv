/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTGPU_DISPLAY_DEBUG_H_
#define _MTGPU_DISPLAY_DEBUG_H_

#include "os-interface.h"

#if defined(OS_SECTION_ARG_TYPE_IS_NON_STRING)
#define _section(s) __section(s)
#else
#define _section(s) __section(#s)
#endif

extern ulong display_debug;

#define DISP_PRINT_REG		BIT(5)
#define DISP_PRINT_TRACE	BIT(4)
#define DISP_PRINT_DEBUG	BIT(3)
#define DISP_PRINT_INFO		BIT(2)
#define DISP_PRINT_WARN		BIT(1)
#define DISP_PRINT_ERROR	BIT(0)

#define DSC_DEBUG_MASK		(0xFF00000000UL)
#define DSC_DEBUG_CTL		((display_debug & DSC_DEBUG_MASK)   >> 32)
#define GLB_DEBUG_MASK		(0xFF000000UL)
#define GLB_DEBUG_CTL		((display_debug & GLB_DEBUG_MASK)   >> 24)
#define DISPC_DEBUG_MASK	(0x00FF0000UL)
#define DISPC_DEBUG_CTL		((display_debug & DISPC_DEBUG_MASK) >> 16)
#define HDMI_DEBUG_MASK		(0x0000FF00UL)
#define HDMI_DEBUG_CTL		((display_debug & HDMI_DEBUG_MASK)  >> 8)
#define DP_DEBUG_MASK		(0x000000FFUL)
#define DP_DEBUG_CTL		((display_debug & DP_DEBUG_MASK)    >> 0)

/* DSC debug log */
#define DSC_DBG_REG(fmt, ...)								\
({											\
	if (DSC_DEBUG_CTL & DISP_PRINT_REG)						\
		os_pr_info("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DSC_TRACE(fmt, ...)								\
({											\
	if (DSC_DEBUG_CTL & DISP_PRINT_TRACE)						\
		os_pr_info("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DSC_DEBUG(fmt, ...)								\
({											\
	if (DSC_DEBUG_CTL & DISP_PRINT_DEBUG)						\
		os_pr_info("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DSC_INFO(fmt, ...)								\
({											\
	if (DSC_DEBUG_CTL & DISP_PRINT_INFO)						\
		os_pr_info("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DSC_WARN(fmt, ...)								\
({											\
	if (DSC_DEBUG_CTL & DISP_PRINT_WARN)						\
		os_pr_warn("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DSC_ERROR(fmt, ...)								\
({											\
	if (DSC_DEBUG_CTL & DISP_PRINT_ERROR)						\
		os_pr_err("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DSC_INFO_ONCE(fmt, ...)									\
({												\
	if (DSC_DEBUG_CTL & DISP_PRINT_INFO) {							\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_info("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DSC_WARN_ONCE(fmt, ...)									\
({												\
	if (DSC_DEBUG_CTL & DISP_PRINT_WARN) {							\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_warn("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DSC_ERROR_ONCE(fmt, ...)								\
({												\
	if (DSC_DEBUG_CTL & DISP_PRINT_ERROR) {							\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_err("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DSC_ERROR_RATELIMITATED(fmt, ...)							\
({												\
	if (DSC_DEBUG_CTL & DISP_PRINT_ERROR)							\
		os_pr_err_ratelimited("DSC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

/* global debug log */
#define GLB_DBG_REG(fmt, ...)							\
({										\
	if (GLB_DEBUG_CTL & DISP_PRINT_REG)					\
		os_pr_info("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

#define GLB_TRACE(fmt, ...)							\
({										\
	if (GLB_DEBUG_CTL & DISP_PRINT_TRACE)					\
		os_pr_info("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

#define GLB_DEBUG(fmt, ...)							\
({										\
	if (GLB_DEBUG_CTL & DISP_PRINT_DEBUG)					\
		os_pr_info("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

#define GLB_INFO(fmt, ...)							\
({										\
	if (GLB_DEBUG_CTL & DISP_PRINT_INFO)					\
		os_pr_info("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

#define GLB_WARN(fmt, ...)							\
({										\
	if (GLB_DEBUG_CTL & DISP_PRINT_WARN)					\
		os_pr_warn("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

#define GLB_ERROR(fmt, ...)							\
({										\
	if (GLB_DEBUG_CTL & DISP_PRINT_ERROR)					\
		os_pr_err("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

#define GLB_INFO_ONCE(fmt, ...)								\
({											\
	if (GLB_DEBUG_CTL & DISP_PRINT_INFO) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_pr_info("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
		}									\
	}										\
})

#define GLB_WARN_ONCE(fmt, ...)								\
({											\
	if (GLB_DEBUG_CTL & DISP_PRINT_WARN) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_pr_warn("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
		}									\
	}										\
})

#define GLB_ERROR_ONCE(fmt, ...)							\
({											\
	if (GLB_DEBUG_CTL & DISP_PRINT_ERROR) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_pr_err("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
		}									\
	}										\
})

#define GLB_ERROR_RATELIMITATED(fmt, ...)						\
({											\
	if (GLB_DEBUG_CTL & DISP_PRINT_ERROR)						\
		os_pr_err_ratelimited("DISP-TOP: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

/* DISPC debug log */
#define DISPC_DBG_REG(fmt, ...)								\
({											\
	if (DISPC_DEBUG_CTL & DISP_PRINT_REG)						\
		os_pr_info("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DISPC_TRACE(fmt, ...)								\
({											\
	if (DISPC_DEBUG_CTL & DISP_PRINT_TRACE)						\
		os_pr_info("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DISPC_DEBUG(fmt, ...)								\
({											\
	if (DISPC_DEBUG_CTL & DISP_PRINT_DEBUG)						\
		os_pr_info("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DISPC_INFO(fmt, ...)								\
({											\
	if (DISPC_DEBUG_CTL & DISP_PRINT_INFO)						\
		os_pr_info("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DISPC_WARN(fmt, ...)								\
({											\
	if (DISPC_DEBUG_CTL & DISP_PRINT_WARN)						\
		os_pr_warn("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DISPC_ERROR(fmt, ...)								\
({											\
	if (DISPC_DEBUG_CTL & DISP_PRINT_ERROR)						\
		os_pr_err("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DISPC_INFO_ONCE(fmt, ...)								\
({												\
	if (DISPC_DEBUG_CTL & DISP_PRINT_INFO) {						\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_info("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DISPC_WARN_ONCE(fmt, ...)								\
({												\
	if (DISPC_DEBUG_CTL & DISP_PRINT_WARN) {						\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_warn("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DISPC_ERROR_ONCE(fmt, ...)								\
({												\
	if (DISPC_DEBUG_CTL & DISP_PRINT_ERROR) {						\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_err("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DISPC_ERROR_RATELIMITATED(fmt, ...)							\
({												\
	if (DISPC_DEBUG_CTL & DISP_PRINT_ERROR)							\
		os_pr_err_ratelimited("DISPC-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);\
})

/* HDMI debug log */
#define HDMI_DBG_REG(fmt, ...)								\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_REG)						\
		os_pr_info("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);		\
})

#define HDMI_TRACE(fmt, ...)								\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_TRACE)						\
		os_pr_info("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);		\
})

#define HDMI_DEBUG(fmt, ...)								\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_DEBUG)						\
		os_pr_info("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);		\
})

#define HDMI_INFO(fmt, ...)								\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_INFO)						\
		os_pr_info("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);		\
})

#define HDMI_WARN(fmt, ...)								\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_WARN)						\
		os_pr_warn("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);		\
})

#define HDMI_ERROR(fmt, ...)								\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_ERROR)						\
		os_pr_err("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);			\
})

#define HDMI_INFO_ONCE(fmt, ...)							\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_INFO) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_pr_info("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);	\
		}									\
	}										\
})

#define HDMI_WARN_ONCE(fmt, ...)							\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_WARN) {						\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_pr_warn("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);	\
		}									\
	}										\
})

#define HDMI_ERROR_ONCE(fmt, ...)							\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_ERROR) {					\
		static bool _section(.data.once) __print_once;				\
											\
		if (!__print_once) {							\
			__print_once = true;						\
			os_pr_err("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);		\
		}									\
	}										\
})

#define HDMI_WARN_RATELIMITED(fmt, ...)							\
({											\
	if (HDMI_DEBUG_CTL & DISP_PRINT_WARN)						\
		os_pr_warn_ratelimited("HDMI: %s() " fmt, __func__, ##__VA_ARGS__);	\
})

/* DP debug log */
#define DP_DBG_REG(fmt, ...)								\
({											\
	if (DP_DEBUG_CTL & DISP_PRINT_REG)						\
		os_pr_info("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DP_TRACE(fmt, ...)								\
({											\
	if (DP_DEBUG_CTL & DISP_PRINT_TRACE)						\
		os_pr_info("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DP_DEBUG(fmt, ...)								\
({											\
	if (DP_DEBUG_CTL & DISP_PRINT_DEBUG)						\
		os_pr_info("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DP_INFO(fmt, ...)								\
({											\
	if (DP_DEBUG_CTL & DISP_PRINT_INFO)						\
		os_pr_info("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DP_WARN(fmt, ...)								\
({											\
	if (DP_DEBUG_CTL & DISP_PRINT_WARN)						\
		os_pr_warn("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DP_ERROR(fmt, ...)								\
({											\
	if (DP_DEBUG_CTL & DISP_PRINT_ERROR)						\
		os_pr_err("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
})

#define DP_INFO_ONCE(fmt, ...)									\
({												\
	if (DP_DEBUG_CTL & DISP_PRINT_INFO) {							\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_info("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DP_WARN_ONCE(fmt, ...)									\
({												\
	if (DP_DEBUG_CTL & DISP_PRINT_WARN) {							\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_warn("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#define DP_ERROR_ONCE(fmt, ...)									\
({												\
	if (DP_DEBUG_CTL & DISP_PRINT_ERROR) {							\
		static bool _section(.data.once) __print_once;					\
												\
		if (!__print_once) {								\
			__print_once = true;							\
			os_pr_err("DP-%d: %s() " fmt, ctx->id, __func__, ##__VA_ARGS__);	\
		}										\
	}											\
})

#endif /* _MTGPU_DISPLAY_DEBUG_H_ */
