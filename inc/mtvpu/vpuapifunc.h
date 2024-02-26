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

#ifndef VPUAPI_UTIL_H_INCLUDED
#define VPUAPI_UTIL_H_INCLUDED

#include "vpuapi.h"

// COD_STD
enum {
    AVC_DEC  = 0,
    VC1_DEC  = 1,
    MP2_DEC  = 2,
    MP4_DEC  = 3,
    DV3_DEC  = 3,
    RV_DEC   = 4,
    AVS_DEC  = 5,
    VPX_DEC  = 7,
    MAX_DEC  = 7,
    AVC_ENC  = 8,
    MP4_ENC  = 11,
    MAX_CODECS,
};

// new COD_STD since WAVE series
enum {
    W_HEVC_DEC  = 0x00,
    W_HEVC_ENC  = 0x01,
    W_AVC_DEC   = 0x02,
    W_AVC_ENC   = 0x03,
    W_VP9_DEC   = 0x16,
    W_AVS2_DEC  = 0x18,
    W_AV1_DEC   = 0x1A,
    W_AV1_ENC	= 0x1B,
    STD_UNKNOWN = 0xFF
};

// AUX_COD_STD
enum {
    MP4_AUX_MPEG4 = 0,
    MP4_AUX_DIVX3 = 1
};

enum {
    VPX_AUX_THO = 0,
    VPX_AUX_VP6 = 1,
    VPX_AUX_VP8 = 2,
    VPX_AUX_NUM
};

enum {
    AVC_AUX_AVC = 0,
    AVC_AUX_MVC = 1
};

// BIT_RUN command
enum {
    DEC_SEQ_INIT         = 1,
    ENC_SEQ_INIT         = 1,
    DEC_SEQ_END          = 2,
    ENC_SEQ_END          = 2,
    PIC_RUN              = 3,
    SET_FRAME_BUF        = 4,
    ENCODE_HEADER        = 5,
    ENC_PARA_SET         = 6,
    DEC_PARA_SET         = 7,
    DEC_BUF_FLUSH        = 8,
    ENC_CHANGE_PARAMETER = 9,
    VPU_SLEEP            = 10,
    VPU_WAKE             = 11,
    ENC_ROI_INIT         = 12,
    FIRMWARE_GET         = 0xf
};

enum {
    SRC_BUFFER_EMPTY        = 0,    //!< source buffer doesn't allocated.
    SRC_BUFFER_ALLOCATED    = 1,    //!< source buffer has been allocated.
    SRC_BUFFER_SRC_LOADED   = 2,    //!< source buffer has been allocated.
    SRC_BUFFER_USE_ENCODE   = 3     //!< source buffer was sent to VPU. but it was not used for encoding.
};

enum {
    FramebufCacheNone,
    FramebufCacheMaverickI,
    FramebufCacheMaverickII,
};

#ifndef MAX
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#define HEVC_MAX_SUB_LAYER_ID   6
#define AVS2_MAX_SUB_LAYER_ID   7
#define DECODE_ALL_TEMPORAL_LAYERS  -1
#define DECODE_ALL_SPATIAL_LAYERS   -1

//#define API_DEBUG

#ifdef API_DEBUG
#ifdef _MSC_VER
#define APIDPRINT(_fmt, ...)            VLOG(INFO, _fmt, __VA_ARGS__)
#else
#define APIDPRINT(_fmt, ...)            VLOG(INFO, _fmt, ##__VA_ARGS__)
#endif
#else
#define APIDPRINT(_fmt, ...)
#endif

extern Uint32 __VPU_BUSY_TIMEOUT;
extern VpuAttr g_VpuCoreAttributes[MAX_NUM_VPU_CORE];

/**
 * PRODUCT: CODA9/WAVE5/WAVE6
 */
typedef struct {
    union {
        struct {
            int             useBitEnable;
            int             useIpEnable;
            int             useDbkYEnable;
            int             useDbkCEnable;
            int             useOvlEnable;
            int             useBtpEnable;
            PhysicalAddress bufBitUse;
            PhysicalAddress bufIpAcDcUse;
            PhysicalAddress bufDbkYUse;
            PhysicalAddress bufDbkCUse;
            PhysicalAddress bufOvlUse;
            PhysicalAddress bufBtpUse;
        } coda9;
        struct {
            //Decoder
            int             useBitEnable;
            int             useIpEnable;
            int             useLfRowEnable;
            int             useSclEnable;
            //Encoder
            int             useEncRdoEnable;
            int             useEncLfEnable;

        } wave;
    } u;
    int             bufSize;
    PhysicalAddress bufBase;
} SecAxiInfo;

typedef struct _tag_FramebufferIndex {
    Int16 tiledIndex;
    Int16 linearIndex;
} FramebufferIndex;

typedef struct {
    DecOpenParam    openParam;
    DecInitialInfo  initialInfo;
    DecInitialInfo  newSeqInfo;     /* Temporal new sequence information */
    PhysicalAddress streamWrPtr;
    PhysicalAddress streamRdPtr;
    int             streamEndflag;
    int             frameDisplayFlag;
    int             clearDisplayIndexes;
    int             setDisplayIndexes;
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    PhysicalAddress frameDisplayFlagRegAddr;
    PhysicalAddress currentPC;
    PhysicalAddress busyFlagAddr;
    PhysicalAddress rootPageTableStartAddr;
    PhysicalAddress rootPageTableEndAddr;
    int             streamBufSize;
    FrameBuffer     frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t    vbFrame;
    vpu_buffer_t    vbWTL;
    vpu_buffer_t    vbPPU;
    vpu_buffer_t    vbMV[MAX_REG_FRAME];
    vpu_buffer_t    vbFbcYTbl[MAX_REG_FRAME];
    vpu_buffer_t    vbFbcCTbl[MAX_REG_FRAME];
    int             frameAllocExt;
    int             ppuAllocExt;
    int             numFrameBuffers;
    int             numFbsForDecoding;                  /*!<< number of framebuffers used in decoding */
    int             numFbsForWTL;                       /*!<< number of linear framebuffer for displaying when DecInfo::wtlEnable is set to 1 */
    int             stride;
    int             frameBufferHeight;
    int             rotationEnable;
    int             mirrorEnable;
    int             deringEnable;
    MirrorDirection mirrorDirection;
    int             rotationAngle;
    FrameBuffer     rotatorOutput;
    int             rotatorStride;
    int             rotatorOutputValid;
    int             initialInfoObtained;
    int             vc1BframeDisplayValid;
    int             mapType;
    int             tiled2LinearEnable;
    int             tiled2LinearMode;
    int             wtlEnable;
    int             wtlMode;
    FrameBufferFormat   wtlFormat;                      /*!<< default value: FORMAT_420 8bit */
    SecAxiInfo          secAxiInfo;
    MaverickCacheConfig cacheConfig;
    int seqInitEscape;

    // Report Information
    PhysicalAddress userDataBufAddr;
    vpu_buffer_t    vbUserData;

    Uint32          userDataEnable;                    /* User Data Enable Flag
                                                          CODA9xx: TRUE or FALSE
                                                          WAVExxx: Refer to H265_USERDATA_FLAG_xxx values in vpuapi.h */
    int             userDataBufSize;
    int             userDataReportMode;                // User Data report mode (0 : interrupt mode, 1 interrupt disable mode)


    LowDelayInfo    lowDelayInfo;
    Uint64          frameStartPos;
    Uint64          frameEndPos;
    int             frameDelay;
    vpu_buffer_t    vbSlice;            // AVC, VP8 only
    vpu_buffer_t    vbWork;
    vpu_buffer_t    vbTemp;
    vpu_buffer_t    vbReport;
    vpu_buffer_t    vbTask;
    DecOutputInfo   decOutInfo[MAX_GDI_IDX];
    TiledMapConfig  mapCfg;
    int             reorderEnable;
    Uint32          avcErrorConcealMode;
    DRAMConfig      dramCfg;            //coda960 only
    int             thumbnailMode;
    int             seqChangeMask;      // WAVE410
    Uint32          mmuCtxId;
    BOOL            mmuEnable;
    BOOL            scalerEnable;
    Uint32          scaleWidth;
    Uint32          scaleHeight;
    TemporalIdMode  tempIdSelectMode;
    Int32           targetTempId;
    Int32           targetSpatialId;    // AV1 only
    Int32           instanceQueueCount;
    Int32           reportQueueCount;
    Uint32          firstCycleCheck;
    Uint32          cyclePerTick;
    Uint32          productCode;
    Uint32          vlcBufSize;
    Uint32          paramBufSize;
    vpu_buffer_t    vbVaapiParam;
    vpu_buffer_t    vbDecParam;
    Uint32          vaWidth;
    Uint32          vaHeight;
    Uint32          picSize4Linear;
    Uint32          picInfo4Linear;
    Uint32          picSize4Compress;
    Uint32          picInfo4Compress;
    Uint32          waitFor2ndField;
} DecInfo;

#define CODA9_AVC_Q_MATRIX_OFFSET       (0x3500)
#define CODA9_AVC_DIRECT_MEM_OFFSET     (0x10*2)
#define CODA9_MP4_INDIRECT_MEM_OFFSET   (0x330*2)
#define CODA9_MP4_DIRECT_MEM_OFFSET     (0x8*2)
#define CODA9_MP2_INDIRECT_MEM_OFFSET   (0x400*2)
#define CODA9_MP2_DIRECT_MEM_OFFSET     (0x24*2)
#define CODA9_VC1_DIRECT_MEM_OFFSET     (0x10*2)
#define CODA9_AVS_DIRECT_MEM_OFFSET     (0x10*2)
#define CODA9_AVS_INDIRECT_MEM_OFFSET   (0x600*2)
#define CODA9_VP8_DIRECT_MEM_OFFSET     (0x8*2)

