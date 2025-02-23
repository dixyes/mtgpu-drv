/*************************************************************************/ /*!
@File
@Title          RGX HWPerf and Debug Types and Defines Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Common data types definitions for hardware performance API
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
#ifndef RGX_HWPERF_H_
#define RGX_HWPERF_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* These structures are used on both GPU and CPU and must be a size that is a
 * multiple of 64 bits, 8 bytes to allow the FW to write 8 byte quantities at
 * 8 byte aligned addresses. RGX_FW_STRUCT_*_ASSERT() is used to check this.
 */

/******************************************************************************
 * Includes and Defines
 *****************************************************************************/

#include "img_types.h"
#include "img_defs.h"

#include "rgx_common.h"
#include "rgx_hwperf_common.h"
#include "pvrsrv_tlcommon.h"
#include "pvrsrv_sync_km.h"


#if defined(RGX_BVNC_CORE_KM_HEADER) && defined(RGX_BNC_CONFIG_KM_HEADER)
/* HWPerf interface assumption checks */
static_assert(RGX_FEATURE_NUM_CLUSTERS <= 16U,
			  "Cluster count too large for HWPerf protocol definition");
#endif

/*! Perf counter control words */
#define RGX_HWPERF_CTRL_NOP						(0)			/*!< only update HW counters */
#define RGX_HWPERF_CTRL_STATE_UPDATE_EN			(1U << 31)	/*!< persistent state update; see other flags below */
#define RGX_HWPERF_CTRL_GEOM_FULLRANGE			(1U)		/*!< selectable geom and 3D counters are full range */
#define RGX_HWPERF_CTRL_COMP_FULLRANGE			(2U)		/*!< selectable compute counters are full range */
#define RGX_HWPERF_CTRL_TDM_FULLRANGE			(4U)		/*!< selectable TDM counters are full range */


/******************************************************************************
 * Data Stream Common Types
 *****************************************************************************/

/*! All the Data Masters HWPerf is aware of. When a new DM is added to this
 * list, it should be appended at the end to maintain backward compatibility
 * of HWPerf data.
 */
typedef enum {

	RGX_HWPERF_DM_GP,
	RGX_HWPERF_DM_2D,
	RGX_HWPERF_DM_TA,
	RGX_HWPERF_DM_3D,
	RGX_HWPERF_DM_CDM,
	RGX_HWPERF_DM_RTU,
	RGX_HWPERF_DM_SHG,
	RGX_HWPERF_DM_TDM,

	RGX_HWPERF_DM_LAST,

	RGX_HWPERF_DM_INVALID = 0x1FFFFFFF
} RGX_HWPERF_DM;

/*! Enum containing bit position for 32bit feature flags used in hwperf and api */
typedef enum {
	RGX_HWPERF_FEATURE_PERFBUS_FLAG                = 0x0001,
	RGX_HWPERF_FEATURE_S7_TOP_INFRASTRUCTURE_FLAG  = 0x0002,
	RGX_HWPERF_FEATURE_XT_TOP_INFRASTRUCTURE_FLAG  = 0x0004,
	RGX_HWPERF_FEATURE_PERF_COUNTER_BATCH_FLAG     = 0x0008,
	RGX_HWPERF_FEATURE_ROGUEXE_FLAG                = 0x0010,
	RGX_HWPERF_FEATURE_DUST_POWER_ISLAND_S7_FLAG   = 0x0020,
	RGX_HWPERF_FEATURE_PBE2_IN_XE_FLAG             = 0x0040,
	RGX_HWPERF_FEATURE_WORKLOAD_ESTIMATION         = 0x0080,
	RGX_HWPERF_FEATURE_MULTICORE_FLAG              = 0x0100,
	RGX_HWPERF_FEATURE_RAYTRACING_FLAG             = 0x0200,
	RGX_HWPERF_FEATURE_CXT_TOP_INFRASTRUCTURE_FLAG = 0x0400,
	RGX_HWPERF_FEATURE_VOLCANIC_FLAG               = 0x0800,
	RGX_HWPERF_FEATURE_ROGUE_FLAG                  = 0x1000,
	RGX_HWPERF_FEATURE_OCEANIC_FLAG                = 0x2000
} RGX_HWPERF_FEATURE_FLAGS;

/*! This structure holds the data of a firmware packet. */
typedef struct
{
	RGX_HWPERF_DM eDM;				/*!< DataMaster identifier, see RGX_HWPERF_DM */
	IMG_UINT32 ui32TxtActCyc;		/*!< Meta TXTACTCYC register value */
	IMG_UINT32 ui32FWPerfCount0;	/*!< Meta/MIPS PERF_COUNT0 register */
	IMG_UINT32 ui32FWPerfCount1;	/*!< Meta/MIPS PERF_COUNT1 register */
	IMG_UINT64 ui64SoCTimestamp;	/*!< SoC timestamp */
	IMG_UINT32 ui32TimeCorrIndex;	/*!< Internal field */
	IMG_UINT32 ui32Padding;			/*!< Reserved */
} RGX_HWPERF_FW_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_FW_DATA);

typedef enum
{
	RGX_HWPERF_FW_DBG_SECTION_A_START,
	RGX_HWPERF_FW_DBG_SECTION_A_FINISH,
	RGX_HWPERF_FW_DBG_SECTION_B_START,
	RGX_HWPERF_FW_DBG_SECTION_B_FINISH,
	RGX_HWPERF_FW_DBG_SECTION_C_START,
	RGX_HWPERF_FW_DBG_SECTION_C_FINISH,
	RGX_HWPERF_FW_DBG_SECTION_D_START,
	RGX_HWPERF_FW_DBG_SECTION_D_FINISH,
	RGX_HWPERF_FW_DBG_SECTION_E_START,
	RGX_HWPERF_FW_DBG_SECTION_E_FINISH,
} RGX_HWPERF_FW_DBG_SECTION;

typedef struct
{
	IMG_UINT64                ui64SoCTimestamp;  /*!< SoC timestamp */
	IMG_UINT32                ui32TimeCorrIndex; /*!< Index to the time correlation when the packet was generated */
	RGX_HWPERF_DM             eDM;               /*!< Data Master number, see RGX_HWPERF_DM */
	RGX_HWPERF_FW_DBG_SECTION eSection;          /*!< Code section to profile distinguishable by A, B, C, D, E */
	IMG_UINT32                ui32DMContext;     /*!< GPU Data Master (FW) Context */
	IMG_UINT64                ui64UserData1;     /*!< Customized data section */
	IMG_UINT64                ui64UserData2;
	IMG_UINT64                ui64UserData3;
} RGX_HWPERF_FW_DBG_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_FW_DBG_DATA);

/*! This structure holds the data of a hardware packet, including counters. */
typedef struct
{
	IMG_UINT32 ui32DMCyc;         /*!< DataMaster cycle count register, 0 if none */
	IMG_UINT32 ui32FrameNum;      /*!< Frame number, undefined on some DataMasters */
	IMG_UINT32 ui32PID;           /*!< Process identifier */
	IMG_UINT32 ui32DMContext;     /*!< GPU Data Master (FW) Context */
	IMG_UINT32 ui32WorkTarget;    /*!< RenderTarget for a TA,3D; Frame context for RTU, 0x0 otherwise */
	IMG_UINT32 ui32ExtJobRef;     /*!< Client driver context job reference used for tracking/debugging */
	IMG_UINT32 ui32IntJobRef;     /*!< RGX Data master context job reference used for tracking/debugging */
	IMG_UINT64 ui64SoCTimestamp;  /*!< SoC timestamp */
	IMG_UINT32 ui32TimeCorrIndex; /*!< Index to the time correlation at the time the packet was generated */
	IMG_UINT32 ui32BlkInfo;       /*!< <31..16> NumBlocks <15..0> Counter block stream offset */
	IMG_UINT32 ui32WorkCtx;       /*!< Work context: Render Context for TA/3D; RayTracing Context for RTU/SHG; 0x0 otherwise */
	IMG_UINT32 ui32CtxPriority;   /*!< Context priority */
	IMG_UINT32 ui32GPUIdMask;     /*!< GPU IDs active within this event */
	IMG_UINT64 ui64PFMNormDumpCntr; /*!< Record the number of dumps in PFM normal mode*/
	IMG_UINT32 ui32KickInfo;      /*!< <31..8> Reserved <7..0> GPU Pipeline DM kick ID, 0 if not using Pipeline DMs */
	IMG_UINT32 ui32Padding;       /*!< Reserved. To ensure correct alignment */
	IMG_UINT32 aui32CountBlksStream[RGX_HWPERF_ZERO_OR_MORE_ELEMENTS]; /*!< Optional variable length Counter data */
	IMG_UINT32 ui32Padding2;      /*!< Reserved. To ensure correct alignment (not written in the packet) */
} RGX_HWPERF_HW_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_HW_DATA);
RGX_FW_STRUCT_OFFSET_ASSERT(RGX_HWPERF_HW_DATA, aui32CountBlksStream);

/*! Mask for use with the aui32CountBlksStream field when decoding the
 * counter block ID and mask word. */
#define RGX_HWPERF_CNTBLK_ID_MASK	0xFFFF0000U
#define RGX_HWPERF_CNTBLK_ID_SHIFT	16U

/*! MAX value used in server handling of counter config arrays */
#define RGX_CNTBLK_COUNTERS_MAX PVRSRV_HWPERF_COUNTERS_PERBLK


/*! Obtains the counter block ID word from an aui32CountBlksStream field.
 * The word combines Control bits (15-12), GPU-Id (11-8), Group (7-4), Unit
 * within group (3-0) */
#define RGX_HWPERF_GET_CNTBLK_IDW(_word)           ((IMG_UINT16)(((_word)&RGX_HWPERF_CNTBLK_ID_MASK)>>RGX_HWPERF_CNTBLK_ID_SHIFT))

/*! Obtains the counter block ID from the supplied RGX_HWPERF_HW_DATA address
 * and stream index. May be used in decoding the counter block stream words of
 * a RGX_HWPERF_HW_DATA structure. */
#define RGX_HWPERF_GET_CNTBLK_ID(_data_addr, _idx) RGX_HWPERF_GET_CNTBLK_IDW((_data_addr)->aui32CountBlksStream[(_idx)])

/*! Obtains the GPU ID from the supplied RGX_HWPERF_HW_DATA CNTBLK_IDW */
#define RGX_HWPERF_GET_CNTBLK_GPUW(_word)          ((IMG_UINT16)(((_word)&RGX_CNTBLK_ID_MC_GPU_MASK)>>RGX_CNTBLK_ID_MC_GPU_SHIFT))

#define RGX_HWPERF_GET_CNT_MASKW(_word)           ((IMG_UINT16)((_word)&(~RGX_HWPERF_CNTBLK_ID_MASK)))

/*! Obtains the counter mask from the supplied RGX_HWPERF_HW_DATA address
 * and stream index. May be used in decoding the counter block stream words
 * of a RGX_HWPERF_HW_DATA structure. */
#define RGX_HWPERF_GET_CNT_MASK(_data_addr, _idx) RGX_HWPERF_GET_CNT_MASKW((_data_addr)->aui32CountBlksStream[(_idx)])

