/*******************************************************************************
@File
@Title          Common bridge header for rgxhwperf
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Declares common defines and structures used by both the client
                and server side of the bridge for rgxhwperf
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/

#ifndef COMMON_RGXHWPERF_BRIDGE_H
#define COMMON_RGXHWPERF_BRIDGE_H

#include <powervr/mem_types.h>

#include "img_defs.h"
#include "img_types.h"
#include "pvrsrv_error.h"

#include "rgx_hwperf.h"

#define PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST			0
#define PVRSRV_BRIDGE_RGXHWPERF_RGXCTRLHWPERF			PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+0
#define PVRSRV_BRIDGE_RGXHWPERF_RGXCONFIGUREHWPERFBLOCKS	PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+1
#define PVRSRV_BRIDGE_RGXHWPERF_RGXGETHWPERFBVNCFEATUREFLAGS	PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+2
#define PVRSRV_BRIDGE_RGXHWPERF_RGXCONTROLHWPERFBLOCKS		PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+3
#define PVRSRV_BRIDGE_RGXHWPERF_RGXACQUIREHWPERFFSETTINGS       PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+4
#define PVRSRV_BRIDGE_RGXHWPERF_RGXRELEASEHWPERFFSETTINGS       PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+5
#define PVRSRV_BRIDGE_RGXHWPERF_RGXGETHWPERFMASK                PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+6
#define PVRSRV_BRIDGE_RGXHWPERF_RGXGETHWPERFBLOCKCONFIGS        PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+7
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAGETSOCTIMER                 PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+8
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMINIT                     PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+9
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMMSSCONFIG                PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+10
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMGLOBALCONFIG             PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+11
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMINSTANCECONFIG           PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+12
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMWRAPPERCONFIG            PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+13
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMDUMPTRIGGER              PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+14
#define PVRSRV_BRIDGE_RGXHWPERF_MUSAPFMDEINIT                   PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+15
#define PVRSRV_BRIDGE_RGXHWPERF_CMD_LAST            		(PVRSRV_BRIDGE_RGXHWPERF_CMD_FIRST+15)

/*******************************************
            RGXCtrlHWPerf
 *******************************************/

/* Bridge in structure for RGXCtrlHWPerf */
typedef struct PVRSRV_BRIDGE_IN_RGXCTRLHWPERF_TAG
{
	IMG_UINT64 ui64Mask;
	IMG_BOOL bToggle;
	IMG_UINT32 ui32StreamId;
} __packed PVRSRV_BRIDGE_IN_RGXCTRLHWPERF;

/* Bridge out structure for RGXCtrlHWPerf */
typedef struct PVRSRV_BRIDGE_OUT_RGXCTRLHWPERF_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RGXCTRLHWPERF;

/*******************************************
	    PVRSRVGetHWPerfMask
 *******************************************/
typedef struct PVRSRV_BRIDGE_IN_RGXGETHWPERFMASK_TAG
{
	IMG_UINT32 ui32StreamId;
} __packed PVRSRV_BRIDGE_IN_RGXGETHWPERFMASK;

typedef struct PVRSRV_BRIDGE_OUT_RGXGETHWPERFMASK_TAG
{
	PVRSRV_ERROR eError;
	IMG_UINT64 ui64Mask;
} __packed PVRSRV_BRIDGE_OUT_RGXGETHWPERFMASK;

/*******************************************
            PVRSRVGetSOCTimer
 *******************************************/
typedef struct PVRSRV_BRIDGE_OUT_RGXGETSOCTIMER_TAG
{
	PVRSRV_ERROR eError;
	IMG_UINT64 ui64SocTimestamp;
	IMG_UINT64 ui64OSTimestamp;
} __packed PVRSRV_BRIDGE_OUT_RGXGETSOCTIMER;

/*******************************************
	    RGXAquireHWPerfSettings
 *******************************************/
typedef struct PVRSRV_BRIDGE_OUT_RGXACQUIREHWPERFSETTING_TAG
{
	PVRSRV_ERROR eError;
	IMG_HANDLE hPMR;
} __packed PVRSRV_BRIDGE_OUT_RGXACQUIREHWPERFSETTING;

/*******************************************
            RGXReleaseHWPerfSettings
 *******************************************/
typedef struct PVRSRV_BRIDGE_IN_RGXRELEASEHWPERFSETTING_TAG
{
	IMG_HANDLE hPMR;
} __packed PVRSRV_BRIDGE_IN_RGXRELEASEHWPERFSETTING;

typedef struct PVRSRV_BRIDGE_OUT_RGXRELEASEHWPERFSETTING_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RGXRELEASEHWPERFSETTING;

/*******************************************
            RGXConfigureHWPerfBlocks
 *******************************************/

/* Bridge in structure for RGXConfigureHWPerfBlocks */
typedef struct PVRSRV_BRIDGE_IN_RGXCONFIGUREHWPERFBLOCKS_TAG
{
	RGX_HWPERF_CONFIG_CNTBLK *psBlockConfigs;
	IMG_UINT32 ui32ArrayLen;
	IMG_UINT32 ui32CtrlWord;
} __packed PVRSRV_BRIDGE_IN_RGXCONFIGUREHWPERFBLOCKS;

/* Bridge out structure for RGXConfigureHWPerfBlocks */
typedef struct PVRSRV_BRIDGE_OUT_RGXCONFIGUREHWPERFBLOCKS_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RGXCONFIGUREHWPERFBLOCKS;

/*******************************************
	    RGXGetHWPerfBlockConfigs
 *******************************************/