#pragma pack(push, 1)
typedef struct {
//------------------------------------------------------------------------------
//    SPS/PPS/SLICE HEADER VARIABLES
//------------------------------------------------------------------------------
    Uint16 dSeqParaSetId;           //10h
    Uint16 dProfileIdc;             //11h
    Uint16 dMaxRefFrame;            //12h
    Uint16 dMaxFrameBit;            //13h
    Uint16 dMaxFrameNumH;           //14h
    Uint16 dMaxFrameNumL;           //15h
    Uint16 dPicOrderCntType;        //16h
    Uint16 dPicOrderCntBit;         //17h
    Uint16 dPicOrderZeroFlag;       //18h
    Uint16 dNumRefFrame;            //19h
    Uint16 dGapsInFrameNum;         //1Ah
    Uint16 dFrameMbsOnlyFlag;       //1Bh
    Uint16 dMbAffFlag;              //1Ch
    Uint16 dDirect8x8Flag;          //1Dh
    Uint16 dMbAddrH;                //1Eh
    Uint16 dLevelIdc;               //1Fh
    Uint16 dMaxDpbSize;             //20h
    Uint16 dReorderEnable;          //21h
    Uint16 dChromaIdc;              //22h
    Uint16 dPicStructFlag;          //23h
    Uint16 dConstSetflag7_0;        //24h
    Uint16 dAspectRatioH;           //25h
    Uint16 dAspectRatioL;           //26h
    Uint16 dBitRateH;               //27h
    Uint16 dBitRateL;               //28h
    Uint16 dPicParaSetId;           //29h
    Uint16 dPicOrderPrsFlag;        //2Ah
    Uint16 dNumSliceGrpMinus1;      //2Bh
    Uint16 dPicNumRefIdxL0Minus1;   //2Ch
    Uint16 dUseCabac;               //2Dh
    Uint16 dPicNumRefIdxL1Minus1;   //2Eh
    Uint16 dWpFlag;                 //2Fh
    Uint16 dWpIdc;                  //30h
    Uint16 dPicInitQpY;             //31h
    Uint16 dChromaQpOffset;         //32h
    Uint16 dDeblkCtrlPresent;       //33h
    Uint16 dConstrainIntra;         //34h
    Uint16 dRedundPicCntPresent;    //35h
    Uint16 dSecondChromaQpOffset;   //36h
    Uint16 dTransform8x8ModeFlag;   //37h
    Uint16 dSliceType;              //38h
    Uint16 dFrameNum;               //39h
    Uint16 dFirstFieldExist;        //3Ah
    Uint16 dFirstFieldNalRefFlag;   //3Bh
    Uint16 dFirstFieldFrameNum;     //3Ch
    Uint16 dFirstFieldBottom;       //3Dh
    Uint16 dFirstFieldFrameIdx;     //3Eh
    Uint16 dFieldPicFlag;           //3Fh
    Uint16 dSecondField;            //40h
    Uint16 dPicType;                //41h
    Uint16 dBottomFieldFlag;        //42h
    Uint16 dMbAff;                  //43h
    Uint16 dMbNumPicL;              //44h
    Uint16 dIdrPicId;               //45h
    Uint16 dDirectMvMode;           //46h
    Uint16 dCabacInitIdc;           //47h
    Uint16 dNumRefIdxL0Minus1;      //48h
    Uint16 dNumRefIdxL1Minus1;      //49h
    Uint16 dSliceQpY;               //4Ah
    Uint16 dDisableDeblk;           //4Bh
    Uint16 dDeblkOffsetA;           //4Ch
    Uint16 dDeblkOffsetB;           //4Dh

//------------------------------------------------------------------------------
//    PICTURE VARIABLES
//------------------------------------------------------------------------------
    Uint16 dNalRefIdc;              //4Eh
    Uint16 dIdrPicture;             //4Fh
    Uint16 dCurFrameNum;            //50h
    Uint16 dFirstSlice;             //51h
    Uint16 dMbNumPicH;              //52h
    Uint16 dDummy;                  //53h
    Uint16 dDecFrameNumH;           //54h
    Uint16 dDecFrameNumL;           //55h
    Uint16 dPrevDispPicIdx;         //56h

//------------------------------------------------------------------------------
//    MB VARIABLES
//------------------------------------------------------------------------------
    Uint16 dMbNumX;                 //57h
    Uint16 dMbNumY;                 //58h
    Uint16 dMbAddrL;                //59h
    Uint16 dMbIntra;                //5Ah
    Uint16 dMbPosX;                 //5Bh
    Uint16 dMbPosY;                 //5Ch
    Uint16 dMbType;                 //5Dh
    Uint16 dMcWpMode;               //5Eh
    Uint16 dMcLogWdYc;              //5Fh
    Uint16 dBIdxCur;                //60h
    Uint16 dCpbMinus1;              //61h
    Uint16 dHeaderIsrFlag;          //62h
    Uint16 dBIdxNxt;                //63h
    Uint16 dBIdxSaveR7;             //64h
    Uint16 dPsRetFlag;              //65h
    Uint16 dPicStride;              //66h

//------------------------------------------------------------------------------
//    HP SCALING_MATRIX_PARAMETER
//------------------------------------------------------------------------------
    Uint16 dSeqScalingMatrixFlag;   //67h
    Uint16 dPicScalingMatrixFlag;   //68h
    Uint16 dSeqScalingListFlag;     //69h
    Uint16 dPicScalingListFlag;     //6Ah
    Uint16 dSeqDefScalingList;      //6Bh
    Uint16 dPicDefScalingList;      //6Ch

//------------------------------------------------------------------------------
//    MV COL
//------------------------------------------------------------------------------
    Uint16 dReorderInitListNum;     //6Dh
    Uint16 dDecColPicAddrH;         //6Eh
    Uint16 dDecColPicAddrL;         //6Fh
    Uint16 dRefColPicAddrH;         //70h
    Uint16 dRefColPicAddrL;         //71h
    Uint16 dDpbPocCurH;             //72h
    Uint16 dDpbPocCurL;             //73h
    Uint16 dDpbPocTopH;             //74h
    Uint16 dDpbPocTopL;             //75h
    Uint16 dDpbPocBotH;             //76h
    Uint16 dDpbPocBotL;             //77h
    Uint16 dColPicLong;             //78h
    Uint16 dColPicField;            //79h
    Uint16 dNearPocBot;             //7Ah

//------------------------------------------------------------------------------
//    MMCO
//------------------------------------------------------------------------------
    Uint16 dMmcoCurRefNum;          //7Bh
    Uint16 dMmcoCurRefType;         //7Ch
    Uint16 dMmcoSlideWinOutFlag;    //7Dh
    Uint16 dMmcoResetFlag;          //7Eh
    Uint16 dFrameBufNum;            //7Fh
    Uint16 dMmcoPrevRefFrameNum;    //80h
    Uint16 dMmcoPrevDispPicIdx;     //81h
    Uint16 dDpbPrevMmcoResetFlag;   //82h

//------------------------------------------------------------------------------
//    FrameIdx
//------------------------------------------------------------------------------
    Uint16 dPrevDecFrameIdx;        //83h
    Uint16 dDecFrameIdx;            //84h
    Uint16 dDispFrameIdx;           //85h
    Uint16 dSuccess;                //86h

//------------------------------------------------------------------------------
//    ROTATOR (POST PROCESSING)
//------------------------------------------------------------------------------
    Uint16 dPostRotateMode;         //87h
    Uint16 dPostRotateEn;           //88h
    Uint16 dPostInFrmIdx;           //89h
    Uint16 dPostDstFrmIdx;          //8Ah
    Uint16 dPostMbNumX;             //8Bh
    Uint16 dPostMbNumY;             //8Ch
    Uint16 dPostMbPosX;             //8Dh
    Uint16 dPostMbPosY;             //8Eh
    Uint16 dPostProcEn;             //8Fh

//------------------------------------------------------------------------------
//    SLICE BUFFER / VUI PARAMETER
//------------------------------------------------------------------------------
    Uint16 dNalHrdParamFlag;        //90h
    Uint16 dVclHrdParamFlag;        //91h
    Uint16 dPrevNalRefIdc;          //92h
    Uint16 dPrevIdrPicture;         //93h
    Uint16 dFirstVclNal;            //94h
    Uint16 dSaveNalRefIdc;          //95h
    Uint16 dSaveIdrPicture;         //96h
    Uint16 dBpSliceBufLack;         //97h

//------------------------------------------------------------------------------
//    SPS/PPS BUFFER
//------------------------------------------------------------------------------
    Uint16 dPpsBackupedFlag;        //98h
    Uint16 dSpsBufRdPtrH;           //99h
    Uint16 dSpsBufRdPtrL;           //9Ah
    Uint16 dPsBufIsrFlag;           //9Bh
    Uint16 dAlignBit;               //9Ch
    Uint16 dZeroNum;                //9Dh
    Uint16 dNumBit;                 //9Eh
    Uint16 dMcFastInterpolDisable;  //9Fh

//------------------------------------------------------------------------------
//    ADD-ON FEATURE VARIABLES
//------------------------------------------------------------------------------
    Uint16 dIdrPicFlagField;        //A0h
    Uint16 dSliceTypeDecoded;       //A1h
    Uint16 dSliceTypeField;         //A2h
    Uint16 dPicStruct;              //A3h
    Uint16 dNpfIdx;                 //A4h
    Uint16 dSkipFlag;               //A5h
    Uint16 dUserDataPtr;            //A6h
    Uint16 dMvcReportH;             //A7h
    Uint16 dMvcReportL;             //A8h
    Uint16 dBackupPsId;             //A9h
    Uint16 dDisableConcealMethods;  //AAh
    Uint16 dAvcDecCodeNumH;         //ABh
    Uint16 dAvcDecCodeNumL;         //ACh
    Uint16 dAuFound;                //ADh
    Uint16 dErrReasonFlag;          //AEh
    Uint16 dEnUserFrmDelay;         //AFh
    Uint16 dActiveFormat;           //B0h
    Uint16 dX264MvExpEn;            //B1h
    Uint16 dFrameNumbak;            //B2h
    Uint16 dDpbBufferingReported;   //B3h
    Uint16 dChunkReuse;             //B4h
    Uint16 dUserDataBaseAddrH;      //B5h
    Uint16 dUserDataBaseAddrL;      //B6h
    Uint16 dUserDataBufSizeH;       //B7h
    Uint16 dUserDataBufSizeL;       //B8h
    Uint16 dSliceTypeCur;           //B9h
    Uint16 dSliceTypeTop;           //BAh
    Uint16 dSliceTypeBot;           //BBh
    Uint16 dIdrFrameFlag;           //BCh
    Uint16 dSeqChanged;             //BDh
    Uint16 dWrongFrameNumFlag;      //BEh
    Uint16 dClosedCaptionType;      //BFh
    Uint16 dRecoveryFrameCnt;       //C0h
    Uint16 dExactRecoveryPoint;     //C1h
    Uint16 dBrokenLinkFlag;         //C2h
    Uint16 dChangingSliceGroupIdc;  //C3h
    Uint16 dDpbAllocFlag;           //C4h
    Uint16 dDpbPutPicFlag;          //C5h
    Uint16 dMbNumInPicL;            //C6h
    Uint16 dMbNumInPicH;            //C7h
    Uint16 dFirstMbAddrL;           //C8h
    Uint16 dFirstMbAddrH;           //C9h
    Uint16 dFrmErrMbNumL;           //CAh
    Uint16 dFrmErrMbNumH;           //CBh
    Uint16 FREE_CC;                 //CCh
    Uint16 FREE_CD;                 //CDh
    Uint16 FREE_CE;                 //CEh
    Uint16 FREE_CF;                 //CFh
    Uint16 dMbSkipRun;              //D0h
    Uint16 dSkipRunCntDown;         //D1h
    Uint16 FREE_D2;                 //D2h
    Uint16 dFirstFrame;             //D3h

//------------------------------------------------------------------------------
//    MULTI VPU FULL FRAME PARALLEL DECODER
//------------------------------------------------------------------------------
    Uint16 dMbySyncEnable;          //D4h
    Uint16 dMbySyncFrameNum;        //D5h

//------------------------------------------------------------------------------
//    AVC_BASELINE
//------------------------------------------------------------------------------
    Uint16 dFmoMapType;             //D6h
    Uint16 dFmoNumGrp;              //D7h
    Uint16 dFmoNumGrpMinus1;        //D8h
    Uint16 dFmoGrp;                 //D9h
    Uint16 dFmoGrpChangeDir;        //DAh
    Uint16 dFmoGrpChangeRate;       //DBh
    Uint16 dFmoMbNumGrp0;           //DCh
    Uint16 dFmoMap345;              //DDh
    Uint16 dFmoCycleBit;            //DEh
    Uint16 dFmoSliceBufInit;        //DFh
    Uint16 dFmoPrevGrp;             //E0h
    Uint16 dFmoMbAvailA;            //E1h
    Uint16 dFmoMbAvailD;            //E2h
    Uint16 dFmoMbAvail;             //E3h
    Uint16 dExtBitBufR1Bak;         //E4h

//------------------------------------------------------------------------------
//    DPB
//------------------------------------------------------------------------------
    Uint16 dDpbBufCnt;              //E5h
    Uint16 dDpbDispFifoWrPtr;       //E6h
    Uint16 dDpbDispFifoRdPtr;       //E7h
    Uint16 dDpbDispFifoCnt;         //E8h
    Uint16 dDpbFrameDelay;          //E9h
    Uint16 dDpbNumReorderFrame;     //EAh
    Uint16 dDpbMaxDecBuffering;     //EBh
} /*__attribute__((packed))*/ Coda9AvcDirectMemory;