/*! Context switch packet event */
typedef struct
{
	RGX_HWPERF_DM	eDM;					/*!< DataMaster identifier, see RGX_HWPERF_DM */
	IMG_UINT32		ui32DMContext;			/*!< GPU Data Master (FW) Context */
	IMG_UINT32		ui32FrameNum;			/*!< Client Frame number (TA, 3D only) */
	IMG_UINT32		ui32TxtActCyc;			/*!< Meta TXTACTCYC register value */
	IMG_UINT32		ui32PerfCycle;			/*!< Cycle count. Used to measure HW context store latency */
	IMG_UINT32		ui32PerfPhase;			/*!< Phase. Used to determine geometry content */
	IMG_UINT32		ui32Padding[2];			/*!< Padding to 8 DWords */
} RGX_HWPERF_CSW_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_CSW_DATA);

/*! Enumeration of clocks supporting this event */
typedef enum
{
	RGX_HWPERF_CLKS_CHG_INVALID = 0,

	RGX_HWPERF_CLKS_CHG_NAME_CORE = 1,

	RGX_HWPERF_CLKS_CHG_LAST,
} RGX_HWPERF_CLKS_CHG_NAME;

/*! This structure holds the data of a clocks change packet. */
typedef struct
{
	IMG_UINT64                ui64NewClockSpeed;         /*!< New Clock Speed (in Hz) */
	RGX_HWPERF_CLKS_CHG_NAME  eClockName;                /*!< Clock name */
	IMG_UINT32                ui32CalibratedClockSpeed;  /*!< Calibrated new GPU clock speed (in Hz) */
	IMG_UINT64                ui64OSTimeStamp;           /*!< OSTimeStamp sampled by the host */
	IMG_UINT64                ui64CRTimeStamp;           /*!< CRTimeStamp sampled by the host and
	                                                          correlated to OSTimeStamp */
} RGX_HWPERF_CLKS_CHG_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_CLKS_CHG_DATA);

/*! Enumeration of GPU utilisation states supported by this event */
typedef IMG_UINT32 RGX_HWPERF_GPU_STATE;

/*! This structure holds the data of a GPU utilisation state change packet. */
typedef struct
{
	RGX_HWPERF_GPU_STATE	eState;		/*!< New GPU utilisation state */
	IMG_UINT32				uiUnused1;	/*!< Padding */
	IMG_UINT32				uiUnused2;	/*!< Padding */
	IMG_UINT32				uiUnused3;	/*!< Padding */
} RGX_HWPERF_GPU_STATE_CHG_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_GPU_STATE_CHG_DATA);


/*! Signature pattern 'HPE1' found in the first word of a PWR_EST packet data */
#define HWPERF_PWR_EST_V1_SIG	0x48504531

/*! Macros to obtain a component field from a counter ID word */
#define RGX_HWPERF_GET_PWR_EST_HIGH_FLAG(_word) (((_word)&0x80000000)>>31)
#define RGX_HWPERF_GET_PWR_EST_GPUID(_word)     (((_word)&0x70000000)>>28)
/*!< Obtains the GPU ID from a counter ID word */
#define RGX_HWPERF_GET_PWR_EST_UNIT(_word)      (((_word)&0x0F000000)>>24)
#define RGX_HWPERF_GET_PWR_EST_NUMBER(_word)    ((_word)&0x0000FFFF)

#define RGX_HWPERF_PWR_EST_HIGH_OFFSET		(31)
#define RGX_HWPERF_PWR_EST_GPUID_OFFSET		(28)
#define RGX_HWPERF_PWR_EST_GPUID_MASK		(0x7U)
#define RGX_HWPERF_PWR_EST_UNIT_OFFSET		(24)
#define RGX_HWPERF_PWR_EST_UNIT_MASK		(0xFU)
#define RGX_HWPERF_PWR_EST_VALUE_MASK		(0xFFFFU)

/*! This macro constructs a counter ID for a power estimate data stream from
 * the component parts of: high word flag, unit id, GPU id, counter number */
#define RGX_HWPERF_MAKE_PWR_EST_COUNTERID(_high, _unit, _core, _number) \
			((IMG_UINT32)(((IMG_UINT32)((IMG_UINT32)(_high)&0x1U)<<RGX_HWPERF_PWR_EST_HIGH_OFFSET) | \
			((IMG_UINT32) ((IMG_UINT32)(_unit)&RGX_HWPERF_PWR_EST_UNIT_MASK)<<RGX_HWPERF_PWR_EST_UNIT_OFFSET) | \
			((IMG_UINT32) ((IMG_UINT32)(_core)&RGX_HWPERF_PWR_EST_GPUID_MASK)<<RGX_HWPERF_PWR_EST_GPUID_OFFSET) | \
			                           ((_number)&RGX_HWPERF_PWR_EST_VALUE_MASK)))

/*! This structure holds the data for a power estimate packet. */
typedef struct
{
	IMG_UINT32  ui32StreamVersion;  /*!< Version word, HWPERF_PWR_EST_V1_SIG */
	IMG_UINT32  ui32StreamSize;     /*!< Size of array in bytes of stream data
	                                     held in the aui32StreamData member */
	IMG_UINT32  aui32StreamData[RGX_HWPERF_ONE_OR_MORE_ELEMENTS]; /*!< Counter data */
	IMG_UINT32  ui32Padding; /*!< Reserved. To ensure correct alignment */
} RGX_HWPERF_PWR_EST_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_PWR_EST_DATA);

/*! Enumeration of the kinds of power change events that can occur */
typedef enum
{
	RGX_HWPERF_PWR_UNDEFINED    = 0,
	RGX_HWPERF_PWR_ON           = 1, /*!< Whole device powered on */
	RGX_HWPERF_PWR_OFF          = 2, /*!< Whole device powered off */
	RGX_HWPERF_PWR_UP           = 3, /*!< Power turned on to a HW domain */
	RGX_HWPERF_PWR_DOWN         = 4, /*!< Power turned off to a HW domain */
	RGX_HWPERF_PWR_SAFETY_RESET = 5, /*!< Resetting the GPU HW units for safety reasons */
	RGX_HWPERF_PWR_PHR_FULL     = 6, /*!< Periodic HW full GPU Reset */

	RGX_HWPERF_PWR_LAST,
} RGX_HWPERF_PWR;

/*! This structure holds the data of a power packet. */
typedef struct
{
	RGX_HWPERF_PWR eChange;                  /*!< Defines the type of power change */
	IMG_UINT32     ui32Domains;              /*!< HW Domains affected */
	IMG_UINT64     ui64OSTimeStamp;          /*!< OSTimeStamp sampled by the host */
	IMG_UINT64     ui64CRTimeStamp;          /*!< CRTimeStamp sampled by the host and
	                                              correlated to OSTimeStamp */
	IMG_UINT32     ui32CalibratedClockSpeed; /*!< GPU clock speed (in Hz) at the time
	                                              the two timers were correlated */
	IMG_UINT32     ui32Unused1;              /*!< Padding */
} RGX_HWPERF_PWR_CHG_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_PWR_CHG_DATA);


/*
 * PDVFS, GPU clock frequency changes and workload estimation profiling
 * data.
 */
/*! DVFS and work estimation events. */
typedef enum
{
	RGX_HWPERF_DVFS_EV_INVALID,                 /*!< Invalid value. */
	RGX_HWPERF_DVFS_EV_PROACTIVE_EST_START,     /*!< Proactive DVFS estimate start */
	RGX_HWPERF_DVFS_EV_PROACTIVE_EST_FINISHED,  /*!< Proactive DVFS estimate finished */
	RGX_HWPERF_DVFS_EV_REACTIVE_EST_START,      /*!< Reactive DVFS estimate start */
	RGX_HWPERF_DVFS_EV_REACTIVE_EST_FINISHED,   /*!< Reactive DVFS estimate finished */
	/* workload estimation */
	RGX_HWPERF_DVFS_EV_WORK_EST_START,          /*!< Workload estimation start */
	RGX_HWPERF_DVFS_EV_WORK_EST_FINISHED,       /*!< Workload estimation finished */
	RGX_HWPERF_DVFS_EV_FREQ_CHG,                /*!< DVFS OPP/clock frequency change */

	RGX_HWPERF_DVFS_EV_LAST               /*!< Number of element. */
} RGX_HWPERF_DVFS_EV;

/*! Enumeration of DVFS transitions that can occur */
typedef enum
{
	RGX_HWPERF_DVFS_OPP_NONE        = 0x0,  /*!< No OPP change, already operating at required freq */
#if defined(SUPPORT_PDVFS_IDLE)
	RGX_HWPERF_DVFS_OPP_IDLE        = 0x1,  /*!< GPU is idle, defer the OPP change */
#endif
	/* 0x2 to 0xF reserved */
	RGX_HWPERF_DVFS_OPP_UPDATE      = 0x10, /*!< OPP change, new point is encoded in bits [3:0] */
	RGX_HWPERF_DVFS_OPP_LAST        = 0x20,
} RGX_HWPERF_DVFS_OPP;

typedef union
{
	/*! This structure holds the data of a proactive DVFS calculation packet. */
	struct
	{
		IMG_UINT64     ui64DeadlineInus;         /*!< Next deadline in microseconds */
		IMG_UINT32     ui32Frequency;            /*!< Required freq to meet deadline at 90% utilisation */
		IMG_UINT32     ui32WorkloadCycles;       /*!< Current workload estimate in cycles */
		IMG_UINT32     ui32TxtActCyc;            /*!< Meta TXTACTCYC register value */
	} sProDVFSCalc;

	/*! This structure holds the data of a reactive DVFS calculation packet. */
	struct
	{
		IMG_UINT32     ui32Frequency;            /*!< Required freq to achieve average 90% utilisation */
		IMG_UINT32     ui32Utilisation;          /*!< GPU utilisation since last update */
		IMG_UINT32     ui32TxtActCyc;            /*!< Meta TXTACTCYC register value */
	} sDVFSCalc;

	/*! This structure holds the data of a work estimation packet. */
	struct
	{
		IMG_UINT32     ui32CyclesPrediction;     /*!< Predicted cycle count for this workload */
		IMG_UINT32     ui32CyclesTaken;          /*!< Actual cycle count for this workload */
		RGXFWIF_DM     eDM;                      /*!< Target DM */
		IMG_UINT32     ui32ReturnDataIndex;      /*!< Index into workload estimation table */
		IMG_UINT32     ui32TxtActCyc;            /*!< Meta TXTACTCYC register value */
	} sWorkEst;

	/*! This structure holds the data of an OPP clock frequency transition packet. */
	struct
	{
		IMG_UINT32     ui32OPPData;              /*!< OPP transition */
	} sOPP;

} RGX_HWPERF_DVFS_DETAIL;

/*! DVFS sub-event data structure */
typedef struct {
	RGX_HWPERF_DVFS_EV      eEventType;          /*!< DVFS sub-event type */
	RGX_HWPERF_DVFS_DETAIL  uData;               /*!< DVFS sub-event data */
} RGX_HWPERF_DVFS_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_DVFS_DATA);


/*! Firmware Activity event. */
typedef enum
{
	RGX_HWPERF_FWACT_EV_INVALID,            /*!< Invalid value. */
	RGX_HWPERF_FWACT_EV_REGS_SET,           /*!< Registers set. */
	RGX_HWPERF_FWACT_EV_HWR_DETECTED,       /*!< HWR detected. */
	RGX_HWPERF_FWACT_EV_HWR_RESET_REQUIRED, /*!< Reset required. */
	RGX_HWPERF_FWACT_EV_HWR_RECOVERED,      /*!< HWR recovered. */
	RGX_HWPERF_FWACT_EV_HWR_FREELIST_READY, /*!< Freelist ready. */
	RGX_HWPERF_FWACT_EV_FEATURES,           /*!< Features present */
	RGX_HWPERF_FWACT_EV_FILTER_SET,         /*!< Event filter set. */

	RGX_HWPERF_FWACT_EV_LAST                /*!< Number of element. */
} RGX_HWPERF_FWACT_EV;

