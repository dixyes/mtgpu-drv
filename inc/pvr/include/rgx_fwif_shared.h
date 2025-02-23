/*************************************************************************/ /*!
@File
@Title          RGX firmware interface structures
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    RGX firmware interface structures shared by both host client
                and host server
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

#if !defined(RGX_FWIF_SHARED_H)
#define RGX_FWIF_SHARED_H

#include "img_types.h"
#include "img_defs.h"
#include "rgx_common.h"
#include "powervr/mem_types.h"

/* Indicates the number of RTDATAs per RTDATASET */
#if defined(SUPPORT_AGP)
#if defined(SUPPORT_AGP4)
#define RGXMKIF_NUM_RTDATAS           4U
#define RGXMKIF_NUM_GEOMDATAS         4U
#define RGXMKIF_NUM_RTDATA_FREELISTS  20U /* RGXMKIF_NUM_RTDATAS * RGXFW_MAX_FREELISTS */
#define RGX_NUM_GEOM_CORES           (4U)
#else
#define RGXMKIF_NUM_RTDATAS           4U
#define RGXMKIF_NUM_GEOMDATAS         4U
#define RGXMKIF_NUM_RTDATA_FREELISTS  12U /* RGXMKIF_NUM_RTDATAS * RGXFW_MAX_FREELISTS */
#define RGX_NUM_GEOM_CORES           (2U)
#endif
#else
#define RGXMKIF_NUM_RTDATAS           2U
#define RGXMKIF_NUM_GEOMDATAS         1U
#define RGXMKIF_NUM_RTDATA_FREELISTS  16U  /* RGX_MAX_NUM_CORES * RGXFW_MAX_FREELISTS */
#define RGX_NUM_GEOM_CORES           (1U)
#endif

/* Maximum number of UFOs in a CCB command.
 * The number is based on having 32 sync prims (as originally), plus 32 sync
 * checkpoints.
 * Once the use of sync prims is no longer supported, we will retain
 * the same total (64) as the number of sync checkpoints which may be
 * supporting a fence is not visible to the client driver and has to
 * allow for the number of different timelines involved in fence merges.
 */
#define RGXFWIF_CCB_CMD_MAX_UFOS			(32U+32U)

/*
 * This is a generic limit imposed on any DM (TA,3D,CDM,TDM,2D,TRANSFER)
 * command passed through the bridge.
 * Just across the bridge in the server, any incoming kick command size is
 * checked against this maximum limit.
 * In case the incoming command size is larger than the specified limit,
 * the bridge call is retired with error.
 * 
 * The size should be less than 170K theoretically.
 */
#define RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE	(256 * 1024U)

typedef struct RGXFWIF_DEV_VIRTADDR_
{
	IMG_UINT32	ui32Addr;
} RGXFWIF_DEV_VIRTADDR;

typedef struct
{
	IMG_DEV_VIRTADDR        RGXFW_ALIGN psDevVirtAddr;
	RGXFWIF_DEV_VIRTADDR    pbyFWAddr;
} UNCACHED_ALIGN RGXFWIF_DMA_ADDR;

typedef IMG_UINT8	RGXFWIF_CCCB;

typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_UFO_ADDR;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_CLEANUP_CTL;


/*!
 * @InGroup ClientCCBTypes
 * @Brief Command data for fence & update types Client CCB commands.
 */
typedef struct
{
	PRGXFWIF_UFO_ADDR	puiAddrUFO; /*!< Address to be checked/updated */
	IMG_UINT32			ui32Value;  /*!< Value to check-against/update-to */
} RGXFWIF_UFO;

typedef struct
{
	IMG_UINT32			ui32SubmittedCommands;	/*!< Number of commands received by the FW */
	IMG_UINT32			ui32ExecutedCommands;	/*!< Number of commands executed by the FW */
} UNCACHED_ALIGN RGXFWIF_CLEANUP_CTL;

#define	RGXFWIF_PRBUFFER_START        IMG_UINT32_C(0)
#define	RGXFWIF_PRBUFFER_ZSBUFFER     IMG_UINT32_C(0)
#define	RGXFWIF_PRBUFFER_MSAABUFFER   IMG_UINT32_C(1)
#define	RGXFWIF_PRBUFFER_MAXSUPPORTED IMG_UINT32_C(2)

typedef IMG_UINT32 RGXFWIF_PRBUFFER_TYPE;

typedef enum
{
	RGXFWIF_PRBUFFER_UNBACKED = 0,
	RGXFWIF_PRBUFFER_BACKED,
	RGXFWIF_PRBUFFER_BACKING_PENDING,
	RGXFWIF_PRBUFFER_UNBACKING_PENDING,
}RGXFWIF_PRBUFFER_STATE;

