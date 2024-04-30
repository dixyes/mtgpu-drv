/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#	define PLATFORM_WIN32
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#	define PLATFORM_LINUX
#else
#	define PLATFORM_NON_OS
#endif

#if defined(_MSC_VER)
/* #include <windows.h> */
#define inline _inline
#elif defined(__GNUC__)
#elif defined(__ARMCC__)
#else
#  error "Unknown compiler."
#endif

#define VPUAPI_VERSION		0x564

#if defined(linux) || defined(__linux) || defined(ANDROID) || defined(CNM_FPGA_HAPS_INTERFACE) || defined(CNM_SIM_PLATFORM)
#define SUPPORT_MULTI_INST_INTR
#endif
#if defined(linux) || defined(__linux) || defined(ANDROID)
#define SUPPORT_INTERRUPT
#endif

//#define SUPPORT_SMMU

/* CODA980 */
#define CODA980

#define CLIP_PIC_DELTA_QP
#define RC_MIN_MAX_PARA_CHANGE

/* WAVE6 */

/* #define SUPPORT_READ_BITSTREAM_IN_ENCODER */

/* WAVE517 */

#define SUPPORT_VAAPI_INTERFACE

/* JPEG */

#define JPU_API_VERSION		0x128

#define SUPPORT_PADDING_UNALIGNED_YUV
#define SUPPORT_STOP_CHECKING

#endif /* __CONFIG_H__ */