/*! Cause of the HWR event. */
typedef enum
{
	RGX_HWPERF_HWR_REASON_INVALID,              /*!< Invalid value. */
	RGX_HWPERF_HWR_REASON_LOCKUP,               /*!< Lockup. */
	RGX_HWPERF_HWR_REASON_PAGEFAULT,            /*!< Page fault. */
	RGX_HWPERF_HWR_REASON_POLLFAIL,             /*!< Poll fail. */
	RGX_HWPERF_HWR_REASON_DEADLINE_OVERRUN,     /*!< Deadline overrun. */
	RGX_HWPERF_HWR_REASON_CSW_DEADLINE_OVERRUN, /*!< Hard Context Switch deadline overrun. */

	RGX_HWPERF_HWR_REASON_LAST                  /*!< Number of elements. */
} RGX_HWPERF_HWR_REASON;


/* Fixed size for BVNC string so it does not alter packet data format
 * Check it is large enough against official BVNC string length maximum
 */
#define RGX_HWPERF_MAX_BVNC_LEN (24)
static_assert((RGX_HWPERF_MAX_BVNC_LEN >= RGX_BVNC_STR_SIZE_MAX),
			  "Space inside HWPerf packet data for BVNC string insufficient");

#define RGX_HWPERF_MAX_BVNC_BLOCK_LEN (20U)

/*! BVNC Features */
typedef struct
{
	/*! Counter block ID, see RGX_HWPERF_CNTBLK_ID */
	IMG_UINT16 ui16BlockID;

	/*! Number of counters in this block type */
	IMG_UINT16 ui16NumCounters;

	/*! Number of blocks of this type */
	IMG_UINT16 ui16NumBlocks;

	/*! Reserved for future use */
	IMG_UINT16 ui16Reserved;
} RGX_HWPERF_BVNC_BLOCK;

/*! BVNC Features */
typedef struct
{
	IMG_CHAR aszBvncString[RGX_HWPERF_MAX_BVNC_LEN]; /*!< BVNC string */
	IMG_UINT32 ui32BvncKmFeatureFlags;               /*!< See RGX_HWPERF_FEATURE_FLAGS */
	IMG_UINT16 ui16BvncBlocks;                       /*!< Number of blocks described in aBvncBlocks */
	IMG_UINT16 ui16BvncGPUCores;                     /*!< Number of GPU cores present */
	RGX_HWPERF_BVNC_BLOCK aBvncBlocks[RGX_HWPERF_MAX_BVNC_BLOCK_LEN];  /*!< Supported Performance Blocks for BVNC. See RGX_HWPERF_BVNC_BLOCK */
} RGX_HWPERF_BVNC;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_BVNC);

/*! Performance Counter Configuration data element. */
typedef struct
{
	IMG_UINT32 ui32BlockID;               /*!< Counter Block ID. See RGX_HWPERF_CNTBLK_ID */
	IMG_UINT32 ui32NumCounters;           /*!< Number of counters configured */
	IMG_UINT32 ui32CounterVals[RGX_CNTBLK_COUNTERS_MAX];  /*!< Counters configured (ui32NumCounters worth of entries) */
} RGX_HWPERF_COUNTER_CFG_DATA_EL;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_COUNTER_CFG_DATA_EL);

/*! Performance Counter Configuration data. */
typedef struct
{
	IMG_UINT32 ui32EnabledBlocks;          /*!< Number of Enabled Blocks. */
	RGX_HWPERF_COUNTER_CFG_DATA_EL uData;  /*!< Start of variable length data. See RGX_HWPERF_COUNTER_CFG_DATA_EL */
	IMG_UINT32 ui32Padding;                /*!< reserved */
} RGX_HWPERF_COUNTER_CFG;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_COUNTER_CFG);

/*! Sub-event's data. */
typedef union
{
	struct
	{
		RGX_HWPERF_DM eDM;				/*!< Data Master ID. */
		RGX_HWPERF_HWR_REASON eReason;	/*!< Reason of the HWR. */
		IMG_UINT32 ui32DMContext;		/*!< FW render context */
	} sHWR;								/*!< HWR sub-event data. */

	RGX_HWPERF_BVNC sBVNC;              /*!< BVNC Features. See RGX_HWPERF_BVNC */
	struct
	{
		IMG_UINT32 ui32EvMaskLo;		/*!< Low order 32 bits of Filter Mask */
		IMG_UINT32 ui32EvMaskHi;		/*!< High order 32 bits of Filter Mask */
	} sEvMsk;							/*!< HW Filter Mask */
	RGX_HWPERF_COUNTER_CFG sPCC;        /*!< Performance Counter Config. See RGX_HWPERF_COUNTER_CFG */
} RGX_HWPERF_FWACT_DETAIL;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_FWACT_DETAIL);

/*! This structure holds the data of a FW activity event packet */
typedef struct
{
	RGX_HWPERF_FWACT_EV eEvType;           /*!< Event type. */
	RGX_HWPERF_FWACT_DETAIL uFwActDetail;  /*!< Data of the sub-event. */
	IMG_UINT32 ui32Padding;                /*!< Reserved. */
} RGX_HWPERF_FWACT_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_FWACT_DATA);


typedef enum {
	RGX_HWPERF_UFO_EV_UPDATE,          /*!< Update on the UFO objects. */
	RGX_HWPERF_UFO_EV_CHECK_SUCCESS,   /*!< Successful check on UFO objects. */
	RGX_HWPERF_UFO_EV_PRCHECK_SUCCESS, /*!< Successful partial render check on UFO objects. */
	RGX_HWPERF_UFO_EV_CHECK_FAIL,      /*!< Unsuccessful check on UFO objects. */
	RGX_HWPERF_UFO_EV_PRCHECK_FAIL,    /*!< Unsuccessful partial render check on UFO objects. */
	RGX_HWPERF_UFO_EV_FORCE_UPDATE,    /*!< Forced erroring of the UFO objects. */

	RGX_HWPERF_UFO_EV_LAST             /*!< Reserved. Do not use. */
} RGX_HWPERF_UFO_EV;

/*! Data stream tuple. */
typedef union
{
	struct
	{
		IMG_UINT32 ui32FWAddr;        /*!< UFO's unique address */
		IMG_UINT32 ui32Value;         /*!< Value of the UFO object */
	} sCheckSuccess;
	struct
	{
		IMG_UINT32 ui32FWAddr;        /*!< UFO's unique address */
		IMG_UINT32 ui32Value;         /*!< Value of the UFO object */
		IMG_UINT32 ui32Required;      /*!< Value of the UFO object required by the fence */
	} sCheckFail;
	struct
	{
		IMG_UINT32 ui32FWAddr;        /*!< UFO's unique address */
		IMG_UINT32 ui32OldValue;      /*!< Value of UFO object before update */
		IMG_UINT32 ui32NewValue;      /*!< Value of UFO object after update */
	} sUpdate;
} RGX_HWPERF_UFO_DATA_ELEMENT;

/*! This structure holds the packet payload data for UFO event. */
typedef struct
{
	RGX_HWPERF_UFO_EV eEvType;                                      /*!< Subtype of the event. See RGX_HWPERF_UFO_EV */
	IMG_UINT64        ui64SoCTimestamp;                             /*!< SoC timestamp */
	IMG_UINT32        ui32TimeCorrIndex;                            /*!< Index to the timer correlation data
	                                                                  at the time the packet was generated.
	                                                                  Used to approximate Host timestamps for
	                                                                  these events. */
	IMG_UINT32 ui32PID;                                             /*!< Client process identifier */
	IMG_UINT32 ui32ExtJobRef;                                       /*!< Reference used by callers of the RGX
	                                                                  API to track submitted work (for
	                                                                  debugging/trace purposes) */
	IMG_UINT32 ui32IntJobRef;                                       /*!< Internal reference used to track
	                                                                  submitted work (for debugging / trace
	                                                                  purposes) */
	IMG_UINT32 ui32DMContext;                                       /*!< GPU Data Master (FW) Context */
	IMG_UINT32 ui32StreamInfo;                                      /*!< Encoded number of elements in the
	                                                                  stream and stream data offset in the
	                                                                  payload */
	RGX_HWPERF_DM eDM;                                              /*!< Data Master number, see RGX_HWPERF_DM */
	IMG_UINT32    ui32Padding;                                      /*!< Unused, reserved */
	IMG_UINT32    aui32StreamData[RGX_HWPERF_ONE_OR_MORE_ELEMENTS]; /*!< Series of tuples holding UFO objects data */
} RGX_HWPERF_UFO_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_UFO_DATA);


/*!
 * RGX_HWPERF_KICK_TYPE describes the type of kick for events received / sent
 * between KICK_START / KICK_END inclusively for all event types.
 */
typedef enum
{
	RGX_HWPERF_KICK_TYPE_TA3D,  /*!< Replaced by separate TA and 3D types */
	RGX_HWPERF_KICK_TYPE_CDM,   /*!< Compute Data Master Kick */
	RGX_HWPERF_KICK_TYPE_RS,    /*!< Ray Store Kick */
	RGX_HWPERF_KICK_TYPE_SHG,   /*!< Scene Hierarchy Generator Kick */
	RGX_HWPERF_KICK_TYPE_TQTDM, /*!< TQ 2D Data Master Kick */
	RGX_HWPERF_KICK_TYPE_SYNC,  /*!< Sync Kick */
	RGX_HWPERF_KICK_TYPE_TA,    /*!< TA Kick */
	RGX_HWPERF_KICK_TYPE_3D,    /*!< 3D Kick */
	RGX_HWPERF_KICK_TYPE_CE,    /*!< ce Kick */
	RGX_HWPERF_KICK_TYPE_LAST,

	RGX_HWPERF_KICK_TYPE_FORCE_32BIT = 0x7fffffff
} RGX_HWPERF_KICK_TYPE;

typedef struct
{
	RGX_HWPERF_KICK_TYPE ui32EnqType; /*!< Workload type sent to FW for
	                                   scheduling on GPU hardware.
	                                   See RGX_HWPERF_KICK_TYPE */
	IMG_UINT32 ui32PID;               /*!< Client process identifier */
	IMG_UINT32 ui32ExtJobRef;         /*!< Reference used by callers of the RGX API
	                                   to track submitted work (for debugging /
	                                   trace purposes) */
	IMG_UINT32 ui32IntJobRef;         /*!< internal reference used to track submitted
	                                   work (for debugging / trace purposes) */
	IMG_UINT32 ui32DMContext;         /*!< GPU Data Master (FW) Context */
	IMG_UINT32 ui32Padding;           /*!< Unused, reserved */
	IMG_UINT64 ui64CheckFence_UID;    /*!< ID of fence gating work execution on GPU */
	IMG_UINT64 ui64UpdateFence_UID;   /*!< ID of fence triggered after work completes on GPU */
	IMG_UINT64 ui64DeadlineInus;      /*!< Workload deadline in system monotonic time */
	IMG_UINT32 ui32CycleEstimate;     /*!< Estimated cycle time for the workload */
	PVRSRV_FENCE hCheckFence;         /*!< Fence this enqueue task waits for, before starting */
	PVRSRV_FENCE hUpdateFence;        /*!< Fence this enqueue task signals, on completion */
	PVRSRV_TIMELINE hUpdateTimeline;  /*!< Timeline on which the above hUpdateFence is created */

	/* Align structure size to 8 bytes */
} RGX_HWPERF_HOST_ENQ_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_ENQ_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_ENQ_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef struct
{
	RGX_HWPERF_KICK_TYPE ui32NotifyType; /*!< Workload type sent to FW for scheduling on GPU hardware.
	                                      See RGX_HWPERF_KICK_TYPE */
	IMG_UINT32 ui32PID;                  /*!< Client process identifier */
	IMG_UINT32 ui32DMContext;            /*!< GPU Data Master (FW) Context */
	IMG_UINT32 ui32Padding;              /*!< Align structure size to 8 bytes */
} RGX_HWPERF_HOST_NOTIFY_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_NOTIFY_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_NOTIFY_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef struct
{
	RGX_HWPERF_UFO_EV eEvType;   /*!< Subtype of the event */
	IMG_UINT32 ui32StreamInfo;   /*!< Encoded number of elements in the stream and
	                              stream data offset in the payload */
#ifdef __CHECKER__
	/* Since we're not conforming to the C99 standard by not using a flexible
	 * array member need to add a special case for Smatch static code analyser. */
	IMG_UINT32 aui32StreamData[];
#else
	IMG_UINT32 aui32StreamData[RGX_HWPERF_ONE_OR_MORE_ELEMENTS];
	                             /*!< Series of tuples holding UFO objects data */

	IMG_UINT32 ui32Padding;      /*!< Reserved, align structure size to 8 bytes */
#endif
} RGX_HWPERF_HOST_UFO_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_UFO_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_UFO_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