typedef struct {
    Uint16 dChromaIdc;              //08h
    Uint16 dPicX;                   //09h
    Uint16 dPicY;                   //0Ah
    Uint16 dFieldPrediction;        //0Bh
    Uint16 dVopFcodeForward;        //0Ch
    Uint16 dVopFcodeBackward;       //0Dh
    Uint16 dQuantType;              //0Eh
    Uint16 dPackModeBak;            //0Fh
    Uint16 dPackModeOp;             //10h
    Uint16 dFCodeResBitForward;     //11h
    Uint16 dFCodeResMaskForward;    //12h
    Uint16 dFCodeResBitBackward;    //13h
    Uint16 dFCodeResMaskBackward;   //14h
    Uint16 dPackMode;               //15h
    Uint16 dUserDivx;               //16h
    Uint16 dPrevVop;                //17h
    Uint16 dQpel;                   //18h
    Uint16 dMp4LastDisp;            //19h
    Uint16 dCheckReuse;             //1Ah
    Uint16 dDecFrmIdx;              //1Bh
    Uint16 dDispFrmIdx;             //1Ch
    Uint16 dFrameSkipMode;          //1Dh
    Uint16 dIframeSch;              //1Eh
    Uint16 dSuccess;                //1Fh

//------------------------------------------------------------------------------
//    [20 - 3F] FRAME VARIABLES
//------------------------------------------------------------------------------
    Uint16 dVopTimeRes;             //20h
    Uint16 dVopTimeInc;             //21h
    Uint16 dDataPartEn;             //22h
    Uint16 dRVlcEn;                 //23h
    Uint16 dVopType;                //24h
    Uint16 dRoundType;              //25h
    Uint16 dIntraDcVlcThr;          //26h
    Uint16 dVopQuant;               //27h
    Uint16 dFCode;                  //28h
    Uint16 dFCodeResBit;            //29h
    Uint16 dFCodeResMask;           //2Ah
    Uint16 dVopTime;                //2Bh
    Uint16 dReduceResEn;            //2Ch
    Uint16 dVopCoded;               //2Dh
    Uint16 dFlv263;                 //2Eh
    Uint16 dFlvFormat;              //2Fh
    Uint16 dShortVideoHeader;       //30h
    Uint16 dNumGob;                 //31h
    Uint16 dMbNumGob;               //32h
    Uint16 dGobNum;                 //33h
    Uint16 dAnnexI;                 //34h
    Uint16 dAnnexJ;                 //35h
    Uint16 dAnnexK;                 //36h
    Uint16 dAnnexT;                 //37h
    Uint16 dCustomPcf;              //38h
    Uint16 dAnnexD;                 //39h
    Uint16 dPlusType;               //3Ah
    Uint16 dUui;                    //3Bh
    Uint16 dVolDecoded;             //3Ch
    Uint16 dTimeIncrBits;           //3Dh
    Uint16 dBackRefI;               //3Eh
    Uint16 dSpriteVop;              //3Fh

//------------------------------------------------------------------------------
//    [40 - 4F] VARIABLES
//------------------------------------------------------------------------------
    Uint16 dGobFrameId;             //40h
    Uint16 dGobNumDec;              //41h
    Uint16 dConcealMbQs;            //42h
    Uint16 dFirstMbAddr;            //43h
    Uint16 dDecMbAddr;              //44h
    Uint16 dEndMbAddr;              //45h
    Uint16 dErrMbNum;               //46h
    Uint16 dNumPad;                 //47h
    Uint16 dNumBit;                 //48h
    Uint16 dTempBit;                //49h
    Uint16 dMp4DecFrmNumH;          //4Ah
    Uint16 dMp4DecFrmNumL;          //4Bh
    Uint16 dMbOffset;               //4Ch
    Uint16 dFrameBufNum;            //4Dh
    Uint16 dVopTimeAsp;             //4Eh
    Uint16 dMbDpErrFlag;            //4Fh

//------------------------------------------------------------------------------
//    [50 - 5F] MB VARIABLES
//------------------------------------------------------------------------------
    Uint16 dMbNumX;                 //50h
    Uint16 dMbNumY;                 //51h
    Uint16 dMbNumInPic;             //52h
    Uint16 dMbAddr;                 //53h
    Uint16 dMbPosX;                 //54h
    Uint16 dMbPosY;                 //55h
    Uint16 dMbModb;                 //56h
    Uint16 dDummy1;                 //57h
    Uint16 dMbIntra;                //58h
    Uint16 dNotCoded;               //59h
    Uint16 dMbTypeQ;                //5Ah
    Uint16 dMbType4V;               //5Bh
    Uint16 dAcPredFlag;             //5Ch
    Uint16 dMbQuant;                //5Dh
    Uint16 dCbp;                    //5Eh
    Uint16 dUseIntraDcVlc;          //5Fh

//------------------------------------------------------------------------------
//    [60 - 6F] VARIABLES
//------------------------------------------------------------------------------
    Uint16 dReSyncMarkBit;          //60h
    Uint16 dPrevMbQuant;            //61h
    Uint16 dPicStride;              //62h
    Uint16 dMp4DecDbkOn;            //63h
    Uint16 dForPred;                //64h
    Uint16 dBackPred;               //65h
    Uint16 dBVopFlag;               //66h
    Uint16 dDpBufPage;              //67h
    Uint16 dDpBufPtr;               //68h
    Uint16 dTRD;                    //69h
    Uint16 dTRB;                    //6Ah
    Uint16 dSliceId;                //6Bh
    Uint16 dBMbType;                //6Ch
    Uint16 dRefCnt;                 //6Dh
    Uint16 dDummy2;                 //6Eh
    Uint16 dDummy3;                 //6Fh
    Uint16 dFirstPkt;               //70h
    Uint16 dDeringEn;               //71h
    Uint16 dQpLoadBuf;              //72h
    Uint16 dQpSaveBuf;              //73h
    Uint16 dPrevBVopFlag;           //74h
    Uint16 dRotEn;                  //75h
    Uint16 dRotMode;                //76h
    Uint16 dRotProcEn;              //77h
    Uint16 dRotLoadEn;              //78h
    Uint16 dQpProc;                 //79h
    Uint16 dParCode;                //7Ah

//-----------------------------------------------------------------------------
//    [7B - 7F] Padding
//-----------------------------------------------------------------------------
    Uint16 dEosPattern;             //7Bh
    Uint16 dPadTmp0;                //7Ch
    Uint16 dPadTmp1;                //7Dh

//-----------------------------------------------------------------------------
//    [7B - 9F] BitStream
//-----------------------------------------------------------------------------
    Uint16 dDummy4;                 //7Eh
    Uint16 dDummy5;                 //7Fh
    Uint16 dExtBBStaAddrH;          //80h
    Uint16 dExtBBStaAddrL;          //81h
    Uint16 dExtBBSizeH;             //82h
    Uint16 dExtBBSizeL;             //83h
    Uint16 dExtBBEndAddrH;          //84h
    Uint16 dExtBBEndAddrL;          //85h
    Uint16 dExtBBRdPtrH;            //86h
    Uint16 dExtBBRdPtrL;            //87h
    Uint16 dExtBBWrPtrH;            //88h
    Uint16 dExtBBWrPtrL;            //89h
    Uint16 dExtStartOffset;         //8Ah
    Uint16 dExtBitBufIntFlag;       //8Bh
    Uint16 dExtBitBufEndFlag;       //8Ch
    Uint16 dGbuBufMemSel;           //8Dh
    Uint16 dStreamResycDone;        //8Eh
    Uint16 dDoubleBufEn;            //8Fh
    Uint16 dExtResidue;             //90h
    Uint16 dPicEndMode;             //91h
    Uint16 dSignExtH;               //92h
    Uint16 dSignExtL;               //93h
    Uint16 dNalUnitType;            //94h
    Uint16 dRbspEndFlag;            //95h
    Uint16 dRbspEndSize;            //96h
    Uint16 dRbspEndByteCntL;        //97h
    Uint16 dRbspEndByteCntH;        //98h
    Uint16 dBitBufRegBir;           //99h
    Uint16 dBitBufPreLoad;          //9Ah
    Uint16 dExtBBResidue;           //9Bh
    Uint16 dTcntH;                  //9Ch
    Uint16 dTcntL;                  //9Dh
    Uint16 dTcntBackupH;            //9Eh
    Uint16 dTcntBackupL;            //9Fh

//------------------------------------------------------------------------------
//    [A0 - AF] RBSP NAL
//------------------------------------------------------------------------------
    Uint16 dFwdRefIdx;              //A0h
    Uint16 dBwdRefIdx;              //A1h
    Uint16 dRecIdx;                 //A2h
    Uint16 dIPDelayIdx;             //A3h
    Uint16 dPrevDispIdx;            //A4h
    Uint16 dOutputIdx;              //A5h
    Uint16 dDbkIdx;                 //A6h
    Uint16 dPrevFrmBufIdx;          //A7h
    Uint16 dFwdRefTimeBase;         //A8h
    Uint16 dBwdRefTimeBase;         //A9h
    Uint16 dUseDownQMat;            //AAh
    Uint16 dDctType;                //ABh
    Uint16 dAlterVertFlag;          //ACh
    Uint16 dInterlaced;             //ADh
    Uint16 dTopFieldFirst;          //AEh
    Uint16 dFwdRefVopTime;          //AFh

//------------------------------------------------------------------------------
//    [B0 - BF] B-VOP
//------------------------------------------------------------------------------
    Uint16 dBwdRefVopTime;          //B0h
    Uint16 dFixedRate;              //B1h
    Uint16 dTFrame;                 //B2h
    Uint16 dFTRD;                   //B3h
    Uint16 dFTRB;                   //B4h
    Uint16 dFwTopFieldRef;          //B5h
    Uint16 dFwBotFieldRef;          //B6h
    Uint16 dBwTopFieldRef;          //B7h
    Uint16 dBwBotFieldRef;          //B8h
    Uint16 dFieldDelta;             //B9h
    Uint16 dConcealCnt;             //BAh
    Uint16 dMvpRunFlag;             //BBh
    Uint16 dAxiSecEnBlk;            //BCh
    Uint16 dAxiSecUseSramAddr;      //BDh
    Uint16 dDisposablePic;          //BEh
    Uint16 dPrevDisposablePic;      //BFh

//------------------------------------------------------------------------------
//    [C1 - C5] GMC
//------------------------------------------------------------------------------
    Uint16 dGmcDisable;             //C0h
    Uint16 dSpriteGmc;              //C1h
    Uint16 dNoOfWarpPoints;         //C2h
    Uint16 dGmcBrightChange;        //C3h
    Uint16 dWarpAccuracy;           //C4h
    Uint16 dMcSel;                  //C5h
    Uint16 reserved0;
    Uint16 reserved1;
    /* size need alignment 8 at least */
} /*__attribute__((packed))*/ Coda9Mp4DirectMemory;

typedef struct {
    Uint16 tmp0;
    Uint16 tmp1;
    Uint16 pDownQMatIntra[64];      //332h
    Uint16 pDownQMatInter[64];      //372h
    Uint16 pTraj[2];                //3B2h
} /*__attribute__((packed))*/ Coda9Mp4IndirectMemory;

typedef struct {
//------------------------------------------------------------------------------
//    [20 - 2F] STD CODEC/ORDERING/DEBUG LAYER SUBROUTINE
//------------------------------------------------------------------------------
    Uint16 tmp0;
    Uint16 dMpeg2;                  //025h
    Uint16 dProgSeq;                //026h
    Uint16 dLoadBit;                //027h
    Uint16 dFrameRate;              //028h
    Uint16 dFRateExtN;              //029h
    Uint16 dFRateExtD;              //02Ah
    Uint16 dRefCnt;                 //02Bh
    Uint16 dProfile;                //02Ch
    Uint16 dLevel;                  //02Dh
    Uint16 dDummy0;                 //02Eh

//------------------------------------------------------------------------------
//    [30 - 4F] PICTURE LAYER SUBROUTINE
//------------------------------------------------------------------------------
    Uint16 dPicTypePrev;            //02Fh
    Uint16 dDcPrecision;            //030h
    Uint16 dFramePic;               //031h
    Uint16 dFieldPic;               //032h
    Uint16 dField;                  //033h
    Uint16 dPicType;                //034h
    Uint16 dAlternateScan;          //035h
    Uint16 dQScaleType;             //036h
    Uint16 dVlcFormat;              //037h
    Uint16 dConcealMV;              //038h
    Uint16 dTopFieldFirst;          //039h
    Uint16 dRepeatFirst;            //03Ah
    Uint16 dSecondField;            //03Bh
    Uint16 dFrameDct;               //03Ch
    Uint16 dPicStruct;              //03Dh
    Uint16 dProgFrame;              //03Eh
    Uint16 dFieldSeq;               //03Fh
    Uint16 dPictureI;               //040h
    Uint16 dPictureP;               //041h
    Uint16 dPictureB;               //042h
    Uint16 dFirstFieldPicType;      //043h
    Uint16 dFirstFieldPictureI;     //044h
    Uint16 dMbFieldY;               //045h
    Uint16 dVertMBA;                //046h
    Uint16 dFirstMbInSlice;         //047h
    Uint16 dMbAInc;                 //048h
    Uint16 dPictureD;               //049h
    Uint16 dMp2MvpCmd1;             //04Ah
    Uint16 dMp2MvpCmd2;             //04Bh
    Uint16 dLinearFrmIdx;           //04Ch
    Uint16 dDummy1;                 //04Dh
    Uint16 dDummy2;                 //04Eh
    Uint16 dErrReasonFlag;          //04Fh
    Uint16 dTemp;                   //050h
    Uint16 dTemp2;                  //051h
    Uint16 dPicX;                   //052h
    Uint16 dPicY;                   //053h
    Uint16 dMbNumInPic;             //054h
    Uint16 dMbNumX;                 //055h
    Uint16 dMbNumY;                 //056h
    Uint16 dMbAddr;                 //057h
    Uint16 dMbPosX;                 //058h
    Uint16 dMbPosY;                 //059h
    Uint16 dNumPad;                 //05Ah
    Uint16 dActiveFormat;           //05Bh
    Uint16 dPrevVertMBA;            //05Ch
    Uint16 dPicXSeq;                //05Dh
    Uint16 dPicYSeq;                //05Eh
    Uint16 dPicHeaderDecoded;       //05Fh
} /*__attribute__((packed))*/ Coda9Mp2DirectMemory;

typedef struct {
    Uint16 pRSize[2][2];            //400h
    Uint16 pLimit[2][2];            //404h
    Uint16 pDummy[8];               //408h
    Uint16 pIdxQueue[16];           //410h
    Uint16 pIntraQMat[64];          //420h
    Uint16 pInterQMat[64];          //460h
} /*__attribute__((packed))*/ Coda9Mp2IndirectMemory;

typedef struct {
    Uint16 dSeqPicWidth;            //08h
    Uint16 dSeqPicHeight;           //09h
    Uint16 dSeqHScale;              //0Ah
    Uint16 dSeqVScale;              //0Bh
    Uint16 dMbNumX;                 //0Ch
    Uint16 dMbNumY;                 //0Dh
    Uint16 dMbNumInPic;             //0Eh
    Uint16 dPicChunkSizeH;          //0Fh
    Uint16 dPicChunkSizeL;          //10h
    Uint16 dPicHasSeqMeta;          //11h
    Uint16 dPicType;                //12h
    Uint16 dPicVersionNumber;       //13h
    Uint16 dPicShowFrame;           //14h
    Uint16 dPicFirstDpSizeH;        //15h
    Uint16 dPicFirstDpSizeL;        //16h
    Uint16 reserved;                //17h
} /*__attribute__((packed))*/ Coda9Vp8DirectMemory;

typedef struct {
    Uint16 dProfile;                //010h
    Uint16 dLevel;                  //011h
    Uint16 dChromaIdc;              //012h
    Uint16 dByteOffset;             //013h
    Uint16 dBitOffset;              //014h
    Uint16 dPostProcFlag;           //015h
    Uint16 dMaxCodedX;              //016h
    Uint16 dMaxCodedY;              //017h
    Uint16 dPullDown;               //018h
    Uint16 dInterlace;              //019h
    Uint16 dTfcntrFlag;             //01Ah
    Uint16 dFInterpFlag;            //01Bh
    Uint16 dPSF;                    //01Ch
    Uint16 dDisplayX;               //01Dh
    Uint16 dDisplayY;               //01Eh
    Uint16 dAspectX;                //01Fh
    Uint16 dAspectY;                //020h
    Uint16 dFrameRateNr;            //021h
    Uint16 dFrameRateDr;            //022h
    Uint16 dColorPrim;              //023h
    Uint16 dTransferChar;           //024h
    Uint16 dMatrixCoef;             //025h
    Uint16 dHrdParamFlag;           //026h
    Uint16 dHrdNumLeakyBuckets;     //027h
    Uint16 dBrokenLink;             //028h
    Uint16 dClosedEntry;            //029h
    Uint16 dPanScanFlag;            //02Ah
    Uint16 dRefDistFlag;            //02Bh
    Uint16 dLoopFilter;             //02Ch
    Uint16 dFastUVMc;               //02Dh
    Uint16 dExtendedMV;             //02Eh
    Uint16 dDQuant;                 //02Fh
    Uint16 dVSTransform;            //030h
    Uint16 dOverlap;                //031h
    Uint16 dQuantizer;              //032h
    Uint16 dSeqCodedX;              //033h
    Uint16 dSeqCodedY;              //034h
    Uint16 dExtendedDMV;            //035h
    Uint16 dRangeMapYFlag;          //036h
    Uint16 dRangeMapY;              //037h
    Uint16 dRangeMapCFlag;          //038h
    Uint16 dRangeMapC;              //039h
    Uint16 dNumFrames;              //03Ah
    Uint16 dDummy0;                 //03Bh
    Uint16 dStreamFormat;           //03Ch
    Uint16 dMultiRes;               //03Dh
    Uint16 dSyncmarker;             //03Eh
    Uint16 dRangeRed;               //03Fh
    Uint16 dMaxBFrames;             //040h
    Uint16 dFrameSize;              //041h
    Uint16 dDummy1;                 //042h
    Uint16 dX8Intra;                //043h
    Uint16 dBit16Xform;             //044h
    Uint16 dDctTabSwitch;           //045h
    Uint16 dBetaRTM;                //046h
    Uint16 dNumberOfPanScanWindows; //047h
    Uint16 dYUV411;                 //048h
    Uint16 dSprite;                 //049h
    Uint16 dFrameMode;              //04Ah
    Uint16 dPicType0;               //04Bh
    Uint16 dPicType1;               //04Ch
    Uint16 dTempRefCount;           //04Dh
    Uint16 dTopFieldFirst;          //04Eh
    Uint16 dRptFirstField;          //04Fh
    Uint16 dRptFrameCount;          //050h
    Uint16 dSecondField;            //051h
    Uint16 dBottomField;            //052h
    Uint16 dCodedX;                 //053h
    Uint16 dCodedY;                 //054h
    Uint16 dMbNumX;                 //055h
    Uint16 dMbNumY;                 //056h
    Uint16 dSliceY;                 //057h
    Uint16 dUVSamp;                 //058h
    Uint16 dInterpFrm;              //059h
    Uint16 dPostProc;               //05Ah
    Uint16 dResPic;                 //05Bh
    Uint16 dRangeRedFrm;            //05Ch
    Uint16 dBFraction;              //05Dh
    Uint16 dRefDist;                //05Eh
    Uint16 dNumRef;                 //05Fh
    Uint16 dRefField;               //060h
    Uint16 dRndCtrl;                //061h
    Uint16 dMvRange;                //062h
    Uint16 dDMvRange;               //063h
    Uint16 dMvMode;                 //064h
    Uint16 dIntComp;                //065h    ; dIntComp [0]
    Uint16 dLumScale;               //066h    ; dIntScale[0]
    Uint16 dLumShift;               //067h    ; dIntShift[0]
    Uint16 dIntComp1;               //068h    ; dIntComp [1]
    Uint16 dLumScale1;              //069h    ; dIntScale[1]
    Uint16 dLumShift1;              //06Ah    ; dIntShift[1]
    Uint16 dBitpCoded;              //06Bh
    Uint16 dCondOver;               //06Ch
    Uint16 dBitpMode0;              //06Dh
    Uint16 dBitpMode1;              //06Eh
    Uint16 dBitpMode2;              //06Fh
    Uint16 dMbModeTab;              //070h
    Uint16 dMv2BpTab;               //071h
    Uint16 dMv4BpTab;               //072h
    Uint16 dMvTab;                  //073h
    Uint16 dCbpTab;                 //074h
    Uint16 dTTMbf;                  //075h
    Uint16 dTTFrm;                  //076h
    Uint16 dTransDcTab;             //077h
    Uint16 dTransAcFrm0;            //078h
    Uint16 dTransAcFrm1;            //079h
    Uint16 dPQIndexLe8;             //07Ah
    Uint16 dPQuant0;                //07Bh
    Uint16 dPQuant1;                //07Ch
    Uint16 dHalfQp;                 //07Dh
    Uint16 dPQuantizer;             //07Eh
    Uint16 dDQuantFrm;              //07Fh
    Uint16 dDQProfile;              //080h
    Uint16 dDQEdge;                 //081h
    Uint16 dDQBiLevel;              //082h
    Uint16 dX8IntraMb;              //083h
    Uint16 dDctTabMb;               //084h
    Uint16 dPicType;                //085h
    Uint16 dMbAddr;                 //086h
    Uint16 dMbPosX;                 //087h
    Uint16 dMbPosY;                 //088h
    Uint16 reserved0;               //089h
    Uint16 reserved1;               //08Ah
    Uint16 reserved2;               //08Bh
} /*__attribute__((packed))*/ Coda9Vc1DirectMemory;

