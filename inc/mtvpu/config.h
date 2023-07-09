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
#elif defined(unix) || defined(__unix)
#   define PLATFORM_QNX
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

#define API_VERSION_MAJOR       5
#define API_VERSION_MINOR       6
#define API_VERSION_PATCH       4
#define API_VERSION             ((API_VERSION_MAJOR<<16) | (API_VERSION_MINOR<<8) | API_VERSION_PATCH)

#if defined(linux) || defined(__linux) || defined(ANDROID) || defined(CNM_FPGA_HAPS_INTERFACE) || defined(CNM_SIM_PLATFORM)
#define SUPPORT_MULTI_INST_INTR
#endif
#if defined(linux) || defined(__linux) || defined(ANDROID)
#define SUPPORT_INTERRUPT
#endif

/* do not define BIT_CODE_FILE_PATH in case of multiple product support.
 * because wave410 and coda980 has different firmware binary format.
 */

#define CODA960_BITCODE_PATH                "coda960.out"
#define CODA980_BITCODE_PATH                "coda980.out"
#define WAVE627_BITCODE_PATH                "seurat.bin"
#define WAVE637_BITCODE_PATH                "seurat.bin"
#define WAVE521C_DUAL_BITCODE_PATH          "chagall_dual.bin"
#define WAVE521E1_BITCODE_PATH              "degas_e1.bin"
#define WAVE537_BITCODE_PATH                "vincent_dual.bin"
#define WAVE521_BITCODE_PATH                "chagall.bin"
#define WAVE517_BITCODE_PATH                "vincent.bin"
#define WAVE511_BITCODE_PATH                "chagall.bin"

/* CODA980 */
#define CODA980

#define CLIP_PIC_DELTA_QP

#define RC_MIN_MAX_PARA_CHANGE

/* WAVE6 */

/* #define SUPPORT_READ_BITSTREAM_IN_ENCODER */

/* WAVE517 */

#define SUPPORT_VAAPI_INTERFACE

#endif /* __CONFIG_H__ */