/*!
 * RGX_HWPERF_HOST_RESOURCE_TYPE describes the type of resource which has been
 * Allocated, Freed or Modified. The values are used to determine which event
 * data structure to use to decode the data from the event stream
 */
typedef enum
{
	RGX_HWPERF_HOST_RESOURCE_TYPE_INVALID,   /*!< Invalid */
	RGX_HWPERF_HOST_RESOURCE_TYPE_SYNC,      /*!< SyncPrim */
	RGX_HWPERF_HOST_RESOURCE_TYPE_TIMELINE_DEPRECATED,
	                                         /*!< Timeline resource packets are
	                                           now emitted in client hwperf buffer */
	RGX_HWPERF_HOST_RESOURCE_TYPE_FENCE_PVR, /*!< Fence for use on GPU (SYNC_CP backed) */
	RGX_HWPERF_HOST_RESOURCE_TYPE_SYNC_CP,   /*!< Sync Checkpoint */
	RGX_HWPERF_HOST_RESOURCE_TYPE_FENCE_SW,  /*!< Fence created on SW timeline */

	RGX_HWPERF_HOST_RESOURCE_TYPE_LAST       /*!< End of enumeration */
} RGX_HWPERF_HOST_RESOURCE_TYPE;

typedef union
{
	/*! Data for TYPE_TIMELINE (*Deprecated*). This sub-event is no longer
     *  generated in the HOST stream. Timeline data is now provided in the
     *  CLIENT stream instead.
     */
	struct
	{
		IMG_UINT32 uiPid;             /*!< Identifier of owning process */
		IMG_UINT64 ui64Timeline_UID1; /*!< Unique identifier for timeline resource */
		IMG_CHAR acName[PVRSRV_SYNC_NAME_LENGTH];
		                              /*!< Label or name given to the sync resource */
		IMG_UINT32 ui32Padding;       /*!< Reserved. Align structure size to 8 bytes */
	} sTimelineAlloc;

	/*! Data for TYPE_FENCE_PVR */
	struct
	{
		IMG_PID uiPID;                /*!< Identifier of owning process */
		PVRSRV_FENCE hFence;          /*!< Unique identifier for the fence resource */
		IMG_UINT32 ui32CheckPt_FWAddr; /*!< Unique identifier of the check point
		                                backing this fence on the GPU */
		IMG_CHAR acName[PVRSRV_SYNC_NAME_LENGTH];
		                              /*!< Label or name given to the sync resource */
	} sFenceAlloc;

	/*! Data for TYPE_SYNC_CP */
	struct
	{
		IMG_UINT32 ui32CheckPt_FWAddr; /*!< Unique identifier for the check point resource */
		PVRSRV_TIMELINE hTimeline;     /*!< Unique identifier for the timeline resource */
		IMG_PID uiPID;                 /*!< Identifier of owning process */
		PVRSRV_FENCE hFence;           /*!< Unique identifier for the fence resource */
		IMG_CHAR acName[PVRSRV_SYNC_NAME_LENGTH];
		                               /*!< Label or name given to the sync resource */
	} sSyncCheckPointAlloc;

	/*! Data for TYPE_FENCE_SW */
	struct
	{
		IMG_PID uiPID;                 /*!< Identifier of owning process */
		PVRSRV_FENCE hSWFence;         /*!< Unique identifier for the SWFence resource */
		PVRSRV_TIMELINE hSWTimeline;   /*!< Unique identifier for the timeline resource */
		IMG_UINT64 ui64SyncPtIndex;    /*!< Sync-pt index where this SW timeline has reached */
		IMG_CHAR acName[PVRSRV_SYNC_NAME_LENGTH];
		                               /*!< Label or name given to the sync resource */
	} sSWFenceAlloc;

	/*! Data for TYPE_SYNC */
	struct
	{
		IMG_UINT32 ui32FWAddr;         /*!< Identifier of sync resource */
		IMG_CHAR acName[PVRSRV_SYNC_NAME_LENGTH];
		                               /*!< Label or name given to the sync resource */
	} sSyncAlloc;
} RGX_HWPERF_HOST_ALLOC_DETAIL;

typedef struct
{
	RGX_HWPERF_HOST_RESOURCE_TYPE ui32AllocType;
	                                 /*!< This describes the type of the resource
	                                  allocated in the driver. See
	                                  RGX_HWPERF_HOST_RESOURCE_TYPE */
	RGX_HWPERF_HOST_ALLOC_DETAIL RGXFW_ALIGN uAllocDetail;
	                                 /*!< Union of structures providing further
	                                  data regarding the resource allocated.
	                                  Size of data varies with union member that
	                                  is present, check ``ui32AllocType`` value
	                                  to decode */
} RGX_HWPERF_HOST_ALLOC_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_ALLOC_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_ALLOC_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef union
{
	/*! Data for TYPE_TIMELINE (*Deprecated*) */
	struct
	{
		IMG_UINT32 uiPid;             /*!< Identifier of owning process */
		IMG_UINT64 ui64Timeline_UID1; /*!< Unique identifier for the timeline resource */
		IMG_UINT32 ui32Padding;       /*!< Reserved. Align structure size to 8 bytes */
	} sTimelineDestroy;

	/*! Data for TYPE_FENCE_PVR */
	struct
	{
		IMG_UINT64 ui64Fence_UID;     /*!< Unique identifier for the fence resource */
		IMG_UINT32 ui32Padding;       /*!< Reserved. */
	} sFenceDestroy;

	/*! Data for TYPE_SYNC_CP */
	struct
	{
		IMG_UINT32 ui32CheckPt_FWAddr;  /*!< Unique identifier for the check point resource */
	} sSyncCheckPointFree;

	/*! Data for TYPE_SYNC */
	struct
	{
		IMG_UINT32 ui32FWAddr;        /*!< Unique identifier for the sync resource */
	} sSyncFree;
} RGX_HWPERF_HOST_FREE_DETAIL;

typedef struct
{
	RGX_HWPERF_HOST_RESOURCE_TYPE ui32FreeType;
	                              /*!< This describes the type of the resource
	                               freed or released by the driver. See
	                               RGX_HWPERF_HOST_RESOURCE_TYPE */
	RGX_HWPERF_HOST_FREE_DETAIL uFreeDetail;
	                              /*!< Union of structures providing further data
	                               regarding the resource freed. Size of data
	                               varies with union member that is present,
	                               check ``ui32FreeType`` value to decode */
	IMG_UINT32 ui32Padding;       /*!< Reserved. Align structure size to 8 bytes */
} RGX_HWPERF_HOST_FREE_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_FREE_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_FREE_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef struct
{
	IMG_UINT64 ui64CRTimestamp;   /*!< CR timer value from the latest entry of
	                               the time domains correlation table */
	IMG_UINT64 ui64OSTimestamp;   /*!< OS timestamp from the latest entry of the
	                               time domains correlation table */
	IMG_UINT32 ui32ClockSpeed;    /*!< GPU clock speed from the latest entry of
	                               the time domains correlation table */
	IMG_UINT32 ui32Padding;       /*!< Reserved, align structure size to 8 bytes */
} RGX_HWPERF_HOST_CLK_SYNC_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_CLK_SYNC_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_CLK_SYNC_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef union
{
	/*! Data for TYPE_FENCE_PVR */
	struct
	{
		IMG_UINT64 ui64NewFence_UID;  /*!< Unique identifier for the new merged fence
		                               resource that has been created */
		IMG_UINT64 ui64InFence1_UID;  /*!< Unique identifier for the fence resource */
		IMG_UINT64 ui64InFence2_UID;  /*!< Unique identifier of the check point backing
		                               the fence on the GPU */
		IMG_CHAR acName[PVRSRV_SYNC_NAME_LENGTH];
		                              /*!< Label or name given to the sync resource */
		IMG_UINT32 ui32Padding;       /*!< Reserved. Align structure size to 8 bytes */
	} sFenceMerge;
} RGX_HWPERF_HOST_MODIFY_DETAIL;

typedef struct
{
	RGX_HWPERF_HOST_RESOURCE_TYPE ui32ModifyType;
	                                /*!< Describes the type of the resource
	                                 modified by the driver. See
	                                 RGX_HWPERF_HOST_RESOURCE_TYPE */

	RGX_HWPERF_HOST_MODIFY_DETAIL uModifyDetail;
	                                /*!< Union of structures providing further
	                                 data regarding the resource modified.
	                                 Size of data varies with union member that
	                                 is present.
	                                 Check ``uiModifyType`` value to decode */
} RGX_HWPERF_HOST_MODIFY_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_MODIFY_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_MODIFY_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef enum
{
	RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS_UNDEFINED = 0, /*!< Invalid */
	RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS_OK,            /*!< Device OK */
	RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS_RESPONDING,    /*!< Device responding to requests */
	RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS_DEAD,          /*!< Device not responding */
	RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS_FAULT,         /*!< Device has faulted */

	RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS_LAST
} RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS;

typedef enum
{
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_UNDEFINED = 0,     /*!< Invalid */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_NONE,              /*!< No underlying health reason. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_ASSERTED,          /*!< Device has asserted. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_POLL_FAILING,      /*!< Device poll has failed. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_TIMEOUTS,          /*!< Device timeout has fired. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_QUEUE_CORRUPT,     /*!< Queue has become corrupt. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_QUEUE_STALLED,     /*!< Queue has stalled. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_IDLING,            /*!< Device is idling. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_RESTARTING,        /*!< Device restarting. */
	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_MISSING_INTERRUPTS,/*!< Interrupts have been discarded. */

	RGX_HWPERF_HOST_DEVICE_HEALTH_REASON_LAST
} RGX_HWPERF_HOST_DEVICE_HEALTH_REASON;

/*! RGX_HWPERF_DEV_INFO_EV values */
typedef enum
{
	RGX_HWPERF_DEV_INFO_EV_HEALTH,      /*!< Health sub-event */

	RGX_HWPERF_DEV_INFO_EV_LAST         /*!< Last enumeration value */
} RGX_HWPERF_DEV_INFO_EV;