typedef struct {
    Uint16 tmp1;                    //010h
    Uint16 tmp2;                    //011h
    Uint16 tmp3;                    //012h

    Uint16 dPicX;                   //013h
    Uint16 dPicY;                   //014h
    Uint16 dProgressSeq;            //015h
    Uint16 dProfileId;              //016h
    Uint16 dLevelId;                //017h
    Uint16 dLowDelay;               //018h
    Uint16 dSeqChanged;             //019h
    Uint16 dPicType;                //01Ah
    Uint16 dFrameType;              //01Bh
    Uint16 dPictureDistance;        //01Ch
    Uint16 dProgressiveFrame;       //01Dh
    Uint16 dPictureStructure;       //01Eh
    Uint16 dTopFieldFirst;          //01Fh
    Uint16 dRepeatFirstField;       //020h
    Uint16 dFixedPictureQp;         //021h
    Uint16 dPicQp;                  //022h
    Uint16 dSkipModeFlag;           //023h
    Uint16 dDeblkOffSetA;           //024h
    Uint16 dDeblkOffSetB;           //025h
    Uint16 dPictureReferenceFlag;   //026h
    Uint16 dLoopFilterDisable;      //027h
    Uint16 dHeaderDecoded;          //028h
    Uint16 dTmpVal;                 //029h
    Uint16 dQpYLeft;                //02Ah
    Uint16 dQpYUpper;               //02Bh
    Uint16 dAvgQpLeft;              //02Ch
    Uint16 dAvgQpUpper;             //02Dh
    Uint16 dChromaQpCur;            //02Eh
    Uint16 dChromaQpLeft;           //02Fh
    Uint16 dFixedSliceQp;           //030h
    Uint16 dSliceQp;                //031h
    Uint16 dSliceAddress;           //032h
    Uint16 dFieldPic;               //033h
    Uint16 reserved[45];
    Uint16 dWeightQuantFlag;        //061h
    Uint16 dWeightQuantParam;       //062h
    Uint16 dWeightQuantModel;       //063h
    Uint16 dEntropyModeFlag;        //064h
    Uint16 dPskipRef;               //065h
    Uint16 dNewBdirect;             //066h
    Uint16 dQpDeltaCb;              //068h
    Uint16 dQpDeltaCr;              //069h
} /*__attribute__((packed))*/ Coda9AvsDirectMemory;

typedef struct {
    Uint16 pWqm_8x8[64];            //350h
} /*__attribute__((packed))*/ Coda9AvsIndirectMemory;
#pragma pack(pop)

#define VA_PADDING_LOW          4
#define VA_PADDING_MEDIUM       8

typedef unsigned int VAGenericID;
typedef VAGenericID VASurfaceID;

typedef struct _VAPictureH264
{
    VASurfaceID picture_id;
    Uint32 frame_idx; //frame num
    Uint32 flags;
    Int32 TopFieldOrderCnt;
    Int32 BottomFieldOrderCnt;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32 va_reserved[VA_PADDING_LOW];
} VAPictureH264;

/** H.264 Picture Parameter Buffer */
/*
 * For each picture, and before any slice data, a single
 * picture parameter buffer must be send.
 */
typedef struct _VAPictureParameterBufferH264
{
    VAPictureH264 CurrPic;
    VAPictureH264 ReferenceFrames[16];  /* in DPB */
    Uint16 picture_width_in_mbs_minus1;
    Uint16 picture_height_in_mbs_minus1;
    Uint8 bit_depth_luma_minus8;
    Uint8 bit_depth_chroma_minus8;
    Uint8 num_ref_frames;
    union
    {
        struct
        {
            Uint32 chroma_format_idc : 2;
            Uint32 residual_colour_transform_flag : 1; /* Renamed to separate_colour_plane_flag in newer standard versions. */
            Uint32 gaps_in_frame_num_value_allowed_flag : 1;
            Uint32 frame_mbs_only_flag : 1;
            Uint32 mb_adaptive_frame_field_flag : 1;
            Uint32 direct_8x8_inference_flag : 1;
            Uint32 MinLumaBiPredSize8x8 : 1; /* see A.3.3.2 */
            Uint32 log2_max_frame_num_minus4 : 4;
            Uint32 pic_order_cnt_type : 2;
            Uint32 log2_max_pic_order_cnt_lsb_minus4 : 4;
            Uint32 delta_pic_order_always_zero_flag : 1;
        } bits;
        Uint32 value;
    } seq_fields;

    // FMO is not supported.
    Uint8 num_slice_groups_minus1;
    Uint8 slice_group_map_type;
    Uint16 slice_group_change_rate_minus1;
    //~ FMO is not supported.

    Int8 pic_init_qp_minus26;
    Int8 pic_init_qs_minus26;
    Int8 chroma_qp_index_offset;
    Int8 second_chroma_qp_index_offset;

    union
    {
        struct
        {
            Uint32 entropy_coding_mode_flag : 1;
            Uint32 weighted_pred_flag : 1;
            Uint32 weighted_bipred_idc : 2;
            Uint32 transform_8x8_mode_flag : 1;
            Uint32 field_pic_flag : 1;
            Uint32 constrained_intra_pred_flag : 1;
            Uint32 pic_order_present_flag : 1; /* Renamed to bottom_field_pic_order_in_frame_present_flag in newer standard versions. */
            Uint32 deblocking_filter_control_present_flag : 1;
            Uint32 redundant_pic_cnt_present_flag : 1;
            Uint32 reference_pic_flag : 1; /* nal_ref_idc != 0 */
        } bits;
        Uint32 value;
    } pic_fields;

    Uint16 frame_num;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_MEDIUM];
} VAPictureParameterBufferH264;

