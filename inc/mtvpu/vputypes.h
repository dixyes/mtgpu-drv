//-----------------------------------------------------------------------------
// COPYRIGHT (C) 2020   CHIPS&MEDIA INC. ALL RIGHTS RESERVED
// 
// This file is distributed under BSD 3 clause and LGPL2.1 (dual license)
// SPDX License Identifier: BSD-3-Clause
// SPDX License Identifier: LGPL-2.1-only
// 
// The entire notice above must be reproduced on all authorized copies.
// 
// Description  : 
//-----------------------------------------------------------------------------

#ifndef _VPU_TYPES_H_
#define _VPU_TYPES_H_

#include "vpuconfig.h"

#if defined(__KERNEL__)
  #include "linux-types.h"
#endif

#if defined(_MSC_VER)
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
/*
typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;
typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char        int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned char      uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef long long          intmax_t;
typedef unsigned long long uintmax_t;
*/
#endif

/**
* @brief    This type is an 8-bit unsigned integral type, which is used for declaring pixel data.
*/
typedef uint8_t         Uint8;

/**
* @brief    This type is a 32-bit unsigned integral type, which is used for declaring variables with wide ranges and no signs such as size of buffer.
*/
typedef uint32_t        Uint32;

/**
* @brief    This type is a 16-bit unsigned integral type.
*/
typedef uint16_t        Uint16;

/**
* @brief    This type is an 8-bit signed integral type.
*/
typedef int8_t          Int8;

/**
* @brief    This type is a 32-bit signed integral type.
*/
typedef int32_t         Int32;

/**
* @brief    This type is a 16-bit signed integral type.
*/
typedef int16_t         Int16;
#if defined(_MSC_VER)
typedef unsigned __int64 Uint64;
typedef __int64          Int64;
#else
typedef uint64_t        Uint64;
typedef int64_t         Int64;
#endif
#ifndef PhysicalAddress
/**
* @brief    This is a type for representing physical addresses which are recognizable by VPU. 
In general, VPU hardware does not know about virtual address space 
which is set and handled by host processor. All these virtual addresses are 
translated into physical addresses by Memory Management Unit. 
All data buffer addresses such as stream buffer and frame buffer should be given to
VPU as an address on physical address space.
*/
typedef Uint64 PhysicalAddress;

#endif

#ifndef BYTE
/**
* @brief This type is an 8-bit unsigned integral type.
*/
typedef unsigned char   BYTE;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE            1
#endif /* TRUE */
#ifndef FALSE
#define FALSE           0
#endif /* FALSE */
#ifndef NULL
#define NULL	0
#endif

#ifdef __GNUC__
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          \
    /*lint -save -e527 -e530 */ \
{ \
    (P) = (P); \
} \
    /*lint -restore */
#endif

#define UNREFERENCED_FUNCTION __attribute__ ((unused))
#else
#define UNREFERENCED_FUNCTION
#endif

#endif	/* _VPU_TYPES_H_ */
 