/*! RGX_HWPERF_HOST_DEV_INFO_DETAIL is a union of structures providing
 *  further data regarding the device's status
 */
typedef union
{
	/*! Data for device status event */
	struct
	{
		RGX_HWPERF_HOST_DEVICE_HEALTH_STATUS eDeviceHealthStatus;
		                                 /*!< Device's health status */
		RGX_HWPERF_HOST_DEVICE_HEALTH_REASON eDeviceHealthReason;
		                                 /*!< Reason for device's health status */
	} sDeviceStatus;
} RGX_HWPERF_HOST_DEV_INFO_DETAIL;

/*! RGX_HWPERF_HOST_DEV_INFO_DATA contains device health status information */
typedef struct
{
	IMG_UINT32                      ui32Padding;
	                            /*!< Reserved. Align structure size to 8 bytes */
	RGX_HWPERF_DEV_INFO_EV          eEvType;
	                            /*!< Type of the sub-event. See
	                              RGX_HWPERF_DEV_INFO_EV */
	RGX_HWPERF_HOST_DEV_INFO_DETAIL uDevInfoDetail;
	                            /*!< Union of structures providing further data
	                              regarding the device's status. Size of data
	                              varies with union member that is present,
	                              check ``eEvType`` value to decode */
} RGX_HWPERF_HOST_DEV_INFO_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_DEV_INFO_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			 "sizeof(RGX_HWPERF_HOST_DEV_INFO_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

/*! RGX_HWPERF_INFO_EV event subtype for RGX_HWPERF_HOST_INFO_DATA events */
typedef enum
{
	RGX_HWPERF_INFO_EV_MEM_USAGE,     /*!< Memory usage event */
	RGX_HWPERF_INFO_EV_LAST           /*!< End of enumeration */
} RGX_HWPERF_INFO_EV;

/*! RGX_HWPERF_HOST_INFO_DETAIL contains the data payload for the
 * RGX_HWPERF_HOST_INFO_DATA event.
 */
typedef union
{
	/*! Host Memory usage statistics */
	struct
	{
		IMG_UINT32 ui32TotalMemoryUsage;   /*!< Total memory usage */
		/*! Detailed memory usage */
		struct
		{
			IMG_UINT32 ui32Pid;              /*!< Process ID */
			IMG_UINT32 ui32KernelMemUsage;   /*!< Kernel memory usage */
			IMG_UINT32 ui32GraphicsMemUsage; /*!< GPU memory usage */
		} sPerProcessUsage[RGX_HWPERF_ZERO_OR_MORE_ELEMENTS];
	} sMemUsageStats;
} RGX_HWPERF_HOST_INFO_DETAIL;

/*! RGX_HWPERF_HOST_INFO_DATA. Host Info data event payload contains device
 * memory usage information.
 */
typedef struct
{
	IMG_UINT32 ui32Padding;       /*!< Reserved. Align structure size to 8 bytes */
	RGX_HWPERF_INFO_EV eEvType;   /*!< Type of subevent. See RGX_HWPERF_INFO_EV */
	RGX_HWPERF_HOST_INFO_DETAIL uInfoDetail;
	                              /*!< Union of structures providing further data
	                               regarding memory usage. Size varies with union
	                               member that is present, check ``eEvType``
	                               value to decode */
} RGX_HWPERF_HOST_INFO_DATA;