/*!
 * @InGroup RenderTarget
 * @Brief OnDemand Z/S/MSAA Buffers
 */
typedef struct
{
	IMG_UINT32				ui32BufferID;				/*!< Buffer ID*/
	IMG_BOOL				bOnDemand;					/*!< Needs On-demand Z/S/MSAA Buffer allocation */
	RGXFWIF_PRBUFFER_STATE	eState;						/*!< Z/S/MSAA -Buffer state */
	RGXFWIF_CLEANUP_CTL		sCleanupState;				/*!< Cleanup state */
	IMG_UINT32				ui32PRBufferFlags;		/*!< Compatibility and other flags */
} UNCACHED_ALIGN RGXFWIF_PRBUFFER;

/*
 * Used to share frame numbers across UM-KM-FW,
 * frame number is set in UM,
 * frame number is required in both KM for HTB and FW for FW trace.
 *
 * May be used to house Kick flags in the future.
 */
typedef struct
{
	IMG_UINT32 ui32FrameNum; /*!< associated frame number */
} CMD_COMMON;

/*
 * TA and 3D commands require set of firmware addresses that are stored in the
 * Kernel. Client has handle(s) to Kernel containers storing these addresses,
 * instead of raw addresses. We have to patch/write these addresses in KM to
 * prevent UM from controlling FW addresses directly.
 * Typedefs for TA and 3D commands are shared between Client and Firmware (both
 * single-BVNC). Kernel is implemented in a multi-BVNC manner, so it can't use
 * TA|3D CMD type definitions directly. Therefore we have a SHARED block that
 * is shared between UM-KM-FW across all BVNC configurations.
 */
typedef struct
{
	CMD_COMMON           sCmn;      /*!< Common command attributes */
	RGXFWIF_DEV_VIRTADDR sHWRTData; /* RTData associated with this command,
									   this is used for context selection and for storing out HW-context,
									   when TA is switched out for continuing later */

	RGXFWIF_DEV_VIRTADDR asPRBuffer[RGXFWIF_PRBUFFER_MAXSUPPORTED];	/* Supported PR Buffers like Z/S/MSAA Scratch */

} CMDTA3D_SHARED;

/*!
 * Client Circular Command Buffer (CCCB) control structure.
 * This is shared between the Server and the Firmware and holds byte offsets
 * into the CCCB as well as the wrapping mask to aid wrap around. A given
 * snapshot of this queue with Cmd 1 running on the GPU might be:
 *
 *          Roff                           Doff                 Woff
 * [..........|-1----------|=2===|=3===|=4===|~5~~~~|~6~~~~|~7~~~~|..........]
 *            <      runnable commands       ><   !ready to run   >
 *
 * Cmd 1    : Currently executing on the GPU data master.
 * Cmd 2,3,4: Fence dependencies met, commands runnable.
 * Cmd 5... : Fence dependency not met yet.
 */
typedef struct
{
	IMG_UINT32  ui32WriteOffset;    /*!< Host write offset into CCB. This
	                                 *    must be aligned to 16 bytes. */
	IMG_UINT32  ui32ReadOffset;     /*!< Firmware read offset into CCB.
	                                      Points to the command that is
	                                      runnable on GPU, if R!=W */
	IMG_UINT32  ui32DepOffset;      /*!< Firmware fence dependency offset.
	                                      Points to commands not ready, i.e.
	                                      fence dependencies are not met. */
	IMG_UINT32  ui32WrapMask;       /*!< Offset wrapping mask, total capacity
	                                      in bytes of the CCB-1 */

	/* Used for CDM multiqueue. */
	IMG_UINT32  ui32ReadOffset2;
	IMG_UINT32  ui32ReadOffset3;
	IMG_UINT32  ui32ReadOffset4;

	/* Only used for linux guest hwr */
#if (RGX_NUM_OS_SUPPORTED > 1)
	IMG_UINT32 ui32WriteOffsetBackup;
	IMG_UINT32 ui32ReadOffsetBackup;
	IMG_UINT32 ui32DepOffsetBackup;
	IMG_UINT32 ui32LastReadOffsetNeedSkip;
	IMG_BOOL   bNeedSkip;
#endif
} UNCACHED_ALIGN RGXFWIF_CCCB_CTL;


typedef IMG_UINT32 RGXFW_FREELIST_TYPE;