/** H.264 Inverse Quantization Matrix Buffer */
typedef struct _VAIQMatrixBufferH264
{
    /** \brief 4x4 scaling list, in raster scan order. */
    Uint8 ScalingList4x4[6][16];
    /** \brief 8x8 scaling list, in raster scan order. */
    Uint8 ScalingList8x8[2][64];

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VAIQMatrixBufferH264;

/** H.264 Slice Parameter Buffer */
typedef struct _VASliceParameterBufferH264
{
    Uint32 slice_data_size;/* number of bytes in the slice data buffer for this slice */
    /** \brief Byte offset to the NAL Header Unit for this slice. */
    Uint32 slice_data_offset;
    Uint32 slice_data_flag; /* see VA_SLICE_DATA_FLAG_XXX defintions */
    /**
    * \brief Bit offset from NAL Header Unit to the begining of slice_data().
    *
    * This bit offset is relative to and includes the NAL unit byte
    * and represents the number of bits parsed in the slice_header()
    * after the removal of any emulation prevention bytes in
    * there. However, the slice data buffer passed to the hardware is
    * the original bitstream, thus including any emulation prevention
    * bytes.
    */
    Uint16 slice_data_bit_offset;
    Uint16 first_mb_in_slice;
    Uint8 slice_type;
    Uint8 direct_spatial_mv_pred_flag;
    /**
    * H264/AVC syntax element
    *
    * if num_ref_idx_active_override_flag equals 0, host decoder should
    * set its value to num_ref_idx_l0_default_active_minus1.
    */
    Uint8 num_ref_idx_l0_active_minus1;
    /**
    * H264/AVC syntax element
    *
    * if num_ref_idx_active_override_flag equals 0, host decoder should
    * set its value to num_ref_idx_l1_default_active_minus1.
    */
    Uint8 num_ref_idx_l1_active_minus1;
    Uint8 cabac_init_idc;
    Int8 slice_qp_delta;
    Uint8 disable_deblocking_filter_idc;
    Int8 slice_alpha_c0_offset_div2;
    Int8 slice_beta_offset_div2;
    VAPictureH264 RefPicList0[32]; /* See 8.2.4.2 */
    VAPictureH264 RefPicList1[32]; /* See 8.2.4.2 */
    Uint8 luma_log2_weight_denom;
    Uint8 chroma_log2_weight_denom;
    Uint8 luma_weight_l0_flag;
    Int16 luma_weight_l0[32];
    Int16 luma_offset_l0[32];
    Uint8 chroma_weight_l0_flag;
    Int16 chroma_weight_l0[32][2];
    Int16 chroma_offset_l0[32][2];
    Uint8 luma_weight_l1_flag;
    Int16 luma_weight_l1[32];
    Int16 luma_offset_l1[32];
    Uint8 chroma_weight_l1_flag;
    Int16 chroma_weight_l1[32][2];
    Int16 chroma_offset_l1[32][2];

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VASliceParameterBufferH264;

/** MPEG-4 Picture Parameter Buffer */
/*
 * For each frame or field, and before any slice data, a single
 * picture parameter buffer must be send.
 */
typedef struct _VAPictureParameterBufferMPEG4
{
    Uint16 vop_width;
    Uint16 vop_height;
    VASurfaceID forward_reference_picture;
    VASurfaceID backward_reference_picture;
    union {
        struct {
            Uint32 short_video_header : 1;
            Uint32 chroma_format : 2;
            Uint32 interlaced : 1;
            Uint32 obmc_disable : 1;
            Uint32 sprite_enable : 2;
            Uint32 sprite_warping_accuracy : 2;
            Uint32 quant_type : 1;
            Uint32 quarter_sample : 1;
            Uint32 data_partitioned : 1;
            Uint32 reversible_vlc : 1;
            Uint32 resync_marker_disable : 1;
        } bits;
        Uint32 value;
    } vol_fields;
    Uint8 no_of_sprite_warping_points;
    Uint16 sprite_trajectory_du[3];
    Uint16 sprite_trajectory_dv[3];
    Uint8 quant_precision;
    union {
        struct {
            Uint32 vop_coding_type : 2; 
            Uint32 backward_reference_vop_coding_type : 2;
            Uint32 vop_rounding_type : 1;
            Uint32 intra_dc_vlc_thr : 3;
            Uint32 top_field_first : 1;
            Uint32 alternate_vertical_scan_flag : 1;
        } bits;
        Uint32 value;
    } vop_fields;
    Uint8 vop_fcode_forward;
    Uint8 vop_fcode_backward;
    Uint16 vop_time_increment_resolution;
    /* short header related */
    Uint8 num_gobs_in_vop;
    Uint8 num_macroblocks_in_gob;
    /* for direct mode prediction */
    Uint16 TRB;
    Uint16 TRD;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VAPictureParameterBufferMPEG4;

/** MPEG-4 Inverse Quantization Matrix Buffer */
typedef struct _VAIQMatrixBufferMPEG4
{
    /** Same as the MPEG-4:2 bitstream syntax element. */
    Uint32 load_intra_quant_mat;
    /** Same as the MPEG-4:2 bitstream syntax element. */
    Uint32 load_non_intra_quant_mat;
    /** The matrix for intra blocks, in zig-zag scan order. */
    Uint8 intra_quant_mat[64];
    /** The matrix for non-intra blocks, in zig-zag scan order. */
    Uint8 non_intra_quant_mat[64];

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VAIQMatrixBufferMPEG4;

/** MPEG-4 Slice Parameter Buffer */
typedef struct _VASliceParameterBufferMPEG4 {
    Uint32 slice_data_size;   /* number of bytes in the slice data buffer for this slice. */
    Uint32 slice_data_offset; /* the offset to the first byte of slice data. */
    Uint32 slice_data_flag;   /* see VA_SLICE_DATA_FLAG_XXX definitions. */
    Uint32 macroblock_offset; /* the offset to the first bit of MB from the first byte of slice data. */
    Uint32 macroblock_number;
    Int32 quant_scale;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VASliceParameterBufferMPEG4;

/** MPEG-2 Picture Parameter Buffer */
/*
 * For each frame or field, and before any slice data, a single
 * picture parameter buffer must be send.
 */
typedef struct _VAPictureParameterBufferMPEG2
{
    Uint16 horizontal_size;
    Uint16 vertical_size;
    VASurfaceID forward_reference_picture;
    VASurfaceID backward_reference_picture;
    /* meanings of the following fields are the same as in the standard */
    Int32 picture_coding_type;
    Int32 f_code; /* pack all four fcode into this */
    union {
        struct {
            Uint32 intra_dc_precision : 2;
            Uint32 picture_structure : 2;
            Uint32 top_field_first : 1;
            Uint32 frame_pred_frame_dct : 1;
            Uint32 concealment_motion_vectors : 1;
            Uint32 q_scale_type : 1;
            Uint32 intra_vlc_format : 1;
            Uint32 alternate_scan : 1;
            Uint32 repeat_first_field : 1;
            Uint32 progressive_frame : 1;
            Uint32 is_first_field : 1; /* indicate whether the current field
                                        * is the first field for field picture
                                        */
        } bits;
        Uint32 value;
    } picture_coding_extension;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VAPictureParameterBufferMPEG2;

/** MPEG-2 Inverse Quantization Matrix Buffer */
typedef struct _VAIQMatrixBufferMPEG2
{
    /** \brief Same as the MPEG-2 bitstream syntax element. */
    Int32 load_intra_quantiser_matrix;
    /** \brief Same as the MPEG-2 bitstream syntax element. */
    Int32 load_non_intra_quantiser_matrix;
    /** \brief Same as the MPEG-2 bitstream syntax element. */
    Int32 load_chroma_intra_quantiser_matrix;
    /** \brief Same as the MPEG-2 bitstream syntax element. */
    Int32 load_chroma_non_intra_quantiser_matrix;
    /** \brief Luminance intra matrix, in zig-zag scan order. */
    Uint8 intra_quantiser_matrix[64];
    /** \brief Luminance non-intra matrix, in zig-zag scan order. */
    Uint8 non_intra_quantiser_matrix[64];
    /** \brief Chroma intra matrix, in zig-zag scan order. */
    Uint8 chroma_intra_quantiser_matrix[64];
    /** \brief Chroma non-intra matrix, in zig-zag scan order. */
    Uint8 chroma_non_intra_quantiser_matrix[64];

    /** \brief Reserved bytes for future use, must be zero */
    Uint32                va_reserved[VA_PADDING_LOW];
} VAIQMatrixBufferMPEG2;

typedef struct _VABoolCoderContextVPX
{
    /* partition 0 "range" */
    Uint8 range;
    /* partition 0 "value" */
    Uint8 value;
    /*
     * 'partition 0 number of shifts before an output byte is available'
     * it is the number of remaining bits in 'value' for decoding, range [0, 7].
     */

    Uint8 count;
} VABoolCoderContextVPX;

typedef struct  _VAPictureParameterBufferVP8
{
    /* frame width in pixels */
    Uint32 frame_width;
    /* frame height in pixels */
    Uint32 frame_height;

    /* specifies the "last" reference frame */
    VASurfaceID last_ref_frame;
    /* specifies the "golden" reference frame */
    VASurfaceID golden_ref_frame;
    /* specifies the "alternate" referrence frame */
    VASurfaceID alt_ref_frame;
    /* specifies the out-of-loop deblocked frame, not used currently */
    VASurfaceID out_of_loop_frame;

    union {
        struct {
            /* same as key_frame in bitstream syntax, 0 means a key frame */
            Uint32 key_frame                  : 1; 
            /* same as version in bitstream syntax */
            Uint32 version                    : 3;
            /* same as segmentation_enabled in bitstream syntax */
            Uint32 segmentation_enabled               : 1;
            /* same as update_mb_segmentation_map in bitstream syntax */
            Uint32 update_mb_segmentation_map : 1;
            /* same as update_segment_feature_data in bitstream syntax */
            Uint32 update_segment_feature_data        : 1;
            /* same as filter_type in bitstream syntax */
            Uint32 filter_type                        : 1; 
            /* same as sharpness_level in bitstream syntax */
            Uint32 sharpness_level            : 3; 
            /* same as loop_filter_adj_enable in bitstream syntax */
            Uint32 loop_filter_adj_enable             : 1; 
            /* same as mode_ref_lf_delta_update in bitstream syntax */
            Uint32 mode_ref_lf_delta_update   : 1; 
            /* same as sign_bias_golden in bitstream syntax */
            Uint32 sign_bias_golden           : 1; 
            /* same as sign_bias_alternate in bitstream syntax */
            Uint32 sign_bias_alternate                : 1; 
            /* same as mb_no_coeff_skip in bitstream syntax */
            Uint32 mb_no_coeff_skip           : 1; 
            /* flag to indicate that loop filter should be disabled */
            Uint32 loop_filter_disable                : 1; 
        } bits;
        Uint32 value;
    } pic_fields;

    /*
     * probabilities of the segment_id decoding tree and same as 
     * mb_segment_tree_probs in the spec.
     */
    Uint8 mb_segment_tree_probs[3];

    /* Post-adjustment loop filter levels for the 4 segments */
    Uint8 loop_filter_level[4];
    /* loop filter deltas for reference frame based MB level adjustment */
    Int8 loop_filter_deltas_ref_frame[4];
    /* loop filter deltas for coding mode based MB level adjustment */
    Int8 loop_filter_deltas_mode[4];

    /* same as prob_skip_false in bitstream syntax */
    Uint8 prob_skip_false;
    /* same as prob_intra in bitstream syntax */
    Uint8 prob_intra;
    /* same as prob_last in bitstream syntax */
    Uint8 prob_last;
    /* same as prob_gf in bitstream syntax */
    Uint8 prob_gf;

    /* 
     * list of 4 probabilities of the luma intra prediction mode decoding
     * tree and same as y_mode_probs in frame header
     */
    Uint8 y_mode_probs[4]; 
    /*
     * list of 3 probabilities of the chroma intra prediction mode decoding
     * tree and same as uv_mode_probs in frame header
     */
    Uint8 uv_mode_probs[3];
    /* 
     * updated mv decoding probabilities and same as mv_probs in 
     * frame header
     */
    Uint8 mv_probs[2][19];

    VABoolCoderContextVPX bool_coder_ctx;

    Uint32                va_reserved[VA_PADDING_LOW];
} VAPictureParameterBufferVP8;

typedef struct  _VASliceParameterBufferVP8
{
    /*
     * number of bytes in the slice data buffer for the partitions 
     */
    Uint32 slice_data_size;
    /*
     * offset to the first byte of partition data (control partition)
     */
    Uint32 slice_data_offset;
    /*
     * see VA_SLICE_DATA_FLAG_XXX definitions
     */
    Uint32 slice_data_flag;
    /*
     * offset to the first bit of MB from the first byte of partition data(slice_data_offset)
     */
    Uint32 macroblock_offset;

    /*
     * Partitions
     * (1<<log2_nbr_of_dct_partitions)+1, count both control partition (frame header) and toke partition
     */
    Uint8 num_of_partitions;
    /*
     * partition_size[0] is remaining bytes of control partition after parsed by application.
     * exclude current byte for the remaining bits in bool_coder_ctx.
     * exclude the uncompress data chunk since first_part_size 'excluding the uncompressed data chunk'
     */
    Uint32 partition_size[9];

    Uint32                va_reserved[VA_PADDING_LOW];
} VASliceParameterBufferVP8;

typedef struct _VAProbabilityDataBufferVP8
{
    Uint8 dct_coeff_probs[4][8][3][11];

    Uint32                va_reserved[VA_PADDING_LOW];
} VAProbabilityDataBufferVP8;

typedef struct _VAIQMatrixBufferVP8
{
    /*
     * array first dimensional is segment and 2nd dimensional is Q index
     * all Q indexs should be clipped to be range [0, 127]
     */
    Uint16 quantization_index[4][6];

    Uint32                va_reserved[VA_PADDING_LOW];
} VAIQMatrixBufferVP8;

/** VC-1 Picture Parameter Buffer */
/*
 * For each picture, and before any slice data, a picture parameter
 * buffer must be send. Multiple picture parameter buffers may be
 * sent for a single picture. In that case picture parameters will
 * apply to all slice data that follow it until a new picture
 * parameter buffer is sent.
 *
 * Notes:
 *   pic_quantizer_type should be set to the applicable quantizer
 *   type as defined by QUANTIZER (J.1.19) and either
 *   PQUANTIZER (7.1.1.8) or PQINDEX (7.1.1.6)
 */
typedef struct _VAPictureParameterBufferVC1
{
    VASurfaceID forward_reference_picture;
    VASurfaceID backward_reference_picture;
    /* if out-of-loop post-processing is done on the render
     * target, then we need to keep the in-loop decoded
     * picture as a reference picture
     */
    VASurfaceID inloop_decoded_picture;

    /* sequence layer for AP or meta data for SP and MP */
    union {
        struct {
            Uint32 pulldown : 1; /* SEQUENCE_LAYER::PULLDOWN */
            Uint32 interlace : 1; /* SEQUENCE_LAYER::INTERLACE */
            Uint32 tfcntrflag : 1; /* SEQUENCE_LAYER::TFCNTRFLAG */
            Uint32 finterpflag : 1; /* SEQUENCE_LAYER::FINTERPFLAG */
            Uint32 psf : 1; /* SEQUENCE_LAYER::PSF */
            Uint32 multires : 1; /* METADATA::MULTIRES */
            Uint32 overlap : 1; /* METADATA::OVERLAP */
            Uint32 syncmarker : 1; /* METADATA::SYNCMARKER */
            Uint32 rangered : 1; /* METADATA::RANGERED */
            Uint32 max_b_frames : 3; /* METADATA::MAXBFRAMES */
            Uint32 profile : 2; /* SEQUENCE_LAYER::PROFILE or The MSB of METADATA::PROFILE */
        } bits;
        Uint32 value;
    } sequence_fields;

    Uint16 coded_width;		/* ENTRY_POINT_LAYER::CODED_WIDTH */
    Uint16 coded_height;	/* ENTRY_POINT_LAYER::CODED_HEIGHT */
    union {
        struct {
            Uint32 broken_link : 1; /* ENTRY_POINT_LAYER::BROKEN_LINK */
            Uint32 closed_entry : 1; /* ENTRY_POINT_LAYER::CLOSED_ENTRY */
            Uint32 panscan_flag : 1; /* ENTRY_POINT_LAYER::PANSCAN_FLAG */
            Uint32 loopfilter : 1; /* ENTRY_POINT_LAYER::LOOPFILTER */
        } bits;
        Uint32 value;
    } entrypoint_fields;
    Uint8 conditional_overlap_flag; /* ENTRY_POINT_LAYER::CONDOVER */
    Uint8 fast_uvmc_flag;	/* ENTRY_POINT_LAYER::FASTUVMC */
    union {
        struct {
            Uint32 luma_flag : 1; /* ENTRY_POINT_LAYER::RANGE_MAPY_FLAG */
            Uint32 luma : 3; /* ENTRY_POINT_LAYER::RANGE_MAPY */
            Uint32 chroma_flag : 1; /* ENTRY_POINT_LAYER::RANGE_MAPUV_FLAG */
            Uint32 chroma : 3; /* ENTRY_POINT_LAYER::RANGE_MAPUV */
        } bits;
        Uint32 value;
    } range_mapping_fields;

    Uint8 b_picture_fraction;	/* PICTURE_LAYER::BFRACTION */
    Uint8 cbp_table;		/* PICTURE_LAYER::CBPTAB/ICBPTAB */
    Uint8 mb_mode_table;	/* PICTURE_LAYER::MBMODETAB */
    Uint8 range_reduction_frame;/* PICTURE_LAYER::RANGEREDFRM */
    Uint8 rounding_control;	/* PICTURE_LAYER::RNDCTRL */
    Uint8 post_processing;	/* PICTURE_LAYER::POSTPROC */
    Uint8 picture_resolution_index;	/* PICTURE_LAYER::RESPIC */
    Uint8 luma_scale;		/* PICTURE_LAYER::LUMSCALE */
    Uint8 luma_shift;		/* PICTURE_LAYER::LUMSHIFT */
    union {
        struct {
            Uint32 picture_type : 3; /* PICTURE_LAYER::PTYPE */
            Uint32 frame_coding_mode : 3; /* PICTURE_LAYER::FCM */
            Uint32 top_field_first : 1; /* PICTURE_LAYER::TFF */
            Uint32 is_first_field : 1; /* set to 1 if it is the first field */
            Uint32 intensity_compensation : 1; /* PICTURE_LAYER::INTCOMP */
        } bits;
        Uint32 value;
    } picture_fields;
    union {
        struct {
            Uint32 mv_type_mb : 1; 	/* PICTURE::MVTYPEMB */
            Uint32 direct_mb : 1; 	/* PICTURE::DIRECTMB */
            Uint32 skip_mb : 1; 	/* PICTURE::SKIPMB */
            Uint32 field_tx : 1; 	/* PICTURE::FIELDTX */
            Uint32 forward_mb : 1;	/* PICTURE::FORWARDMB */
            Uint32 ac_pred : 1;	/* PICTURE::ACPRED */
            Uint32 overflags : 1;	/* PICTURE::OVERFLAGS */
        } flags;
        Uint32 value;
    } raw_coding;
    union {
        struct {
            Uint32 bp_mv_type_mb : 1;    /* PICTURE::MVTYPEMB */
            Uint32 bp_direct_mb : 1;    /* PICTURE::DIRECTMB */
            Uint32 bp_skip_mb : 1;    /* PICTURE::SKIPMB */
            Uint32 bp_field_tx : 1;    /* PICTURE::FIELDTX */
            Uint32 bp_forward_mb : 1;    /* PICTURE::FORWARDMB */
            Uint32 bp_ac_pred : 1;    /* PICTURE::ACPRED */
            Uint32 bp_overflags : 1;    /* PICTURE::OVERFLAGS */
        } flags;
        Uint32 value;
    } bitplane_present; /* signal what bitplane is being passed via the bitplane buffer */
    union {
        struct {
            Uint32 reference_distance_flag : 1;/* PICTURE_LAYER::REFDIST_FLAG */
            Uint32 reference_distance : 5;/* PICTURE_LAYER::REFDIST */
            Uint32 num_reference_pictures : 1;/* PICTURE_LAYER::NUMREF */
            Uint32 reference_field_pic_indicator : 1;/* PICTURE_LAYER::REFFIELD */
        } bits;
        Uint32 value;
    } reference_fields;
    union {
        struct {
            Uint32 mv_mode : 3; /* PICTURE_LAYER::MVMODE */
            Uint32 mv_mode2 : 3; /* PICTURE_LAYER::MVMODE2 */
            Uint32 mv_table : 3; /* PICTURE_LAYER::MVTAB/IMVTAB */
            Uint32 two_mv_block_pattern_table : 2; /* PICTURE_LAYER::2MVBPTAB */
            Uint32 four_mv_switch : 1; /* PICTURE_LAYER::4MVSWITCH */
            Uint32 four_mv_block_pattern_table : 2; /* PICTURE_LAYER::4MVBPTAB */
            Uint32 extended_mv_flag : 1; /* ENTRY_POINT_LAYER::EXTENDED_MV */
            Uint32 extended_mv_range : 2; /* PICTURE_LAYER::MVRANGE */
            Uint32 extended_dmv_flag : 1; /* ENTRY_POINT_LAYER::EXTENDED_DMV */
            Uint32 extended_dmv_range : 2; /* PICTURE_LAYER::DMVRANGE */
        } bits;
        Uint32 value;
    } mv_fields;
    union {
        struct {
            Uint32 dquant : 2; 	/* ENTRY_POINT_LAYER::DQUANT */
            Uint32 quantizer : 2; 	/* ENTRY_POINT_LAYER::QUANTIZER */
            Uint32 half_qp : 1; 	/* PICTURE_LAYER::HALFQP */
            Uint32 pic_quantizer_scale : 5;/* PICTURE_LAYER::PQUANT */
            Uint32 pic_quantizer_type : 1;/* PICTURE_LAYER::PQUANTIZER */
            Uint32 dq_frame : 1; 	/* VOPDQUANT::DQUANTFRM */
            Uint32 dq_profile : 2; 	/* VOPDQUANT::DQPROFILE */
            Uint32 dq_sb_edge : 2; 	/* VOPDQUANT::DQSBEDGE */
            Uint32 dq_db_edge : 2; 	/* VOPDQUANT::DQDBEDGE */
            Uint32 dq_binary_level : 1; 	/* VOPDQUANT::DQBILEVEL */
            Uint32 alt_pic_quantizer : 5;/* VOPDQUANT::ALTPQUANT */
        } bits;
        Uint32 value;
    } pic_quantizer_fields;
    union {
        struct {
            Uint32 variable_sized_transform_flag : 1;/* ENTRY_POINT_LAYER::VSTRANSFORM */
            Uint32 mb_level_transform_type_flag : 1;/* PICTURE_LAYER::TTMBF */
            Uint32 frame_level_transform_type : 2;/* PICTURE_LAYER::TTFRM */
            Uint32 transform_ac_codingset_idx1 : 2;/* PICTURE_LAYER::TRANSACFRM */
            Uint32 transform_ac_codingset_idx2 : 2;/* PICTURE_LAYER::TRANSACFRM2 */
            Uint32 intra_transform_dc_table : 1;/* PICTURE_LAYER::TRANSDCTAB */
        } bits;
        Uint32 value;
    } transform_fields;

    Uint8 luma_scale2; /* PICTURE_LAYER::LUMSCALE2 */
    Uint8 luma_shift2; /* PICTURE_LAYER::LUMSHIFT2 */
    Uint8 intensity_compensation_field; /* Index for PICTURE_LAYER::INTCOMPFIELD value in Table 109 (9.1.1.48) */

    /** \brief Reserved bytes for future use, must be zero */
    Uint32 va_reserved[VA_PADDING_MEDIUM - 1];
} VAPictureParameterBufferVC1;

/* VC-1 Slice Parameter Buffer */
typedef struct _VASliceParameterBufferVC1
{
    Uint32 slice_data_size;/* number of bytes in the slice data buffer for this slice */
    Uint32 slice_data_offset;/* the offset to the first byte of slice data */
    Uint32 slice_data_flag; /* see VA_SLICE_DATA_FLAG_XXX defintions */
    Uint32 macroblock_offset;/* the offset to the first bit of MB from the first byte of slice data */
    Uint32 slice_vertical_position;
} VASliceParameterBufferVC1;

typedef struct _VAPictureAVS
{
    VASurfaceID     surface_id;
    Uint32          flags;
    Uint32          poc;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32          va_reserved[VA_PADDING_LOW];
} VAPictureAVS;

/* AVS Picture Parameter Buffer */
typedef struct _VAPictureParameterBufferAVS
{
    VAPictureAVS    curr_pic;
    VAPictureAVS    ref_list[2];

    Uint16 width;                                             // coded width
    Uint16 height;                                            // coded height

    Uint16 picture_type                           :  2;       // VAAVSPicType
    Uint16 progressive_seq_flag                   :  1;
    Uint16 progressive_frame_flag                 :  1;
    Uint16 picture_structure_flag                 :  1;       // 0:field coding, 1:frame coding
    Uint16 fixed_pic_qp_flag                      :  1;
    Uint16 picture_qp                             :  6;
    Uint16 loop_filter_disable_flag               :  1;
    Uint16 skip_mode_flag_flag                    :  1;
    Uint16 picture_reference_flag                 :  1;
    Uint16 reserved_flag_1                        :  1;       // no_forward_reference_flag

    Int8   alpha_c_offset;
    Int8   beta_offset;

    struct
    {
        Uint16 guangdian_flag                     :  1;
        Uint16 aec_flag                           :  1;
        Uint16 weight_quant_flag                  :  1;
        Uint16 reserved_flag_2                    :  1;       // pb_field_enhanced_flag
        Uint16 reserved                           : 12;
        Int8   chroma_quant_param_delta_cb;
        Int8   chroma_quant_param_delta_cr;
        Uint8  wqm_8x8[64];
    } guangdian_fields;

    /** \brief Reserved bytes for future use, must be zero */
    Uint32 va_reserved[VA_PADDING_MEDIUM];
} VAPictureParameterBufferAVS;

/* AVS Slice Parameter Buffer */
typedef struct _VASliceParameterBufferAVS
{
    Uint32 slice_data_size;       /* number of bytes in the slice data buffer for this slice */
    Uint32 slice_data_offset;     /* the offset to the first byte of slice data */
    Uint32 slice_data_flag;       /* see VA_SLICE_DATA_FLAG_XXX defintions */
    Uint32 mb_data_bit_offset;    /* offset to the first bit of MB from the first byte of slice data (slice_data_offset) */

    Uint32 slice_vertical_pos         : 16;   /* first mb row of the slice */
    Uint32 fixed_slice_qp_flag        :  1;
    Uint32 slice_qp                   :  6;
    Uint32 slice_weight_pred_flag     :  1;
    Uint32 mb_weight_pred_flag        :  1;

    Uint8 luma_scale[4];
    Int8  luma_shift[4];
    Uint8 chroma_scale[4];
    Int8  chroma_shift[4];

    /** \brief Reserved bytes for future use, must be zero */
    Uint32 va_reserved[VA_PADDING_LOW];
} VASliceParameterBufferAVS;

typedef struct {
    EncOpenParam        openParam;
    EncInitialInfo      initialInfo;
    PhysicalAddress     streamRdPtr;
    PhysicalAddress     streamWrPtr;
    int                 streamEndflag;
    PhysicalAddress     streamRdPtrRegAddr;
    PhysicalAddress     streamWrPtrRegAddr;
    PhysicalAddress     streamBufStartAddr;
    PhysicalAddress     streamBufEndAddr;
    PhysicalAddress     currentPC;
    PhysicalAddress     busyFlagAddr;
    PhysicalAddress     rootPageTableStartAddr;
    PhysicalAddress     rootPageTableEndAddr;
    int                 streamBufSize;
    int                 linear2TiledEnable;
    int                 linear2TiledMode;    // coda980 only
    TiledMapType        mapType;
    int                 userMapEnable;
    FrameBuffer         frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t        vbFrame;
    vpu_buffer_t        vbPPU;
    int                 frameAllocExt;
    int                 ppuAllocExt;
    vpu_buffer_t        vbSubSampFrame;         /*!<< CODA960 only */
    vpu_buffer_t        vbMvcSubSampFrame;      /*!<< CODA960 only */
    int                 numFrameBuffers;
    int                 stride;
    int                 frameBufferHeight;
    int                 rotationEnable;
    Uint32              mmuCtxId;
    BOOL                mmuEnable;
    BOOL                mirrorEnable;
    MirrorDirection     mirrorDirection;
    int                 rotationAngle;
    int                 initialInfoObtained;
    int                 ringBufferEnable;
    SecAxiInfo          secAxiInfo;
    MaverickCacheConfig cacheConfig;
    int                 ActivePPSIdx;           /*!<< CODA980 */
    int                 frameIdx;               /*!<< CODA980 */
    int                 fieldDone;              /*!<< CODA980 */
    int                 lineBufIntEn;
    vpu_buffer_t        vbWork;
    vpu_buffer_t        vbScratch;

    vpu_buffer_t        vbTemp;                     //!< Temp buffer (WAVE encoder )
    vpu_buffer_t        vbArTbl;
    vpu_buffer_t        vbMV[MAX_REG_FRAME];        //!< colMV buffer (WAVE encoder)
    vpu_buffer_t        vbFbcYTbl[MAX_REG_FRAME];   //!< FBC Luma table buffer (WAVE encoder)
    vpu_buffer_t        vbFbcCTbl[MAX_REG_FRAME];   //!< FBC Chroma table buffer (WAVE encoder)
    vpu_buffer_t        vbSubSamBuf[MAX_REG_FRAME]; //!< Sub-sampled buffer for ME (WAVE encoder)
    vpu_buffer_t        vbTask;
    vpu_buffer_t        vbDefCdf;

    TiledMapConfig      mapCfg;
    DRAMConfig          dramCfg;                /*!<< CODA960 */

    Uint32 prefixSeiNalEnable;
    Uint32 prefixSeiDataSize;
    PhysicalAddress prefixSeiNalAddr;
    Uint32 suffixSeiNalEnable;
    Uint32 suffixSeiDataSize;
    PhysicalAddress suffixSeiNalAddr;
    Int32   errorReasonCode;
    Uint64          curPTS;             /**! Current timestamp in 90KHz */
    Uint64          ptsMap[32];         /**! PTS mapped with source frame index */
    Uint32          instanceQueueCount;
    Uint32          reportQueueCount;
    BOOL            instanceQueueFull;
    BOOL            reportQueueEmpty;
    Uint32          encWrPtrSel;
    Uint32          firstCycleCheck;
    Uint32          cyclePerTick;
    Uint32          productCode;
    Uint32          vlcBufSize;
    Uint32          paramBufSize;
    Int32           ringBufferWrapEnable;
    Uint32          streamEndian;
    Uint32          sourceEndian;
    Uint32          customMapEndian;
    VaapiInfo       vaapi;
    Uint32          encWidth;
    Uint32          encHeight;
} EncInfo;


typedef struct CodecInst {
    Int32   inUse;
    Int32   instIndex;
    Int32   coreIdx;
    Int32   codecMode;
    Int32   codecModeAux;
    Int32   productId;
    Int32   loggingEnable;
    Uint32  isDecoder;
    union {
        EncInfo encInfo;
        DecInfo decInfo;
    }* CodecInfo;
} CodecInst;

/**
* @brief This structure is used for reporting bandwidth (only for WAVE5).
*/
typedef struct {
    Uint32 bwMode;         /* 0: total bandwidth, 1: select burst length bandwidth*/
    Uint32 burstLengthIdx;
    Uint32 prpBwRead;
    Uint32 prpBwWrite;
    Uint32 fbdYRead;
    Uint32 fbcYWrite;
    Uint32 fbdCRead;
    Uint32 fbcCWrite;
    Uint32 priBwRead;
    Uint32 priBwWrite;
    Uint32 secBwRead;
    Uint32 secBwWrite;
    Uint32 procBwRead;
    Uint32 procBwWrite;
    Uint32 bwbBwWrite;
} VPUBWData;


typedef struct {
    Uint32 priReason;
    Uint32 regs[0xC0];
    Uint32 debugInfo0;
    Uint32 debugInfo1;
} VPUDebugInfo;

/*******************************************************************************
 * H.265 USER DATA(VUI and SEI)                                                *
 *******************************************************************************/
#define H265_MAX_DPB_SIZE                 17
#define H265_MAX_NUM_SUB_LAYER            8
#define H265_MAX_NUM_ST_RPS               64
#define H265_MAX_CPB_CNT                  32
#define H265_MAX_NUM_VERTICAL_FILTERS     6
#define H265_MAX_NUM_HORIZONTAL_FILTERS   4
#define H265_MAX_TAP_LENGTH               32
#define H265_MAX_NUM_KNEE_POINT           1000

#define H265_MAX_NUM_TONE_VALUE		      1024
#define H265_MAX_NUM_FILM_GRAIN_COMPONENT 3
#define H265_MAX_NUM_INTENSITY_INTERVALS  256
#define H265_MAX_NUM_MODEL_VALUES		  5

#define H265_MAX_LUT_NUM_VAL			  4
#define H265_MAX_LUT_NUM_VAL_MINUS1       33
#define H265_MAX_COLOUR_REMAP_COEFFS	  4
typedef struct
{
    Uint32   offset;
    Uint32   size;
} user_data_entry_t;

typedef struct
{
    Int16   left;
    Int16   right;
    Int16   top;
    Int16   bottom;
} win_t;

typedef struct
{
    Uint32 nal_hrd_param_present_flag                 : 1; // [    0]
    Uint32 vcl_hrd_param_present_flag                 : 1; // [    1]
    Uint32 sub_pic_hrd_params_present_flag            : 1; // [    2]
    Uint32 tick_divisor_minus2                        : 8; // [10: 3]
    Uint32 du_cpb_removal_delay_inc_length_minus1     : 5; // [15:11]
    Uint32 sub_pic_cpb_params_in_pic_timing_sei_flag  : 1; // [   16]
    Uint32 dpb_output_delay_du_length_minus1          : 5; // [21:17]
    Uint32 bit_rate_scale                             : 4; // [25:22]
    Uint32 cpb_size_scale                             : 4; // [29:26]
    Uint32 reserved_0                                 : 2; // [31:30]

    Uint32 initial_cpb_removal_delay_length_minus1    : 10; // [ 9: 0]
    Uint32 cpb_removal_delay_length_minus1            :  5; // [14:10]
    Uint32 dpb_output_delay_length_minus1             :  5; // [19:15]
    Uint32 reserved_1                                 : 12; // [31:20]

    Uint32 fixed_pic_rate_gen_flag[H265_MAX_NUM_SUB_LAYER];
    Uint32 fixed_pic_rate_within_cvs_flag[H265_MAX_NUM_SUB_LAYER];
    Uint32 low_delay_hrd_flag[H265_MAX_NUM_SUB_LAYER];
    Int32  cpb_cnt_minus1[H265_MAX_NUM_SUB_LAYER];
    Int32  elemental_duration_in_tc_minus1[H265_MAX_NUM_SUB_LAYER];

    Uint32 nal_bit_rate_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32 nal_cpb_size_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32 nal_cpb_size_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint32 nal_bit_rate_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint32 nal_cbr_flag[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];

    Uint32 vcl_bit_rate_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32 vcl_cpb_size_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32 vcl_cpb_size_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint32 vcl_bit_rate_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint32 vcl_cbr_flag[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
} h265_hrd_param_t;

typedef struct
{
    Uint32 aspect_ratio_info_present_flag             :  1; // [    0]
    Uint32 aspect_ratio_idc                           :  8; // [ 8: 1]
    Uint32 reserved_0                                 : 23; // [31: 9]

    Uint32 sar_width                                  : 16; // [15: 0]
    Uint32 sar_height                                 : 16; // [31:16]

    Uint32 overscan_info_present_flag                 :  1; // [    0]
    Uint32 overscan_appropriate_flag                  :  1; // [    1]
    Uint32 video_signal_type_present_flag             :  1; // [    2]
    Uint32 video_format                               :  3; // [ 5: 3]
    Uint32 video_full_range_flag                      :  1; // [    6]
    Uint32 colour_description_present_flag            :  1; // [    7]
    Uint32 colour_primaries                           :  8; // [15: 8]
    Uint32 transfer_characteristics                   :  8; // [23:16]
    Uint32 matrix_coefficients                        :  8; // [31:24]

    Uint32 chroma_loc_info_present_flag               :  1; // [    0]
    Int32  chroma_sample_loc_type_top_field           :  8; // [ 8: 1]
    Int32  chroma_sample_loc_type_bottom_field        :  8; // [16: 9]
    Uint32 neutral_chroma_indication_flag             :  1; // [   17]
    Uint32 field_seq_flag                             :  1; // [   18]
    Uint32 frame_field_info_present_flag              :  1; // [   19]
    Uint32 default_display_window_flag                :  1; // [   20]
    Uint32 vui_timing_info_present_flag               :  1; // [   21]
    Uint32 vui_poc_proportional_to_timing_flag        :  1; // [   22]
    Uint32 vui_hrd_parameters_present_flag            :  1; // [   23]
    Uint32 bitstream_restriction_flag                 :  1; // [   24]
    Uint32 tiles_fixed_structure_flag                 :  1; // [   25]
    Uint32 motion_vectors_over_pic_boundaries_flag    :  1; // [   26]
    Uint32 restricted_ref_pic_lists_flag              :  1; // [   27]
    Uint32 reserved_1                                 :  4; // [31:28]

    Uint32 vui_num_units_in_tick                       : 32; // [31: 0]
    Uint32 vui_time_scale                              : 32; // [31: 0]

    Uint32 min_spatial_segmentation_idc               : 12; // [11: 0]
    Uint32 max_bytes_per_pic_denom                    :  5; // [16:12]
    Uint32 max_bits_per_mincu_denom                   :  5; // [21:17]
    Uint32 log2_max_mv_length_horizontal              :  5; // [26:22]
    Uint32 log2_max_mv_length_vertical                :  5; // [31:27]

    Int32 vui_num_ticks_poc_diff_one_minus1           : 32; // [31: 0]

    win_t   def_disp_win;
    h265_hrd_param_t hrd_param;
} h265_vui_param_t;

typedef struct
{
    Uint32 display_primaries_x[3];
    Uint32 display_primaries_y[3];

    Uint32 white_point_x                   : 16;
    Uint32 white_point_y                   : 16;

    Uint32 max_display_mastering_luminance : 32;
    Uint32 min_display_mastering_luminance : 32;
} h265_mastering_display_colour_volume_t;

typedef struct
{
    Uint32 ver_chroma_filter_idc               : 8; // [ 7: 0]
    Uint32 hor_chroma_filter_idc               : 8; // [15: 8]
    Uint32 ver_filtering_field_processing_flag : 1; // [   16]
    Uint32 target_format_idc                   : 2; // [18:17]
    Uint32 num_vertical_filters                : 3; // [21:19]
    Uint32 num_horizontal_filters              : 3; // [24:22]
    Uint32 reserved_0                          : 7; // [31:25]

    Uint16 ver_tap_length_minus1[H265_MAX_NUM_VERTICAL_FILTERS];
    Uint16 hor_tap_length_minus1[H265_MAX_NUM_HORIZONTAL_FILTERS];

    Int32 ver_filter_coeff[H265_MAX_NUM_VERTICAL_FILTERS][H265_MAX_TAP_LENGTH];
    Int32 hor_filter_coeff[H265_MAX_NUM_HORIZONTAL_FILTERS][H265_MAX_TAP_LENGTH];
} h265_chroma_resampling_filter_hint_t;

typedef struct
{
    Uint32 knee_function_id                         : 32; // [31: 0]

    Uint32 knee_function_cancel_flag                :  1; // [    0]
    Uint32 knee_function_persistence_flag           :  1; // [    1]
    Uint32 reserved_0                               : 30; // [31: 2]

    Uint32 input_d_range                            : 32; // [31: 0]
    Uint32 input_disp_luminance                     : 32; // [31: 0]
    Uint32 output_d_range                           : 32; // [31: 0]
    Uint32 output_disp_luminance                    : 32; // [31: 0]
    Uint32 num_knee_points_minus1                   : 32; // [31: 0]

    Uint16 input_knee_point[H265_MAX_NUM_KNEE_POINT];
    Uint16 output_knee_point[H265_MAX_NUM_KNEE_POINT];
} h265_knee_function_info_t;

typedef struct
{
    Uint32 max_content_light_level                : 16; // [15: 0]
    Uint32 max_pic_average_light_level            : 16; // [31:16]
} h265_content_light_level_info_t;

typedef struct
{
    Uint32    colour_remap_id                                 : 32; // [31: 0]

    Uint32    colour_remap_cancel_flag                        :  1; // [    0]
    Uint32    colour_remap_persistence_flag                   :  1; // [    1]
    Uint32    colour_remap_video_signal_info_present_flag     :  1; // [    2]
    Uint32    colour_remap_full_range_flag                    :  1; // [    3]
    Uint32    colour_remap_primaries                          :  8; // [11: 4]
    Uint32    colour_remap_transfer_function                  :  8; // [19:12]
    Uint32    colour_remap_matrix_coefficients                :  8; // [27:20]
    Uint32    reserved_0                                      :  4; // [31:28]

    Uint32    colour_remap_input_bit_depth                    : 16; // [15: 0]
    Uint32    colour_remap_output_bit_depth                   : 16; // [31:16]

    Uint16     pre_lut_num_val_minus1[H265_MAX_LUT_NUM_VAL];

    Uint16    pre_lut_coded_value[H265_MAX_LUT_NUM_VAL][H265_MAX_LUT_NUM_VAL_MINUS1];

    Uint16    pre_lut_target_value[H265_MAX_LUT_NUM_VAL][H265_MAX_LUT_NUM_VAL_MINUS1];

    Uint32    colour_remap_matrix_present_flag                : 1; // [    0]
    Uint32    log2_matrix_denom                               : 4; // [ 4: 1]
    Uint32    reserved_4                                      : 27; // [31: 5]

    Uint16     colour_remap_coeffs[H265_MAX_COLOUR_REMAP_COEFFS][H265_MAX_COLOUR_REMAP_COEFFS];

    Uint16     post_lut_num_val_minus1[H265_MAX_LUT_NUM_VAL];

    Uint16    post_lut_coded_value[H265_MAX_LUT_NUM_VAL][H265_MAX_LUT_NUM_VAL_MINUS1];

    Uint16    post_lut_target_value[H265_MAX_LUT_NUM_VAL][H265_MAX_LUT_NUM_VAL_MINUS1];
} h265_colour_remapping_info_t;

typedef struct
{
    Uint32 film_grain_characteristics_cancel_flag              :  1; // [    0]
    Uint32 film_grain_model_id                                 :  2; // [ 2: 1]
    Uint32 separate_colour_description_present_flag            :  1; // [    3]
    Uint32 film_grain_bit_depth_luma_minus8                    :  3; // [ 6: 4]
    Uint32 film_grain_bit_depth_chroma_minus8                  :  3; // [ 9: 7]
    Uint32 film_grain_full_range_flag                          :  1; // [   10]
    Uint32 reserved_0                                          : 21; // [31:11]

    Uint32 film_grain_colour_primaries                         :  8; // [ 7: 0]
    Uint32 film_grain_transfer_characteristics                 :  8; // [15: 8]
    Uint32 film_grain_matrix_coeffs                            :  8; // [23:16]
    Uint32 blending_mode_id                                    :  2; // [25:24]
    Uint32 log2_scale_factor                                   :  4; // [29:26]
    Uint32 film_grain_characteristics_persistence_flag         :  1; // [   30]
    Uint32 reserved_1                                          :  1; // [   31]

    Uint32  comp_model_present_flag[H265_MAX_NUM_FILM_GRAIN_COMPONENT];

    Uint32  num_intensity_intervals_minus1[H265_MAX_NUM_FILM_GRAIN_COMPONENT];

    Uint32  num_model_values_minus1[H265_MAX_NUM_FILM_GRAIN_COMPONENT];

    Uint8  intensity_interval_lower_bound[H265_MAX_NUM_FILM_GRAIN_COMPONENT][H265_MAX_NUM_INTENSITY_INTERVALS];
    Uint8  intensity_interval_upper_bound[H265_MAX_NUM_FILM_GRAIN_COMPONENT][H265_MAX_NUM_INTENSITY_INTERVALS];

    Uint32 comp_model_value[H265_MAX_NUM_FILM_GRAIN_COMPONENT][H265_MAX_NUM_INTENSITY_INTERVALS][H265_MAX_NUM_MODEL_VALUES];
} h265_film_grain_characteristics_t;

typedef struct
{
    Uint32 tone_map_id                                        : 32; // [31: 0]

    Uint32 tone_map_cancel_flag                                :  1; // [    0]
    Uint32 tone_map_persistence_flag                           :  1; // [    1]
    Uint32 coded_data_bit_depth                                :  8; // [ 9: 2]
    Uint32 target_bit_depth                                    :  8; // [17:10]
    Uint32 tone_map_model_id                                   :  4; // [21:18]
    Uint32 reserved_0                                          : 10; // [31:22]

    Uint32 min_value                                          : 32; // [31: 0]
    Uint32 max_value                                          : 32; // [31: 0]
    Uint32 sigmoid_midpoint                                   : 32; // [31: 0]
    Uint32 sigmoid_width                                      : 32; // [31: 0]

    Uint16 start_of_coded_interval[H265_MAX_NUM_TONE_VALUE];

    Uint32 num_pivots                                         :32; // [31: 0]

    Uint16 coded_pivot_value[H265_MAX_NUM_TONE_VALUE];
    Uint16 target_pivot_value[H265_MAX_NUM_TONE_VALUE];

    Uint32 camera_iso_speed_idc                       :  8; // [ 7: 0]
    Uint32 exposure_index_idc                         :  8; // [15: 8]
    Uint32 exposure_compensation_value_sign_flag      :  1; // [   16]
    Uint32 reserved_1                                 : 15; // [31:17]

    Uint32 camera_iso_speed_value                     : 32; // [31: 0]

    Uint32 exposure_index_value                       : 32; // [31: 0]

    Uint32 exposure_compensation_value_numerator      : 16; // [15: 0]
    Uint32 exposure_compensation_value_denom_idc      : 16; // [31:16]

    Uint32 ref_screen_luminance_white                 : 32; // [31: 0]
    Uint32 extended_range_white_level                 : 32; // [31: 0]

    Uint32 nominal_black_level_code_value             : 16; // [15: 0]
    Uint32 nominal_white_level_code_value             : 16; // [31:16]

    Uint32 extended_white_level_code_value            : 16; // [15: 0]
    Uint32 reserved_2                                 : 16; // [31:16]
} h265_tone_mapping_info_t;

typedef struct
{
    Uint32 pic_struct                               :  4; // [ 3: 0]
    Uint32 source_scan_type                         :  2; // [ 5: 4]
    Uint32 duplicate_flag                           :  1; // [    6]
    Uint32 reserved_0                               : 25; // [31: 7]
} h265_sei_pic_timing_t;


/*******************************************************************************
 * H.264 USER DATA(VUI and SEI)                                                *
 *******************************************************************************/
typedef struct
{
    Int32 cpb_cnt                             : 16; // [15: 0]
    Uint32 bit_rate_scale                     :  8; // [23:16]
    Uint32 cbp_size_scale                     :  8; // [31:24]

    Int32 initial_cpb_removal_delay_length   : 16; // [15: 0]
    Int32 cpb_removal_delay_length           : 16; // [31:16]

    Int32 dpb_output_delay_length            : 16; // [15: 0]
    Int32 time_offset_length                 : 16; // [31:16]

    Uint32 bit_rate_value[32];

    Uint32 cpb_size_value[32];

    Uint8 cbr_flag[32];
} avc_hrd_param_t;

typedef struct
{
    Uint32 aspect_ratio_info_present_flag          :  1; // [    0]
    Uint32 aspect_ratio_idc                        :  8; // [ 8: 1]
    Uint32 reserved_0                              : 23; // [31: 9]

    Uint32 sar_width                               : 16; // [15: 0]
    Uint32 sar_height                              : 16; // [31:15]

    Uint32 overscan_info_present_flag              :  1; // [    0]
    Uint32 overscan_appropriate_flag               :  1; // [    1]
    Uint32 video_signal_type_present_flag          :  1; // [    2]
    Uint32 video_format                            :  3; // [ 5: 3]
    Uint32 video_full_range_flag                   :  1; // [    6]
    Uint32 colour_description_present_flag         :  1; // [    7]
    Uint32 colour_primaries                        :  8; // [15: 8]
    Uint32 transfer_characteristics                :  8; // [23:16]
    Uint32 matrix_coefficients                     :  8; // [31:24]

    Uint32 chroma_loc_info_present_flag            :  1; // [    0]
    Int32 chroma_sample_loc_type_top_field        :  8; // [ 8: 1]
    Int32 chroma_sample_loc_type_bottom_field     :  8; // [16: 9]
    Uint32 vui_timing_info_present_flag            :  1; // [   17]
    Uint32 reserved_1                              : 14; // [31:18]

    Uint32 vui_num_units_in_tick                   : 32;

    Uint32 vui_time_scale                          : 32;

    Uint32 vui_fixed_frame_rate_flag               :  1; // [    0]
    Uint32 vcl_hrd_parameters_present_flag         :  1; // [    1]
    Uint32 nal_hrd_parameters_present_flag         :  1; // [    2]
    Uint32 low_delay_hrd_flag                      :  1; // [    3]
    Uint32 pic_struct_present_flag                 :  1; // [    4]
    Uint32 bitstream_restriction_flag              :  1; // [    5]
    Uint32 motion_vectors_over_pic_boundaries_flag :  1; // [    6]
    Int32 max_bytes_per_pic_denom                 :  8; // [14: 7]
    Int32 max_bits_per_mincu_denom                :  8; // [22:15]
    Uint32 reserved_2                              :  9; // [31:23]

    Int32 log2_max_mv_length_horizontal           :  8; // [ 7: 0]
    Int32 log2_max_mv_length_vertical             :  8; // [15: 8]
    Int32 max_num_reorder_frames                  :  8; // [23:16]
    Int32 max_dec_frame_buffering                 :  8; // [31:24]

    avc_hrd_param_t vcl_hrd;
    avc_hrd_param_t nal_hrd;
} avc_vui_info_t;

typedef struct
{
    Uint32 cpb_removal_delay                         : 32;

    Uint32 dpb_output_delay                          : 32;

    Uint32 pic_struct                                :  4; // [ 3: 0]
    Uint32 num_clock_ts                              : 16; // [19: 4]
    Uint32 resreved_0                                : 11; // [31:20]
} avc_sei_pic_timing_t;

#define AVC_MAX_NUM_FILM_GRAIN_COMPONENT   3
#define AVC_MAX_NUM_INTENSITY_INTERVALS    256
#define AVC_MAX_NUM_MODEL_VALUES           5
#define AVC_MAX_NUM_TONE_VALUE             1024

typedef struct
{
    Uint32 film_grain_characteristics_cancel_flag         :  1; // [    0]
    Uint32 film_grain_model_id                            :  2; // [ 2: 1]
    Uint32 separate_colour_description_present_flag       :  1; // [    3]
    Uint32 reserved_0                                     : 28; // [31: 4]

    Uint32 film_grain_bit_depth_luma_minus8               :  3; // [ 2: 0]
    Uint32 film_grain_bit_depth_chroma_minus8             :  3; // [ 5: 3]
    Uint32 film_grain_full_range_flag                     :  1; // [    6]
    Uint32 film_grain_colour_primaries                    :  8; // [14: 7]
    Uint32 film_grain_transfer_characteristics            :  8; // [22:15]
    Uint32 film_grain_matrix_coeffs                       :  8; // [30:23]
    Uint32 reserved_1                                     :  1; // [   31]

    Uint32 blending_mode_id                               :  2; // [ 1: 0]
    Uint32 log2_scale_factor                              :  4; // [ 5: 2]
    Uint32 film_grain_characteristics_persistence_flag    :  1; // [    6]
    Uint32 reserved_2                                     : 25; // [31: 7]

    Uint32 comp_model_present_flag[AVC_MAX_NUM_FILM_GRAIN_COMPONENT];

    Uint32 num_intensity_intervals_minus1[AVC_MAX_NUM_FILM_GRAIN_COMPONENT];

    Uint32 num_model_values_minus1[AVC_MAX_NUM_FILM_GRAIN_COMPONENT];

    Uint8 intensity_interval_lower_bound[AVC_MAX_NUM_FILM_GRAIN_COMPONENT][AVC_MAX_NUM_INTENSITY_INTERVALS];
    Uint8 intensity_interval_upper_bound[AVC_MAX_NUM_FILM_GRAIN_COMPONENT][AVC_MAX_NUM_INTENSITY_INTERVALS];

    Uint32 comp_model_value[AVC_MAX_NUM_FILM_GRAIN_COMPONENT][AVC_MAX_NUM_INTENSITY_INTERVALS][AVC_MAX_NUM_MODEL_VALUES];
} avc_sei_film_grain_t;

typedef struct
{
    Uint32 tone_map_id                                : 32;

    Uint32 tone_map_cancel_flag                      :  1; // [    0]
    Uint32 reserved_0                                : 31; // [31: 1]

    Uint32 tone_map_repetition_period                 : 32;

    Uint32 coded_data_bit_depth                       :  8; // [ 7: 0]
    Uint32 target_bit_depth                           :  8; // [15: 8]
    Uint32 tone_map_model_id                          :  8; // [23:16]
    Uint32 reserved_1                                 :  8; // [31:24]

    Uint32 min_value                                  : 32;
    Uint32 max_value                                  : 32;
    Uint32 sigmoid_midpoint                           : 32;
    Uint32 sigmoid_width                              : 32;
    Uint16 start_of_coded_interval[AVC_MAX_NUM_TONE_VALUE]; // [1 << target_bit_depth] // 10bits

    Uint32 num_pivots                                 : 16; // [15: 0], [(1 << coded_data_bit_depth)?1][(1 << target_bit_depth)-1] // 10bits
    Uint32 reserved_2                                 : 16; // [31:16]

    Uint16 coded_pivot_value[AVC_MAX_NUM_TONE_VALUE];
    Uint16 target_pivot_value[AVC_MAX_NUM_TONE_VALUE];

    Uint32 camera_iso_speed_idc                       :  8; // [ 7: 0]
    Uint32 reserved_3                                 : 24; // [31: 8]

    Uint32 camera_iso_speed_value                     : 32;

    Uint32 exposure_index_idc                         :  8; // [ 7: 0]
    Uint32 reserved_4                                 : 24; // [31: 8]

    Uint32 exposure_index_value                       : 32;

    Uint32 exposure_compensation_value_sign_flag     :  1; // [    0]
    Uint32 reserved_5                                : 31; // [31: 1]

    Uint32 exposure_compensation_value_numerator     : 16; // [15: 0]
    Uint32 exposure_compensation_value_denom_idc     : 16; // [31:16]

    Uint32 ref_screen_luminance_white                : 32;
    Uint32 extended_range_white_level                : 32;

    Uint32 nominal_black_level_code_value            : 16; // [15: 0]
    Uint32 nominal_white_level_code_value            : 16; // [31:16]

    Uint32 extended_white_level_code_value           : 16; // [15: 0]
    Uint32 reserved_6                                : 16; // [31:16]
} avc_sei_tone_mapping_info_t;

typedef struct
{
    Uint32 colour_remap_id                                 : 32;

    Uint32 colour_cancel_flag                              :  1; // [    0]
    Uint32 colour_remap_repetition_period                  : 16; // [16: 1]
    Uint32 colour_remap_video_signal_info_present_flag     :  1; // [   17]
    Uint32 colour_remap_full_range_flag                    :  1; // [   18]
    Uint32 reserved_0                                      : 13; // [31:19]

    Uint32 colour_remap_primaries                          :  8; // [ 7: 0]
    Uint32 colour_remap_transfer_function                  :  8; // [15: 8]
    Uint32 colour_remap_matrix_coefficients                :  8; // [23:16]
    Uint32 reserved_1                                      :  8; // [31:24]

    Uint32 colour_remap_input_bit_depth                    :  8; // [ 7: 0]
    Uint32 colour_remap_output_bit_depth                   :  8; // [15: 8]
    Uint32 reserved_2                                      : 16; // [31:16]

    Uint32 pre_lut_num_val_minus1[3];
    Uint32 pre_lut_coded_value[3][33];
    Uint32 pre_lut_target_value[3][33];

    Uint32 colour_remap_matrix_present_flag                :  1; // [    0]
    Uint32 log2_matrix_denom                               :  4; // [ 4: 1]
    Uint32 reserved_3                                      : 27; // [31: 5]

    Uint32 colour_remap_coeffs[3][3];

    Uint32 post_lut_num_val_minus1[3];
    Uint32 post_lut_coded_value[3][33];
    Uint32 post_lut_target_value[3][33];
} avc_sei_colour_remap_info_t;


#ifdef __cplusplus
extern "C" {
#endif

RetCode InitCodecInstancePool(Uint32 coreIdx);
RetCode GetCodecInstance(Uint32 coreIdx, CodecInst ** ppInst);
void    FreeCodecInstance(CodecInst * pCodecInst);

int     DecBitstreamBufEmpty(DecInfo * pDecInfo);
RetCode SetParaSet(DecHandle handle, int paraSetType, DecParamSet * para);
void    DecSetHostParaAddr(Uint32 coreIdx, vpu_buffer_t *vb, Uint32 offset, PhysicalAddress paraAddr);

RetCode CheckInstanceValidity(CodecInst* pCodecInst);
Int32   ConfigSecAXICoda9(Uint32 coreIdx, Uint32 productId, Int32 codecMode, SecAxiInfo* sa, Uint32 width, Uint32 height, Uint32 profile);
RetCode UpdateFrameBufferAddr(TiledMapType mapType, FrameBuffer* fbArr, Uint32 numOfFrameBuffers, Uint32 sizeLuma, Uint32 sizeChroma);
RetCode AllocateTiledFrameBufferGdiV1(TiledMapType mapType, PhysicalAddress tiledBaseAddr, FrameBuffer* fbArr, Uint32 numOfFrameBuffers, Uint32 sizeLuma, Uint32 sizeChroma, DRAMConfig* pDramCfg);
RetCode AllocateTiledFrameBufferGdiV2(TiledMapType mapType, FrameBuffer* fbArr, Uint32 numOfFrameBuffers, Uint32 sizeLuma, Uint32 sizeChroma);

RetCode EnterLock(Uint32 coreIdx);
RetCode TryEnterLock(Uint32 coreIdx);
RetCode LeaveLock(Uint32 coreIdx);
RetCode SetClockGate(Uint32 coreIdx, Uint32 on);

RetCode EnterDispFlagLock(Uint32 coreIdx);
RetCode LeaveDispFlagLock(Uint32 coreIdx);

void       SetPendingInst(Uint32 coreIdx, CodecInst *inst);
void       ClearPendingInst(Uint32 coreIdx);
CodecInst* GetPendingInst(Uint32 coreIdx);
int        GetPendingInstIdx(Uint32 coreIdx);

Int32 MaverickCache2Config(MaverickCacheConfig* pCache, BOOL decoder, BOOL interleave, Uint32 bypass, Uint32 burst, Uint32 merge, TiledMapType mapType, Uint32 wayshape);
int   SetTiledMapType(Uint32 coreIdx, TiledMapConfig *pMapCfg, int mapType, int stride, int interleave, DRAMConfig *dramCfg);
int   GetXY2AXIAddr(TiledMapConfig *pMapCfg, int ycbcr, int posY, int posX, int stride, FrameBuffer *fb);
int   GetLowDelayOutput(CodecInst *pCodecInst, DecOutputInfo *lowDelayOutput);
Int32 CalcLumaSize(CodecInst* inst, Int32 productId, Int32 stride, Int32 height, FrameBufferFormat format, BOOL cbcrIntl, TiledMapType mapType, DRAMConfig* pDramCfg, Int32 *pHdrSize);
Int32 CalcChromaSize(CodecInst* inst, Int32 productId, Int32 stride, Int32 height, FrameBufferFormat format, BOOL cbcrIntl, TiledMapType mapType, DRAMConfig* pDramCfg, Int32 *pHdrSize);

//for GDI 1.0
void            SetTiledFrameBase(Uint32 coreIdx, PhysicalAddress baseAddr);
PhysicalAddress GetTiledFrameBase(Uint32 coreIdx, FrameBuffer *frame, int num);

RetCode CheckEncInstanceValidity(EncHandle handle);
RetCode EncParaSet(EncHandle handle, int paraSetType);
RetCode SetSliceMode(EncHandle handle, EncSliceMode *pSliceMode);
RetCode SetHecMode(EncHandle handle, int mode);
void    EncSetHostParaAddr(Uint32 coreIdx, vpu_buffer_t *vb, Uint32 offset, PhysicalAddress paraAddr);
int     LevelCalculation(int MbNumX, int MbNumY, int frameRateInfo, int interlaceFlag, int BitRate, int SliceNum);
/* timescale: 1/90000 */
Uint64  GetTimestamp(EncHandle handle);
RetCode SetEncCropInfo(Int32 codecMode, VpuRect* param, int rotMode, int srcWidth, int srcHeight);

void LoadVaApiParameter(Uint32 coreIdx, Int32 codec, vpu_buffer_t *vbParam, vpu_buffer_t *vbWork);
Uint32 CalcMinFrameBufferCount(Uint32 coreIdx, Int32 codec, vpu_buffer_t *vb);
#if defined(SUPPORT_SW_UART) || defined(SUPPORT_SW_UART_V2)
/* void SwUartHandler(void *context); */
int  create_sw_uart_thread(unsigned long coreIdx);
void destroy_sw_uart_thread(unsigned long coreIdx);
#endif
#ifdef __cplusplus
}
#endif

#endif // endif VPUAPI_UTIL_H_INCLUDED