/* Payload size must be multiple of 8 bytes to align start of next packet. */
static_assert((sizeof(RGX_HWPERF_HOST_INFO_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_INFO_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

/*! FENCE_WAIT_TYPE definitions */
typedef enum
{
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_TYPE_BEGIN = 0,    /*!< Begin */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_TYPE_END,          /*!< End */

	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_TYPE_LAST,         /*!< Do not use */
} RGX_HWPERF_HOST_SYNC_FENCE_WAIT_TYPE;

/*! FENCE_WAIT_RESULT definitions */
typedef enum
{
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT_INVALID = 0, /*!< Invalid */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT_TIMEOUT,     /*!< Timed Out */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT_PASSED,      /*!< Passed */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT_ERROR,       /*!< Errored */

	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT_LAST,        /*!< Do not use */
} RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT;

/*! FENCE_WAIT_DETAIL Event Payload */
typedef union
{
/*! Data for SYNC_FENCE_WAIT_TYPE_BEGIN */
	struct
	{
		IMG_UINT32 ui32TimeoutInMs;                     /*!< Wait timeout (ms) */
	} sBegin;

	/*! Data for SYNC_FENCE_WAIT_TYPE_END */
	struct
	{
		RGX_HWPERF_HOST_SYNC_FENCE_WAIT_RESULT eResult; /*!< Wait result */
	} sEnd;
} RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DETAIL;

/*! RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DATA Event Payload. This data structure
 * is received whenever the host driver handles a wait for sync event request.
 */
typedef struct
{
	IMG_PID uiPID;          /*!< Identifier of the owning process */
	PVRSRV_FENCE hFence;    /*!< Unique identifier for the fence resource */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_TYPE eType;
	                        /*!< Type of the subevent, see
	                         RGX_HWPERF_HOST_SYNC_FENCE_WAIT_TYPE */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DETAIL uDetail;
	                        /*!< Union of structures providing further data
	                         regarding device's status. Size of data varies with
	                         union member that is present, check ``eType`` value
	                         to decode */

} RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DATA;

static_assert((sizeof(RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

/*! RGX_HWPERF_HOST_SYNC_SW_TL_ADV_DATA.
 * Software Timeline Advanced Event Payload. This data structure is received
 * whenever the host driver processes a Software Timeline Advanced event.
 */
typedef struct
{
	IMG_PID uiPID;                /*!< Identifier of the owning process */
	PVRSRV_TIMELINE hTimeline;    /*!< Unique identifier for the timeline resource */
	IMG_UINT64 ui64SyncPtIndex;   /*!< Index of the sync point to which the
	                               timeline has advanced */

} RGX_HWPERF_HOST_SYNC_SW_TL_ADV_DATA;

static_assert((sizeof(RGX_HWPERF_HOST_SYNC_SW_TL_ADV_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_SYNC_SW_TL_ADV_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");

typedef enum
{
	RGX_HWPERF_HOST_CLIENT_INFO_TYPE_INVALID = 0,  /*!< Invalid */
	RGX_HWPERF_HOST_CLIENT_INFO_TYPE_PROCESS_NAME, /*!< Process Name */

	RGX_HWPERF_HOST_CLIENT_INFO_TYPE_LAST,         /*!< Do not use */
} RGX_HWPERF_HOST_CLIENT_INFO_TYPE;

typedef struct
{
	IMG_PID uiClientPID; /*!< Client process identifier */
	IMG_UINT32 ui32Length;  /*!< Number of bytes present in ``acName`` */
	IMG_CHAR acName[RGX_HWPERF_ONE_OR_MORE_ELEMENTS]; /*!< Process name string, null terminated */
} RGX_HWPERF_HOST_CLIENT_PROC_NAME;

#define RGX_HWPERF_HOST_CLIENT_PROC_NAME_SIZE(ui32NameLen) \
	((IMG_UINT32)(offsetof(RGX_HWPERF_HOST_CLIENT_PROC_NAME, acName) + (ui32NameLen)))

typedef union
{
	struct
	{
		IMG_UINT32 ui32Count; /*!< Number of elements in ``asProcNames`` */
		RGX_HWPERF_HOST_CLIENT_PROC_NAME asProcNames[RGX_HWPERF_ONE_OR_MORE_ELEMENTS];
	} sProcName;
} RGX_HWPERF_HOST_CLIENT_INFO_DETAIL;

typedef struct
{
	IMG_UINT32 uiReserved1; /*!< Reserved. Align structure size to 8 bytes */
	RGX_HWPERF_HOST_CLIENT_INFO_TYPE eType;
	                        /*!< Type of the subevent, see
	                         RGX_HWPERF_HOST_CLIENT_INFO_TYPE */
	RGX_HWPERF_HOST_CLIENT_INFO_DETAIL uDetail;
	                        /*!< Union of structures. Size of data
	                         varies with union member that is present,
	                         check ``eType`` value to decode */

} RGX_HWPERF_HOST_CLIENT_INFO_DATA;

#define RGX_HWPERF_HOST_TIME_CORRELATION_MAX_COUNT 64U
typedef struct
{
	IMG_UINT64 ui64OSTimeStamp;
	IMG_UINT64 ui64CRTimeStamp;

	/* Utility variable used to convert CR timer deltas to OS timer deltas (nS),
	 * where the deltas are relative to the timestamps above:
	 * deltaOS = (deltaCR * K) >> decimal_shift, see full explanation below */
	IMG_UINT64 ui64CRDeltaToOSDeltaKNs;

	IMG_UINT32 ui32CoreClockSpeed;
	IMG_UINT32 ui32Reserved;
} RGX_HWPERF_HOST_TIME_CORRELATION_DETAIL;

typedef struct 
{
	IMG_UINT16 ui16Count; /*!< Number of elements in ``asTimeCorrelation`` */
	IMG_UINT16 ui16StartIndex; /*!< Start Index */
	IMG_UINT16 ui16HostIndex; /*!< Host time correlation sequence count */
	IMG_UINT16 ui16Padding;        /*!< Unused, reserved */
	RGX_HWPERF_HOST_TIME_CORRELATION_DETAIL asTimeCorrelation[RGX_HWPERF_ONE_OR_MORE_ELEMENTS];
} RGX_HWPERF_HOST_TIME_CORRELATION_DATA;

static_assert((sizeof(RGX_HWPERF_HOST_CLIENT_INFO_DATA) & (PVRSRVTL_PACKET_ALIGNMENT-1U)) == 0U,
			  "sizeof(RGX_HWPERF_HOST_CLIENT_INFO_DATA) must be a multiple PVRSRVTL_PACKET_ALIGNMENT");
#define RGX_HWPERF_HOST_DEBUG_VALUES_LENGTH 8
typedef struct
{
	IMG_UINT32 ui32PID;
	IMG_UINT32 ui32TID;
	IMG_UINT32 ui32ValuesSize;
	IMG_UINT32 ui32NameSize;
	IMG_UINT64 aui64Values[RGX_HWPERF_HOST_DEBUG_VALUES_LENGTH];
	IMG_CHAR acFuncName[RGX_HWPERF_ONE_OR_MORE_ELEMENTS];
} RGX_HWPERF_HOST_DEBUG_DATA;

typedef enum
{
	RGX_HWPERF_HOST_DMA_TYPE_INVALID = 0,  /*!< Invalid */
	RGX_HWPERF_HOST_DMA_TYPE_USER,         /*!< DMA transfer request from host to device or from device to host */
	RGX_HWPERF_HOST_DMA_TYPE_P2P,          /*!< DMA transfer from device to another device */
	RGX_HWPERF_HOST_DMA_TYPE_LAST,         /*!< Do not use */
} RGX_HWPERF_HOST_DMA_TYPE;

typedef union
{
	struct RGX_HWPERF_HOST_DMA_USER_DATA
	{
		IMG_UINT32 uiReserved1; /*!< Reserved. Align structure size to 8 bytes */
		IMG_BOOL bS2D;          /*!< Is copy from host to device */
		IMG_UINT64 ui64Addr;    /*!< Host address */
		IMG_UINT64 ui64DevAddr; /*!< Device address */
	} sUserData;

	struct RGX_HWPERF_HOST_DMA_P2P_DATA
	{
		IMG_INT32 i32PeerDevID;      /*!< The identifier of the peer device */
		IMG_BOOL bPeer2Local;        /*!< Is copy from peer device to local device */
		IMG_UINT64 ui64LocalDevAddr; /*!< Local device address */
		IMG_UINT64 ui64PeerDevAddr;  /*!< Peer device address */
	} sP2PData;
} RGX_HWPERF_HOST_DMA_DETAIL;

typedef struct
{
	RGX_HWPERF_HOST_DMA_TYPE eType;     /*!< Type of the DMA transfer */
	RGX_HWPERF_HOST_DMA_DETAIL eDetail; /*!< Union of structures. Size of data
	                                         varies with union member that is present,
	                                         check ``eType`` value to decode */
	IMG_PID uiPID;                      /*!< Identifier of the owning process */
	IMG_PID uiTID;                      /*!< Identifier of the owning thread */
	IMG_DEVMEM_SIZE_T uiSize;           /*!< Copy size */
	PVRSRV_TIMELINE hUpdateTimeline;    /*!< Unique identifier for the update timeline resource */
	IMG_UINT32 ui32ExtJobRef;           /*!< Reference used by callers of the RGX API to track
	                                         submitted work (for debugging / trace purposes) */
} RGX_HWPERF_HOST_DMA_DATA;

typedef enum
{
	RGX_HWPERF_RESOURCE_CAPTURE_TYPE_NONE,
	RGX_HWPERF_RESOURCE_CAPTURE_TYPE_DEFAULT_FRAMEBUFFER,
	RGX_HWPERF_RESOURCE_CAPTURE_TYPE_OFFSCREEN_FB_ATTACHMENTS,
	RGX_HWPERF_RESOURCE_CAPTURE_TYPE_TILE_LIFETIME_DATA,

	RGX_HWPERF_RESOURCE_TYPE_COUNT
} RGX_HWPERF_RESOURCE_CAPTURE_TYPE;

typedef struct
{
	IMG_UINT32 ui32Height;
	IMG_UINT32 ui32Width;
	IMG_UINT32 ui32BPP;
	IMG_UINT32 ui32PixFormat;
} RGX_RESOURCE_PER_SURFACE_INFO, *PRGX_RESOURCE_PER_SURFACE_INFO;

typedef struct
{
	IMG_INT32  i32XOffset;        /*!< render surface X shift */
	IMG_INT32  i32YOffset;        /*!< render surface Y shift */
	IMG_UINT32 ui32WidthInTiles;  /*!< number of TLT data points in X */
	IMG_UINT32 ui32HeightInTiles; /*!< number of TLT data points in Y */
} RGX_RESOURCE_PER_TLT_BUFFER_INFO, *PRGX_RESOURCE_PER_TLT_BUFFER_INFO;

typedef union
{
	struct RGX_RESOURCE_CAPTURE_RENDER_SURFACES
	{
		IMG_UINT32 ui32RenderSurfaceCount;
		RGX_RESOURCE_PER_SURFACE_INFO sSurface[RGX_HWPERF_ONE_OR_MORE_ELEMENTS];
	} sRenderSurfaces;

	struct RGX_RESOURCE_CAPTURE_TILE_LIFETIME_BUFFERS
	{
		RGX_RESOURCE_PER_TLT_BUFFER_INFO sTLTBufInfo[RGX_HWPERF_ONE_OR_MORE_ELEMENTS];
	} sTLTBuffers;
} RGX_RESOURCE_CAPTURE_DETAIL;

typedef struct
{
	RGX_HWPERF_RESOURCE_CAPTURE_TYPE eType;
	IMG_PID uPID;
	IMG_UINT32 ui32ContextID;
	IMG_UINT32 ui32FrameNum;
	IMG_UINT32 ui32CapturedTaskJobRef;	/* The job ref of the HW task that emitted the data */
	IMG_INT32 eClientModule;			/* RGX_HWPERF_CLIENT_API - ID that the capture is originating from. */
	RGX_RESOURCE_CAPTURE_DETAIL uDetail; /* eType determines the value of the union */
} RGX_RESOURCE_CAPTURE_INFO, *PRGX_RESOURCE_CAPTURE_INFO;

#define RGX_RESOURCE_CAPTURE_INFO_BASE_SIZE() offsetof(RGX_RESOURCE_CAPTURE_INFO, uDetail)

/*! Tile Lifetime Tracking header size. Only available if
 * RGX_FEATURE_ISP_TILE_LIFETIME_TRACKING is present and enabled via
 * SUPPORT_TLT_PERF
 */
#define RGX_TLT_HARDWARE_HDR_SIZE   (16U)

/* PVRSRVGetHWPerfResourceCaptureResult */
typedef enum
{
	RGX_HWPERF_RESOURCE_CAPTURE_RESULT_NONE = 0,
	RGX_HWPERF_RESOURCE_CAPTURE_RESULT_OK,					/* We got data ok, expect more packets for this request. */
	RGX_HWPERF_RESOURCE_CAPTURE_RESULT_NOT_READY,			/* Signals a timeout on the connection - no data available yet. */
	RGX_HWPERF_RESOURCE_CAPTURE_RESULT_COMPLETE_SUCCESS,	/* The request completed successfully, signals the end of packets for the request. */
	RGX_HWPERF_RESOURCE_CAPTURE_RESULT_COMPLETE_FAILURE		/* The request failed, signals the end of packets for the request. */
} RGX_HWPERF_RESOURCE_CAPTURE_RESULT_STATUS;

typedef struct
{
	IMG_PID uPID;						/* In case of a failed request pass the caller the PID and context ID. */
	IMG_UINT32 ui32CtxID;
	RGX_RESOURCE_CAPTURE_INFO *psInfo;	/* Various meta-data regarding the captured resource which aid the requester when,
											unpacking the resource data, valid if RGX_HWPERF_RESOURCE_CAPTURE_RESULT_OK is returned. */
	IMG_BYTE *pbData;					/* Buffer containing the captured resource data, valid if RGX_HWPERF_RESOURCE_CAPTURE_RESULT_OK is returned. */
} RGX_RESOURCE_CAPTURE_RESULT;

/*! This type is a union of packet payload data structures associated with
 * various FW and Host events */
typedef union
{
	RGX_HWPERF_FW_DATA             sFW;           /*!< Firmware event packet data, events ``0x01-0x04`` */
	RGX_HWPERF_FW_DBG_DATA         sFWDBG;        /*!< DBG data, events ``0x05-0x06`` */
	RGX_HWPERF_HW_DATA             sHW;           /*!< Hardware event packet data,
	                                               events ``0x07-0x19``, ``0x28-0x29``
	                                               See RGX_HWPERF_HW_DATA */
	RGX_HWPERF_CLKS_CHG_DATA       sCLKSCHG;      /*!< Clock change event packet
	                                               data, events ``0x1A`` */
	RGX_HWPERF_GPU_STATE_CHG_DATA  sGPUSTATECHG;  /*!< GPU utilisation state
	                                               change event packet data,
	                                               events ``0x1B`` */
	RGX_HWPERF_PWR_EST_DATA        sPWREST;       /*!< Power estimate event
	                                               packet data,
	                                               events ``0x20-0x22`` */
	RGX_HWPERF_PWR_CHG_DATA        sPWR;          /*!< Power event packet data,
	                                               events ``0x23`` */
	RGX_HWPERF_CSW_DATA            sCSW;          /*!< Context switch packet data,
	                                               events ``0x30-0x31`` */
	RGX_HWPERF_DVFS_DATA           sDVFS;         /*!< DVFS activity data,
	                                               events ``0x32`` */
	RGX_HWPERF_UFO_DATA            sUFO;          /*!< UFO data, events ``0x38`` */
	RGX_HWPERF_FWACT_DATA          sFWACT;        /*!< Firmware activity event
	                                               packet data,
	                                               events ``0x39`` */
	/* */
	RGX_HWPERF_HOST_ENQ_DATA       sENQ;          /*!< Host ENQ data,
	                                               events ``0x01`` (Host) */
	RGX_HWPERF_HOST_UFO_DATA       sHUFO;         /*!< Host UFO data,
	                                               events ``0x02`` (Host) */
	RGX_HWPERF_HOST_ALLOC_DATA     sHALLOC;       /*!< Host Alloc data,
	                                               events ``0x03`` (Host) */
	RGX_HWPERF_HOST_CLK_SYNC_DATA  sHCLKSYNC;     /*!< Host CLK_SYNC data,
	                                               events ``0x04`` (Host) */
	RGX_HWPERF_HOST_FREE_DATA      sHFREE;        /*!< Host Free data,
	                                               events ``0x05`` (Host) */
	RGX_HWPERF_HOST_MODIFY_DATA    sHMOD;         /*!< Host Modify data,
	                                               events ``0x06`` (Host) */
	RGX_HWPERF_HOST_DEV_INFO_DATA  sHDEVINFO;     /*!< Host device info data,
	                                               events ``0x07`` (Host) */
	RGX_HWPERF_HOST_INFO_DATA      sHINFO;        /*!< Host info data,
	                                               events ``0x08`` (Host) */
	RGX_HWPERF_HOST_SYNC_FENCE_WAIT_DATA sWAIT;   /*!< Host fence-wait data,
	                                               events ``0x09`` (Host) */
	RGX_HWPERF_HOST_SYNC_SW_TL_ADV_DATA sSWTLADV; /*!< Host SW-timeline advance
	                                               data, events ``0x0A`` (Host) */
	RGX_HWPERF_HOST_CLIENT_INFO_DATA sHClientInfo; /*!< Host client info,
	                                               events ``0x0B`` (Host) */
	RGX_HWPERF_HOST_TIME_CORRELATION_DATA sHTimeCorrelation; /*!< Time correlation 
	                                               events ``0x0C`` (Host) */
	RGX_HWPERF_HOST_DMA_DATA sHDMA;
	RGX_HWPERF_HOST_NOTIFY_DATA sHNotify;         /*!< Host Notify data,
	                                               events ``0x0F`` (Host) */
	RGX_HWPERF_HOST_DEBUG_DATA sHDebug;
} RGX_HWPERF_V2_PACKET_DATA, *RGX_PHWPERF_V2_PACKET_DATA;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_V2_PACKET_DATA);

#define RGX_HWPERF_GET_PACKET_DATA(_packet_addr) ((RGX_PHWPERF_V2_PACKET_DATA) (IMG_OFFSET_ADDR((_packet_addr), sizeof(RGX_HWPERF_V2_PACKET_HDR))))

#define RGX_HWPERF_GET_DVFS_EVENT_TYPE_PTR(_packet_addr)	\
	((RGX_HWPERF_DVFS_EV*) (IMG_OFFSET_ADDR((_packet_addr), sizeof(RGX_HWPERF_V2_PACKET_HDR) + offsetof(RGX_HWPERF_DVFS_DATA,eEventType))))

/******************************************************************************
 * API Types
 *****************************************************************************/

/*! Counter block IDs for all the hardware blocks with counters.
 * Directly addressable blocks must have a value between 0..15 [0..0xF].
 * Indirect groups have following encoding:
 * First hex digit (LSB) represents a unit number within the group
 * and the second hex digit represents the group number.
 * Group 0 is the direct group, all others are indirect groups.
 */
typedef IMG_UINT32 RGX_HWPERF_CNTBLK_ID;

/*! Directly addressable non bank-switched counter blocks */
#define RGX_CNTBLK_ID_JONES          0x0000U
#define RGX_CNTBLK_ID_SLC            0x0001U	/*!< SLC-specific counter control */
#define RGX_CNTBLK_ID_FBCDC          0x0002U
#define RGX_CNTBLK_ID_FW_CUSTOM      0x0003U	/*!< Custom FW provided counters */

/*! Directly addressable SLC counter blocks - presence depends on GPU. */
#define RGX_CNTBLK_ID_SLCBANK0       0x0004U	/*!< SLCBANK0 counter control */
#define RGX_CNTBLK_ID_SLCBANK1       0x0005U	/*!< SLCBANK1 counter control */
#define RGX_CNTBLK_ID_SLCBANK2       0x0006U	/*!< SLCBANK2 counter control */
#define RGX_CNTBLK_ID_SLCBANK3       0x0007U	/*!< SLCBANK3 counter control */
#define RGX_CNTBLK_ID_SLCBANK_ALL    0x4004U	/*!< SLC ALL block ID */

#define RGX_CNTBLK_ID_DIRECT_LAST    0x0008U	/*!< Indirect blocks start from here */

/*! Indirectly addressable counter blocks */
#define RGX_CNTBLK_ID_ISP0           0x0010U	/*!< ISP 1..N ISP */
#define RGX_CNTBLK_ID_ISP1           0x0011U
#define RGX_CNTBLK_ID_ISP2           0x0012U
#define RGX_CNTBLK_ID_ISP3           0x0013U
#define RGX_CNTBLK_ID_ISP4           0x0014U
#define RGX_CNTBLK_ID_ISP5           0x0015U
#define RGX_CNTBLK_ID_ISP6           0x0016U
#define RGX_CNTBLK_ID_ISP7           0x0017U
#define RGX_CNTBLK_ID_ISP_ALL        0x4010U

#define RGX_CNTBLK_ID_MERCER0        0x0020U	/*!< MERCER 1..N MERCER */
#define RGX_CNTBLK_ID_MERCER1        0x0021U
#define RGX_CNTBLK_ID_MERCER2        0x0022U
#define RGX_CNTBLK_ID_MERCER3        0x0023U
#define RGX_CNTBLK_ID_MERCER4        0x0024U
#define RGX_CNTBLK_ID_MERCER5        0x0025U
#define RGX_CNTBLK_ID_MERCER6        0x0026U
#define RGX_CNTBLK_ID_MERCER7        0x0027U
#define RGX_CNTBLK_ID_MERCER_ALL     0x4020U

#define RGX_CNTBLK_ID_PBE0           0x0030U	/*!< PBE 1..N PBE_PER_SPU x N SPU */
#define RGX_CNTBLK_ID_PBE1           0x0031U
#define RGX_CNTBLK_ID_PBE2           0x0032U
#define RGX_CNTBLK_ID_PBE3           0x0033U
#define RGX_CNTBLK_ID_PBE4           0x0034U
#define RGX_CNTBLK_ID_PBE5           0x0035U
#define RGX_CNTBLK_ID_PBE6           0x0036U
#define RGX_CNTBLK_ID_PBE7           0x0037U
#define RGX_CNTBLK_ID_PBE_ALL        0x4030U

#define RGX_CNTBLK_ID_PBE_SHARED0    0x0040U	/*!< PBE_SHARED 1..N SPU */
#define RGX_CNTBLK_ID_PBE_SHARED1    0x0041U
#define RGX_CNTBLK_ID_PBE_SHARED2    0x0042U
#define RGX_CNTBLK_ID_PBE_SHARED3    0x0043U
#define RGX_CNTBLK_ID_PBE_SHARED_ALL 0x4040U

#define RGX_CNTBLK_ID_USC0           0x0050U	/*!< USC 1..N USC */
#define RGX_CNTBLK_ID_USC1           0x0051U
#define RGX_CNTBLK_ID_USC2           0x0052U
#define RGX_CNTBLK_ID_USC3           0x0053U
#define RGX_CNTBLK_ID_USC4           0x0054U
#define RGX_CNTBLK_ID_USC5           0x0055U
#define RGX_CNTBLK_ID_USC6           0x0056U
#define RGX_CNTBLK_ID_USC7           0x0057U
#define RGX_CNTBLK_ID_USC_ALL        0x4050U

#define RGX_CNTBLK_ID_TPU0           0x0060U	/*!< TPU 1..N TPU */
#define RGX_CNTBLK_ID_TPU1           0x0061U
#define RGX_CNTBLK_ID_TPU2           0x0062U
#define RGX_CNTBLK_ID_TPU3           0x0063U
#define RGX_CNTBLK_ID_TPU4           0x0064U
#define RGX_CNTBLK_ID_TPU5           0x0065U
#define RGX_CNTBLK_ID_TPU6           0x0066U
#define RGX_CNTBLK_ID_TPU7           0x0067U
#define RGX_CNTBLK_ID_TPU_ALL        0x4060U

#define RGX_CNTBLK_ID_SWIFT0         0x0070U	/*!< SWIFT 1..N SWIFT */
#define RGX_CNTBLK_ID_SWIFT1         0x0071U
#define RGX_CNTBLK_ID_SWIFT2         0x0072U
#define RGX_CNTBLK_ID_SWIFT3         0x0073U
#define RGX_CNTBLK_ID_SWIFT4         0x0074U
#define RGX_CNTBLK_ID_SWIFT5         0x0075U
#define RGX_CNTBLK_ID_SWIFT6         0x0076U
#define RGX_CNTBLK_ID_SWIFT7         0x0077U
#define RGX_CNTBLK_ID_SWIFT_ALL      0x4070U

#define RGX_CNTBLK_ID_TEXAS0         0x0080U	/*!< TEXAS 1..N TEXAS */
#define RGX_CNTBLK_ID_TEXAS1         0x0081U
#define RGX_CNTBLK_ID_TEXAS2         0x0082U
#define RGX_CNTBLK_ID_TEXAS3         0x0083U
#define RGX_CNTBLK_ID_TEXAS_ALL      0x4080U

#define RGX_CNTBLK_ID_RAC0           0x0090U	/*!< RAC 1..N RAC */
#define RGX_CNTBLK_ID_RAC1           0x0091U
#define RGX_CNTBLK_ID_RAC2           0x0092U
#define RGX_CNTBLK_ID_RAC3           0x0093U
#define RGX_CNTBLK_ID_RAC_ALL        0x4090U

#define RGX_CNTBLK_ID_LAST           0x0094U	/*!< End of RAC block */

/*! Masks for the counter block ID*/
#define RGX_CNTBLK_ID_UNIT_MASK      (0x000FU)	/*!< Unit within group */
#define RGX_CNTBLK_ID_GROUP_MASK     (0x00F0U)	/*!< Group value */
#define RGX_CNTBLK_ID_GROUP_SHIFT    (4U)
#define RGX_CNTBLK_ID_MC_GPU_MASK    (0x0F00U)	/*!< GPU ID for MC use */
#define RGX_CNTBLK_ID_MC_GPU_SHIFT   (8U)
#define RGX_CNTBLK_ID_UNIT_ALL_MASK  (0x4000U)	/*!< Program all units within a group */

static_assert(
	((RGX_CNTBLK_ID_DIRECT_LAST + ((RGX_CNTBLK_ID_LAST & RGX_CNTBLK_ID_GROUP_MASK) >> RGX_CNTBLK_ID_GROUP_SHIFT)) <= RGX_HWPERF_MAX_BVNC_BLOCK_LEN),
	"RGX_HWPERF_MAX_BVNC_BLOCK_LEN insufficient");

#define RGX_HWPERF_EVENT_MASK_VALUE(e)      (IMG_UINT64_C(1) << (e))

/* When adding new counters here, make sure changes are made to rgxfw_hwperf_fwblk_valid() as well */
#define RGX_CUSTOM_FW_CNTRS	\
                X(TA_LOCAL_FL_SIZE,    0x0, RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_TAKICK) |         \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_PMOOM_TAPAUSE) |  \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_PMOOM_TARESUME) | \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_TAFINISHED))      \
                                                                                                        \
                X(TA_GLOBAL_FL_SIZE,   0x1, RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_TAKICK) |         \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_PMOOM_TAPAUSE) |  \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_PMOOM_TARESUME) | \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_TAFINISHED))      \
                                                                                                        \
                X(3D_LOCAL_FL_SIZE,    0x2, RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_3DKICK) |         \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_3DFINISHED))      \
                                                                                                        \
                X(3D_GLOBAL_FL_SIZE,   0x3, RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_3DKICK) |         \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_3DFINISHED))      \
                                                                                                        \
                X(ISP_TILES_IN_FLIGHT, 0x4, RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_3DKICK) |         \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_3DSPMKICK))