#define RGXFW_LOCAL_FREELIST     IMG_UINT32_C(0)
#define RGXFW_GLOBAL_FREELIST    IMG_UINT32_C(1)
#if defined(SUPPORT_AGP4)
#define RGXFW_GLOBAL2_FREELIST   IMG_UINT32_C(2)
#define RGXFW_GLOBAL3_FREELIST   IMG_UINT32_C(3)
#define RGXFW_GLOBAL4_FREELIST   IMG_UINT32_C(4)
#define RGXFW_MAX_FREELISTS      (RGXFW_GLOBAL4_FREELIST + 1U)
#elif defined(SUPPORT_AGP)
#define RGXFW_GLOBAL2_FREELIST   IMG_UINT32_C(2)
#define RGXFW_MAX_FREELISTS      (RGXFW_GLOBAL2_FREELIST + 1U)
#else
#define RGXFW_LOCALMCG2_FREELIST  IMG_UINT32_C(2)
#define RGXFW_LOCALMCG3_FREELIST  IMG_UINT32_C(3)
#define RGXFW_LOCALMCG4_FREELIST  IMG_UINT32_C(4)
#define RGXFW_LOCALMCG5_FREELIST  IMG_UINT32_C(5)
#define RGXFW_LOCALMCG6_FREELIST  IMG_UINT32_C(6)
#define RGXFW_LOCALMCG7_FREELIST  IMG_UINT32_C(7)
#define RGXFW_LOCALMCG8_FREELIST  IMG_UINT32_C(8)
#define RGXFW_GLOBALMCG2_FREELIST IMG_UINT32_C(9)
#define RGXFW_GLOBALMCG3_FREELIST IMG_UINT32_C(10)
#define RGXFW_GLOBALMCG4_FREELIST IMG_UINT32_C(11)
#define RGXFW_GLOBALMCG5_FREELIST IMG_UINT32_C(12)
#define RGXFW_GLOBALMCG6_FREELIST IMG_UINT32_C(13)
#define RGXFW_GLOBALMCG7_FREELIST IMG_UINT32_C(14)
#define RGXFW_GLOBALMCG8_FREELIST IMG_UINT32_C(15)

#define RGXFW_MAX_FREELISTS   (RGXFW_GLOBALMCG8_FREELIST + 1U)
#endif
#define RGXFW_MAX_HWFREELISTS    (2U)

/*!
 * @Defgroup ContextSwitching Context switching data interface
 * @Brief Types grouping data structures and defines used in realising the Context Switching (CSW) functionality
 * @{
 */

/*!
 * @Brief GEOM DM or TA register controls for context switch
 */
typedef struct
{
	IMG_UINT64	uTAReg_DCE_CONTEXT_STATE_BASE_ADDR;
	IMG_UINT64	uTAReg_TA_CONTEXT_STATE_BASE_ADDR; /*!< The base address of the TA's context state buffer */

	struct
	{
		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_VDM0;
		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_VDM1;
		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_VDM2;

		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_DDM0;
		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_DDM1;
		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_DDM2;
		IMG_UINT64	uTAReg_DCE_CONTEXT_STORE_TASK_XFB;

		/* VDM resume state update controls */
		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_VDM0;
		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_VDM1;
		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_VDM2;


		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_DDM0;
		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_DDM1;
		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_DDM2;
		IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_XFB;
	} asTAState[2];

} RGXFWIF_TAREGISTERS_CSWITCH;
/*! @} End of Defgroup ContextSwitching */

typedef struct
{
	IMG_UINT64	u3DReg_IPP_CONTEXT_ADDR;
} RGXFWIF_3DREGISTERS_CSWITCH;

typedef struct
{
	IMG_UINT64	uCDMReg_CDM_CONTEXT_PDS0;
	IMG_UINT64	uCDMReg_CDM_CONTEXT_PDS1;
	IMG_UINT64	uCDMReg_CDM_TERMINATE_PDS;
	IMG_UINT64	uCDMReg_CDM_TERMINATE_PDS1;

	/* CDM resume controls */
	IMG_UINT64	uCDMReg_CDM_RESUME_PDS0;
	IMG_UINT64	uCDMReg_CDM_CONTEXT_PDS0_B;
	IMG_UINT64	uCDMReg_CDM_RESUME_PDS0_B;

} RGXFWIF_CDM_REGISTERS_CSWITCH;

static_assert((sizeof(RGXFWIF_CDM_REGISTERS_CSWITCH) % 8U) == 0U,
			"the size of the structure must be multiple of 8");

#define RGXFWIF_CDM_REGISTERS_CSWITCH_SIZE sizeof(RGXFWIF_CDM_REGISTERS_CSWITCH)

