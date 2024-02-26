/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTJPU_TYPES_H__
#define __MTJPU_TYPES_H__

#include "linux-types.h"

typedef uint8_t    Uint8;
typedef uint16_t   Uint16;
typedef uint32_t   Uint32;
typedef uint64_t   Uint64;
typedef int8_t     Int8;
typedef int16_t    Int16;
typedef int32_t    Int32;
typedef int64_t    Int64;
typedef int32_t    BOOL;
typedef uint64_t   PhysicalAddress;
typedef unsigned char BYTE;

#ifndef TRUE
#define TRUE                 1
#endif

#ifndef FALSE
#define FALSE                0
#endif

#endif /* __MTJPU_TYPES_H__ */