/*! Counter IDs for the firmware held statistics */
typedef enum
{
#define X(ctr, id, allow_mask)	RGX_CUSTOM_FW_CNTR_##ctr = id,
	RGX_CUSTOM_FW_CNTRS
#undef X

	/* always the last entry in the list */
	RGX_CUSTOM_FW_CNTR_LAST
} RGX_HWPERF_CUSTOM_FW_CNTR_ID;

/*! Identifier for each counter in a performance counting module */
typedef IMG_UINT32 RGX_HWPERF_CNTBLK_COUNTER_ID;

/* sets all the bits from bit _b1 to _b2, in a IMG_UINT64 type */
#define MASK_RANGE_IMPL(b1, b2)	((IMG_UINT64)((IMG_UINT64_C(1) << ((IMG_UINT32)(b2)-(IMG_UINT32)(b1) + 1U)) - 1U) << (IMG_UINT32)b1)
#define MASK_RANGE(R)			MASK_RANGE_IMPL(R##_FIRST_TYPE, R##_LAST_TYPE)
#define RGX_HWPERF_HOST_EVENT_MASK_VALUE(e) (IMG_UINT32_C(1) << (e))

/*! Mask macros for use with RGXCtrlHWPerf() API.
 */
#define RGX_HWPERF_EVENT_MASK_NONE          (IMG_UINT64_C(0x0000000000000000))
#define RGX_HWPERF_EVENT_MASK_DEFAULT       RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_FWACT) | \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_PWR_CHG) | \
                                            RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_CLKS_CHG)
#define RGX_HWPERF_EVENT_MASK_ALL           (IMG_UINT64_C(0xFFFFFFFFFFFFFFFF))

/*! HWPerf Firmware event masks
 * @par
 * All FW Start/End/Debug (SED) events. */
#define RGX_HWPERF_EVENT_MASK_FW_SED    (MASK_RANGE(RGX_HWPERF_FW_EVENT_RANGE))

#define RGX_HWPERF_EVENT_MASK_FW_UFO    (RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_UFO))
#define RGX_HWPERF_EVENT_MASK_FW_CSW    (RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_CSW_START) |\
                                          RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_CSW_FINISHED))
/*! All FW events. */
#define RGX_HWPERF_EVENT_MASK_ALL_FW    (RGX_HWPERF_EVENT_MASK_FW_SED |\
                                          RGX_HWPERF_EVENT_MASK_FW_UFO |\
                                          RGX_HWPERF_EVENT_MASK_FW_CSW)

/*! HW Periodic events (1ms interval). */
#define RGX_HWPERF_EVENT_MASK_HW_PERIODIC   (RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_HW_PERIODIC))
/*! All HW Kick/Finish events. */
#define RGX_HWPERF_EVENT_MASK_HW_KICKFINISH ((MASK_RANGE(RGX_HWPERF_HW_EVENT_RANGE0) |\
                                               MASK_RANGE(RGX_HWPERF_HW_EVENT_RANGE1)) &\
                                              ~(RGX_HWPERF_EVENT_MASK_HW_PERIODIC))