/*!
 * @InGroup ContextSwitching
 * @Brief Render context static register controls for context switch
 */
typedef struct
{
	RGXFWIF_TAREGISTERS_CSWITCH RGXFW_ALIGN asCtxSwitch_GeomRegs[RGX_NUM_GEOM_CORES];
	RGXFWIF_3DREGISTERS_CSWITCH RGXFW_ALIGN sCtxSwitch_3DRegs;  /*!< 3D registers for ctx switch */
} RGXFWIF_STATIC_RENDERCONTEXT_STATE;

#define RGXFWIF_STATIC_RENDERCONTEXT_SIZE sizeof(RGXFWIF_STATIC_RENDERCONTEXT_STATE)

typedef struct
{
	RGXFWIF_CDM_REGISTERS_CSWITCH	RGXFW_ALIGN sCtxSwitch_Regs;	/*!< CDM registers for ctx switch */
} RGXFWIF_STATIC_COMPUTECONTEXT_STATE;

#define RGXFWIF_STATIC_COMPUTECONTEXT_SIZE sizeof(RGXFWIF_STATIC_COMPUTECONTEXT_STATE)

typedef struct
{
	IMG_UINT64	uRDMReg_RDM_CONTEXT_STATE_BASE_ADDR;
} RGXFWIF_RDM_REGISTERS_CSWITCH;

static_assert((sizeof(RGXFWIF_RDM_REGISTERS_CSWITCH) % 8U) == 0U,
			"the size of the structure must be multiple of 8");

#define RGXFWIF_RDM_REGISTERS_CSWITCH_SIZE sizeof(RGXFWIF_RDM_REGISTERS_CSWITCH)

typedef struct
{
	RGXFWIF_RDM_REGISTERS_CSWITCH	RGXFW_ALIGN sCtxSwitch_Regs;	/*!< RDM registers for ctx switch */
} RGXFWIF_STATIC_RAYCONTEXT_STATE;

#define RGXFWIF_STATIC_RAYCONTEXT_SIZE sizeof(RGXFWIF_STATIC_RAYCONTEXT_STATE)

/*!
	@Brief Context reset reason. Last reset reason for a reset context.
*/
#define    RGX_CONTEXT_RESET_REASON_NONE                     0x000  /*!< No reset reason recorded */
#define    RGX_CONTEXT_RESET_REASON_GUILTY_LOCKUP            0x001  /*!< Caused a reset due to locking up */
#define    RGX_CONTEXT_RESET_REASON_INNOCENT_LOCKUP          0x002  /*!< Affected by another context locking up */
#define    RGX_CONTEXT_RESET_REASON_GUILTY_OVERRUNING        0x004  /*!< Overran the global deadline */
#define    RGX_CONTEXT_RESET_REASON_INNOCENT_OVERRUNING      0x008  /*!< Affected by another context overrunning */
#define    RGX_CONTEXT_RESET_REASON_HARD_CONTEXT_SWITCH      0x010  /*!< Forced reset to ensure scheduling requirements */
#define    RGX_CONTEXT_RESET_REASON_WGP_CHECKSUM             0x020  /*!< CDM Mission/safety checksum mismatch */
#define    RGX_CONTEXT_RESET_REASON_TRP_CHECKSUM             0x040  /*!< TRP checksum mismatch */
#define    RGX_CONTEXT_RESET_REASON_GPU_ECC_OK               0x080  /*!< GPU ECC error (corrected, OK) */
#define    RGX_CONTEXT_RESET_REASON_GPU_ECC_HWR              0x100  /*!< GPU ECC error (uncorrected, HWR) */
#define    RGX_CONTEXT_RESET_REASON_FW_ECC_OK                0x200 /*!< FW ECC error (corrected, OK) */
#define    RGX_CONTEXT_RESET_REASON_FW_ECC_ERR               0x400 /*!< FW ECC error (uncorrected, ERR) */
#define    RGX_CONTEXT_RESET_REASON_FW_WATCHDOG              0x800 /*!< FW Safety watchdog triggered */
#define    RGX_CONTEXT_RESET_REASON_FW_PAGEFAULT             0x001000 /*!< FW page fault (no HWR) */
#define    RGX_CONTEXT_RESET_REASON_FW_EXEC_ERR              0x002000 /*!< FW execution error (GPU reset requested) */
#define    RGX_CONTEXT_RESET_REASON_HOST_WDG_FW_ERR          0x004000 /*!< Host watchdog detected FW error */
#define    RGX_CONTEXT_GEOM_OOM_DISABLED                     0x008000 /*!< Geometry DM OOM event is not allowed */
#define    MT_MP_EXCEPTION_WRITE                             0x010000 /*!< ICTRL/RTU/LSU/TCE/MOV */
#define    MT_MP_EXCEPTION_READ                              0x020000 /*!< ICTRL */
#define    MT_MP_EXCEPTION_US_RW_CONFLICT                    0x040000 /*!< US */
#define    MT_MP_EXCEPTION_LMS_RW_NOBARRIER                  0x100000 /*!< LMS */
#define    MT_MP_EXCEPTION_LMS_OOB                           0x200000 /*!< LMA/TME access lms oob */
#define    MT_MP_EXCEPTION_ILLEGAL_INSTRUCTION               0x400000 /*!< ICTRL catch decode error */
#define    MT_MP_EXCEPTION_INVALID_PC                        0x00800000 /*!< ICTRL get invalid pc  */
#define    MT_MP_EXCEPTION_INVALID_MEMORY_SPACE              0x01000000 /*!< LMA: address target to unsupported memory space for pipeline */
#define    MT_MP_EXCEPTION_MISALIGNED_ADDRESS                0x02000000 /*!< LMA/TME/LSU misaligned address */
#define    MT_MP_EXCEPTION_ASYNC_TRANS_BAR_SIZE_OOB          0x04000000 /*!< TME/ASYNC: bar id is greater than bar size */
#define    MT_MP_EXCEPTION_ASYNC_TRANS_BAR_TRANS_OOB         0x08000000 /*!< ASYNC: transaction count is overflow or underflow */
#define    MT_MP_EXCEPTION_ASYNC_TRANS_BAR_ARRIVAL_OOB       0x10000000 /*!< ASYNC: arrival count is overflow or underflow */
#define    MT_MP_EXCEPTION_TCE_ILLEGAL_ACTIVE_MASK           0x20000000 /*!< TCE: detect illegal active mask */
#define    MT_MP_EXCEPTION_TME_ILLEGAL_PARAMETERS            0x40000000 /*!< TME: detect illegal parameters */
#define    RGX_CONTEXT_ACE_STATUS_ERR                        0x80000000 /*!< AXI bus status error */