/* Bridge in structure for RGXGetHWPerfBlockConfigs */
typedef struct PVRSRV_BRIDGE_IN_RGXGETHWPERFBLOCKCONFIGS_TAG
{
	RGX_HWPERF_CONFIG_CNTBLK *psBlockConfigs;
	IMG_UINT32 ui32ArrayLen;
} __packed PVRSRV_BRIDGE_IN_RGXGETHWPERFBLOCKCONFIGS;

/* Bridge out structure for RGXGetHWPerfBlockConfigs */
typedef struct PVRSRV_BRIDGE_OUT_RGXGETHWPERFBLOCKCONFIGS_TAG
{
	PVRSRV_ERROR eError;
	RGX_HWPERF_CONFIG_CNTBLK *psBlockConfigs;
	IMG_UINT32 ui32FoundLen;
} __packed PVRSRV_BRIDGE_OUT_RGXGETHWPERFBLOCKCONFIGS;

/*******************************************
            RGXGetHWPerfBvncFeatureFlags
 *******************************************/

/* Bridge in structure for RGXGetHWPerfBvncFeatureFlags */
typedef struct PVRSRV_BRIDGE_IN_RGXGETHWPERFBVNCFEATUREFLAGS_TAG
{
	IMG_UINT32 ui32EmptyStructPlaceholder;
} __packed PVRSRV_BRIDGE_IN_RGXGETHWPERFBVNCFEATUREFLAGS;

/* Bridge out structure for RGXGetHWPerfBvncFeatureFlags */
typedef struct PVRSRV_BRIDGE_OUT_RGXGETHWPERFBVNCFEATUREFLAGS_TAG
{
	RGX_HWPERF_BVNC sBVNC;
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RGXGETHWPERFBVNCFEATUREFLAGS;

/*******************************************
            RGXControlHWPerfBlocks
 *******************************************/

/* Bridge in structure for RGXControlHWPerfBlocks */
typedef struct PVRSRV_BRIDGE_IN_RGXCONTROLHWPERFBLOCKS_TAG
{
	IMG_UINT16 *pui16BlockIDs;
	IMG_BOOL bEnable;
	IMG_UINT32 ui32ArrayLen;
} __packed PVRSRV_BRIDGE_IN_RGXCONTROLHWPERFBLOCKS;

/* Bridge out structure for RGXControlHWPerfBlocks */
typedef struct PVRSRV_BRIDGE_OUT_RGXCONTROLHWPERFBLOCKS_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RGXCONTROLHWPERFBLOCKS;

/*******************************************
            MUSAPFMDeinit
 *******************************************/
/* Bridge in structure for MUSAPFMDeinit */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMDEINIT_TAG
{
	IMG_HANDLE hPMR;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMDEINIT;

/* Bridge out structure for MUSAPFMDeinit */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMDEINIT_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMDEINIT;

/*******************************************
            RGXHWPerfPFMDumpTrigger
 *******************************************/
/* Bridge in structure for MUSAPFMDumpTrig */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMDUMPTRIGGER_TAG
{
	RGX_HWPERF_PFM_DUMP_TRIGGER_CONFIG sDumpTrig;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMDUMPTRIGGER;

/* Bridge out structure for MUSAPFMDumpTrig */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMDUMPTRIGGER_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMDUMPTRIGGER;

/*******************************************
            MUSAPFMSetMSSConfig
 *******************************************/
/* Bridge in structure for MUSAPFMSetCounterMSSConfig */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMMSSCONFIG_TAG
{
        RGX_HWPERF_PFM_MSS_CONFIG sMSSConfig;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMMSSCONFIG;

/* Bridge out structure for MUSAPFMSetCounterMSSConfig */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMMSSCONFIG_TAG
{
        PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMMSSCONFIG;

/*******************************************
            MUSAPFMSetInstanceConfig
 *******************************************/
/* Bridge in structure for MUSAPFMSetCounterInstanceConfig */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMINSTANCECONFIG_TAG
{
	RGX_HWPERF_PFM_INSTANCE_CONFIG sInstanceConfig;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMINSTANCECONFIG;

/* Bridge out structure for MUSAPFMSetCounterInstanceConfig */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMINSTANCECONFIG_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMINSTANCECONFIG;

/*******************************************
            MUSAPFMSetWrapperConfig
 *******************************************/
/* Bridge in structure for MUSAPFMSetCounterInstanceConfig */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMWRAPPERCONFIG_TAG
{
	RGX_HWPERF_PFM_WRAPPER_CONFIG sWrapperConfig;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMWRAPPERCONFIG;

/* Bridge out structure for MUSAPFMSetCounterInstanceConfig */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMWRAPPERCONFIG_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMWRAPPERCONFIG;

/*******************************************
            MUSAPFMInit
 *******************************************/
/* Bridge in structure for MUSAPFMInit */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMINIT_TAG
{
	RGX_HWPERF_PFM_INIT_PARAM sInitParam;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMINIT;

/* Bridge out structure for MUSAPFMInit */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMINIT_TAG
{
	IMG_HANDLE hPMR;
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMINIT;

/*******************************************
            MUSAPFMSetGlobalCounterConfig
 *******************************************/
/* Bridge in structure for MUSAPFMSetGlobalCounterConfig */
typedef struct PVRSRV_BRIDGE_IN_MUSAPFMGLOBALCONFIG_TAG
{
	RGX_HWPERF_PFM_GLOBAL_CONFIG sGlobalConfig;
} __packed PVRSRV_BRIDGE_IN_MUSAPFMGLOBALCONFIG;

/* Bridge out structure for MUSAPFMSetGlobalCounterConfig */
typedef struct PVRSRV_BRIDGE_OUT_MUSAPFMGLOBALCONFIG_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_MUSAPFMGLOBALCONFIG;
#endif /* COMMON_RGXHWPERF_BRIDGE_H */