#define RGX_HWPERF_EVENT_MASK_ALL_HW        (RGX_HWPERF_EVENT_MASK_HW_KICKFINISH |\
                                              RGX_HWPERF_EVENT_MASK_HW_PERIODIC)

#define RGX_HWPERF_EVENT_MASK_ALL_PWR_EST   (MASK_RANGE(RGX_HWPERF_PWR_EST_RANGE))

#define RGX_HWPERF_EVENT_MASK_ALL_PWR       (RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_CLKS_CHG) |\
                                              RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_GPU_STATE_CHG) |\
                                              RGX_HWPERF_EVENT_MASK_VALUE(RGX_HWPERF_PWR_CHG))

/*! HWPerf Host event masks
 */
#define RGX_HWPERF_EVENT_MASK_HOST_WORK_ENQ  (RGX_HWPERF_HOST_EVENT_MASK_VALUE(RGX_HWPERF_HOST_ENQ))
#define RGX_HWPERF_EVENT_MASK_HOST_ALL_UFO   (RGX_HWPERF_HOST_EVENT_MASK_VALUE(RGX_HWPERF_HOST_UFO))
#define RGX_HWPERF_EVENT_MASK_HOST_ALL_PWR   (RGX_HWPERF_HOST_EVENT_MASK_VALUE(RGX_HWPERF_HOST_CLK_SYNC))


/*! Type used in the RGX API RGXConfigHWPerfCounters() */
typedef struct
{
	/*! Reserved for future use */
	IMG_UINT32 ui32Reserved;

	/*! Counter block ID, see RGX_HWPERF_CNTBLK_ID */
	IMG_UINT16 ui16BlockID;

	/*! Number of configured counters within this block */
	IMG_UINT16 ui16NumCounters;

	/*! Counter register values */
	IMG_UINT16 ui16Counters[RGX_CNTBLK_COUNTERS_MAX];

} UNCACHED_ALIGN RGX_HWPERF_CONFIG_CNTBLK;

RGX_FW_STRUCT_SIZE_ASSERT(RGX_HWPERF_CONFIG_CNTBLK);

typedef enum
{
	RGX_HWPERF_PFM_ARCH_TYPE_QY2,
	RGX_HWPERF_PFM_ARCH_TYPE_PH1,
} RGX_HWPERF_PFM_ARCH_TYPE;

typedef struct
{
	IMG_BOOL bGetStat;
	IMG_UINT64 ui64Offset;
	IMG_UINT64 ui64BufSize;
} RGX_HWPERF_PFM_GET_STAT_CTRL;

typedef struct
{
	RGX_HWPERF_PFM_GET_STAT_CTRL sGetStatCtrlCMSS;
	RGX_HWPERF_PFM_GET_STAT_CTRL sGetStatCtrlMSSMMU;
	RGX_HWPERF_PFM_GET_STAT_CTRL sGetStatCtrlMSSWrap;
	RGX_HWPERF_PFM_GET_STAT_CTRL sGetStatCtrlGPU;
} RGX_HWPERF_PFM_GET_STAT;

typedef struct
{
	IMG_UINT32 ui32CoreId;
	IMG_UINT32 ui32WrapperSelc;
	IMG_UINT32 ui32InstanceSelc;
	IMG_UINT64 ui64BaseAddrOffset;
	IMG_UINT64 ui64PointerAddrOffset;
} RGX_HWPERF_PFM_INSTANCE_CONFIG_QY2;

typedef struct
{
	IMG_UINT32 ui32CoreId;
	IMG_UINT32 ui32WrapperSelc;
	IMG_UINT32 ui32InstanceEnable;
	IMG_UINT64 ui64GroupSelc;
} RGX_HWPERF_PFM_WRAPPER_CONFIG_QY2;

typedef struct
{
	IMG_BOOL bEnablePFM;
	IMG_UINT32 ui32TimeInterval;
	IMG_UINT32 ui32NumPages;
	IMG_UINT32 ui32PointerUpdateFreq;
} RGX_HWPERF_PFM_GLOBAL_CONFIG_QY2;

#define RGX_HWPERF_PFM_L2PU_DEST_ADDR_COUNT 12
typedef struct
{
	IMG_UINT32 ui32Instance;
	IMG_UINT32 ui32LayoutCtrl;
	IMG_UINT64 ui64CountBaseAddrOffset;
	IMG_UINT64 ui64PointerAddrOffset;
	IMG_UINT64 ui64InstanceCtrl;
	IMG_UINT64 ui64TriggerCtrl;
	IMG_UINT32 aui32Group;
	IMG_UINT64 aui64DestAddr[RGX_HWPERF_PFM_L2PU_DEST_ADDR_COUNT];
} RGX_HWPERF_PFM_INSTANCE_CONFIG_PH1;

#define RGX_PFM_MAX_WRAPPER_COUNT_PER_CORE_PH1 8
typedef struct
{
	IMG_UINT32 ui32CoreId;
	IMG_UINT32 ui32Wrapper;
	IMG_UINT32 ui32Count;
	RGX_HWPERF_PFM_INSTANCE_CONFIG_PH1 apsInstCfgs[RGX_PFM_MAX_WRAPPER_COUNT_PER_CORE_PH1];
} RGX_HWPERF_PFM_WRAPPER_CONFIG_PH1;

typedef struct
{
	IMG_BOOL bEnablePFM;
	IMG_UINT32 ui32BufferCtrl;
	IMG_UINT32 ui32MhCtl;
} RGX_HWPERF_PFM_GLOBAL_CONFIG_PH1;

typedef struct
{
	IMG_HANDLE hPmrHandle;
	IMG_HANDLE hOSMMapHandle;
	IMG_PBYTE pBaseAddr;
	IMG_UINT64 ui64Size;
} RGX_HWPERF_PFM_DESC;

typedef struct
{
	RGX_HWPERF_PFM_ARCH_TYPE eArchType;
	union
	{
		RGX_HWPERF_PFM_INSTANCE_CONFIG_QY2 sPFMInstCfgQY2;
		RGX_HWPERF_PFM_INSTANCE_CONFIG_PH1 sPFMInstCfgPH1;
	} uConfig;
} RGX_HWPERF_PFM_INSTANCE_CONFIG;

typedef struct
{
	RGX_HWPERF_PFM_ARCH_TYPE eArchType;
	union
	{
		RGX_HWPERF_PFM_WRAPPER_CONFIG_QY2 sPFMWrapCfgQY2;
		RGX_HWPERF_PFM_WRAPPER_CONFIG_PH1 sPFMWrapCfgPH1;
	} uConfig;
} RGX_HWPERF_PFM_WRAPPER_CONFIG;

typedef struct
{
	RGX_HWPERF_PFM_ARCH_TYPE eArchType;
	union
	{
		RGX_HWPERF_PFM_GLOBAL_CONFIG_QY2 sPFMGlobalCfgQY2;
		RGX_HWPERF_PFM_GLOBAL_CONFIG_PH1 sPFMGlobalCfgPH1;
	} uConfig;
} RGX_HWPERF_PFM_GLOBAL_CONFIG;

typedef enum
{
        RGX_HWPERF_PFM_MSS_DDRC,
        RGX_HWPERF_PFM_MSS_D2D_0,
        RGX_HWPERF_PFM_MSS_D2D_1,
        RGX_HWPERF_PFM_MSS_LLC,
} RGX_HWPERF_PFM_MSS_TYPE_PH1;

typedef struct
{
        RGX_HWPERF_PFM_MSS_TYPE_PH1 eMSSCfgType;
        IMG_UINT32 MSSCfgOffset;
        IMG_BOOL bMSSEnable;
        IMG_UINT32 ui32TimeSlot;
        IMG_UINT32 ui32OrderEnable;
        IMG_UINT32 ui32DumpFreq;
        IMG_UINT32 ui32DestSpace;
        IMG_UINT32 ui32PointSelc;
        IMG_UINT32 ui32GroupSelc;
        IMG_UINT64 ui64BaseAddrOffset;
        IMG_UINT64 ui64PointAddrOffset;
        IMG_UINT32 ui32LayoutCtrl;
        IMG_UINT32 ui32SpaceOverFlow;
        IMG_UINT32 ui32Absolute;
        IMG_UINT32 ui32TrigSource;
        IMG_UINT32 ui32BroadMode;
        IMG_UINT32 ui32RegMode;
        IMG_UINT32 ui32RegCommand;
} RGX_HWPERF_PFM_MSS_CONFIG_PH1;

typedef struct
{
        RGX_HWPERF_PFM_ARCH_TYPE eArchType;
        IMG_UINT32 ui32Count;
        union
        {
                RGX_HWPERF_PFM_MSS_CONFIG_PH1 *psPFMMSSCfgPH1;
        } uConfig;
} RGX_HWPERF_PFM_MSS_CONFIG;

typedef struct
{
        IMG_UINT32 ui32DumpEnable;
        IMG_UINT32 ui32DumpCmd;
        IMG_UINT32 ui32PFMCtrl0;
} RGX_HWPERF_PFM_CMSS_CONFIG_PH1;

typedef enum
{
	RGX_HWPERF_PFM_DUMP_TRIGGER_GPU,
	RGX_HWPERF_PFM_DUMP_TRIGGER_CMSS,
	RGX_HWPERF_PFM_DUMP_TRIGGER_MSS,
} RGX_HWPERF_PFM_DUMP_TRIGGER_TYPE;

typedef enum
{
	RGX_HWPERF_PFM_NORM_DUMP_START,
	RGX_HWPERF_PFM_NORM_DUMP_ONCE,
	RGX_HWPERF_PFM_NORM_DUMP_END,
	RGX_HWPERF_PFM_INTV_DUMP_START,
	RGX_HWPERF_PFM_INTV_DUMP_END,
} RGX_HWPERF_PFM_DUMP_TRIGGER_CONTROL;

typedef struct
{
	RGX_HWPERF_PFM_MSS_TYPE_PH1 eMSSCfgType;
	IMG_UINT32 MSSCfgOffset;
	RGX_HWPERF_PFM_DUMP_TRIGGER_CONTROL ePFMDumpCtrl;
} RGX_HWPERF_PFM_DUMP_TRIG_MSS_PH1;

typedef struct
{
        RGX_HWPERF_PFM_ARCH_TYPE eArchType;
        union
        {
                RGX_HWPERF_PFM_DUMP_TRIG_MSS_PH1 sPFMDumpTrigMSSPH1;
        } uConfig;
} RGX_HWPERF_PFM_DUMP_TRIG_MSS;

typedef struct
{
	RGX_HWPERF_PFM_DUMP_TRIGGER_TYPE eDumpType;
	union
	{
		RGX_HWPERF_PFM_DUMP_TRIGGER_CONTROL ePFMDumpCtrlGPU;
		RGX_HWPERF_PFM_DUMP_TRIGGER_CONTROL ePFMDumpCtrlCMSS;
		RGX_HWPERF_PFM_DUMP_TRIG_MSS sPFMDumpTrigMSS;
	} uConfig;
} RGX_HWPERF_PFM_DUMP_TRIGGER_CONFIG;

typedef struct
{
	RGX_HWPERF_PFM_ARCH_TYPE eArchType;
	IMG_BOOL bIsSysMemory;
	IMG_UINT64 ui64BufSize;
	IMG_UINT32 ui32PfmCMSSCtrl;
} RGX_HWPERF_PFM_INIT_PARAM;

#if defined(__cplusplus)
}
#endif

#endif /* RGX_HWPERF_H_ */

/******************************************************************************
 End of file
******************************************************************************/