typedef struct
{
	/* FIXME: To ensure the normal compilation of other modules in gr-umd, we temporarily need to
	 *        use hard core to define the dimensions of the USC_SLOT registers, the reason is that
	 *        using macro definitions could lead to compilation failures in other modules.
	 *        The first dimension is RGX_MAX_NUM_CORES, the second is RGX_FEATURE_NUM_CLUSTERS,
	 *        the last is RGX_FEATURE_USC_SLOTS. */
	/* TODO: Move the definition of RGX_CONTEXT_RESET_REASON_DATA to shared_include, and replace
	 *       the hard code by macro definitions */
	IMG_UINT32 ui32UscWatchDogStatus[8];
	IMG_UINT64 ui64UscSlot[8][8][96];
} USC_DUMP;

typedef struct
{
	IMG_UINT64 ui64Addr;
	IMG_UINT64 bfPC : 32;
	IMG_UINT64 bfRead : 1;
	IMG_UINT64 bfFault : 1;
	IMG_UINT64 bfROFault : 1;
	IMG_UINT64 bfProtFault : 1;
	IMG_UINT64 bfPageFault : 1;
	IMG_CHAR pszTagID[16];
	IMG_CHAR pszMMULevel[16];
	IMG_CHAR pszTagSB[32];
} PAGE_FAULT_INFO;

typedef struct
{
	IMG_UINT64 ui64MmuFaultStatus1[8];
	IMG_UINT64 ui64MmuFaultStatus2[8];
	PAGE_FAULT_INFO sPageFaultPrint[8];
} MMU_FAULT_DUMP;

/*!
	@Brief Context reset data shared with the host
*/
typedef struct
{
	IMG_UINT64 ui64ResetReason;       /*!< Reset reason */
	IMG_UINT32 ui32ResetExtJobRef;    /*!< External Job ID */
	IMG_BOOL bIsPageFault;
	IMG_UINT64 ui64TimeStamp;
	union CONTEXT_RESET_INFO
	{
		USC_DUMP sUscDump;
		MMU_FAULT_DUMP sMmuFault;
	} sCtxResetInfo;
} RGX_CONTEXT_RESET_REASON_DATA;
#endif /*  RGX_FWIF_SHARED_H */

/******************************************************************************
 End of file (rgx_fwif_shared.h)
******************************************************************************/
