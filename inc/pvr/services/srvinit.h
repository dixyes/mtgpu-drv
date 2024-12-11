/*************************************************************************/ /*!
@File
@Title          Initialisation server internal header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Defines the connections between the various parts of the
                initialisation server.
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
*/ /**************************************************************************/

#ifndef SRVINIT_H
#define SRVINIT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "img_defs.h"
#include "pvrsrv_error.h"
#include "device_connection.h"
#include "device.h"

/*
 * Container for all the apphints
 */
typedef struct _RGX_SRVINIT_APPHINTS_
{
	IMG_UINT32 ui32DriverMode;
	IMG_BOOL   bEnableSignatureChecks;
	IMG_UINT32 ui32SignatureChecksBufSize;

	IMG_BOOL   bAssertOnOutOfMem;
	IMG_BOOL   bAssertOnHWRTrigger;
#if defined(SUPPORT_VALIDATION)
	IMG_UINT32 ui32RenderKillingCtl;
	IMG_UINT32 ui32CDMTDMKillingCtl;
	IMG_BOOL   bValidateIrq;
	IMG_BOOL   bValidateSOCUSCTimer;
	IMG_UINT32 ui32AvailablePowUnitsMask;
	IMG_UINT32 ui32AvailableRACMask;
	IMG_BOOL   bInjectPowUnitsStateMaskChange;
	IMG_BOOL   bEnablePowUnitsStateMaskChange;
	IMG_UINT32 ui32FBCDCVersionOverride;
	IMG_UINT32 aui32TPUTrilinearFracMask[RGXFWIF_TPU_DM_LAST];
	IMG_UINT32 aui32USRMNumRegions[RGXFWIF_USRM_DM_LAST];
	IMG_UINT64 aui64UVBRMNumRegions[RGXFWIF_UVBRM_DM_LAST];
#endif
	IMG_BOOL   bCheckMlist;
	IMG_BOOL   bEnableCalcDMUtil;
	IMG_BOOL   bDisableClockGating;
	IMG_BOOL   bDisableDMOverlap;
	IMG_BOOL   bDisableFEDLogging;
	IMG_BOOL   bDisablePDP;
	IMG_BOOL   bDisableCE;
	IMG_BOOL   bEnableDMKillRand;
	IMG_BOOL   bEnableRandomCsw;
	IMG_BOOL   bEnableSoftResetCsw;
	IMG_BOOL   bFilteringMode;
	IMG_BOOL   bHWPerfDisableCounterFilter;
	IMG_BOOL   bZeroFreelist;
	IMG_UINT32 ui32EnableFWContextSwitch;
	IMG_UINT32 ui32FWContextSwitchProfile;
	IMG_UINT32 ui32ISPSchedulingLatencyMode;
	IMG_UINT32 ui32HWPerfFWBufSize;
	IMG_UINT32 ui32HWPerfHostBufSize;
	IMG_UINT32 ui32HWPerfFilter0;
	IMG_UINT32 ui32HWPerfFilter1;
	IMG_UINT32 ui32HWPerfHostFilter;
	IMG_UINT32 ui32TimeCorrClock;
	IMG_UINT32 ui32HWRDebugDumpLimit;
	IMG_UINT32 ui32JonesDisableMask;
	IMG_UINT32 ui32LogType;
	IMG_UINT32 ui32MetaLogGroup;
	IMG_UINT32 ui32FecLogGroup;
	IMG_UINT32 ui32TruncateMode;
	IMG_UINT32 ui32KCCBSizeLog2;
	IMG_UINT32 ui32CDMArbitrationMode;
	FW_PERF_CONF eFirmwarePerf;
	RGX_ACTIVEPM_CONF eRGXActivePMConf;
	RGX_RD_POWER_ISLAND_CONF eRGXRDPowerIslandConf;

	IMG_BOOL   bEnableTrustedDeviceAceConfig;
	IMG_UINT32 ui32FWContextSwitchCrossDM;
	IMG_UINT32 ui32MemoryOptimization;
#if defined(SUPPORT_PHYSMEM_TEST) && !defined(INTEGRITY_OS) && !defined(__QNXNTO__)
	IMG_UINT32 ui32PhysMemTestPasses;
#endif
} RGX_SRVINIT_APPHINTS;

#if defined(SUPPORT_RGX)
PVRSRV_ERROR RGXInit(PVRSRV_DEVICE_NODE *psDeviceNode);
#endif

#if defined(__cplusplus)
}
#endif
#endif /* SRVINIT_H */

/******************************************************************************
 End of file (srvinit.h)
******************************************************************************/
