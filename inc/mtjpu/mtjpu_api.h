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

#ifndef __MTJPU_API_H__
#define __MTJPU_API_H__

#include "mtjpu_config.h"
#include "mtjpu_types.h"

#define JPU_CEIL(_s, _n)  (((_n) + (_s - 1)) & ~(_s - 1))
#define JPU_FLOOR(_s, _n) (_n & ~(_s - 1))

typedef enum {
	JPEG_BASELINE_DCT,
	JPEG_EXTENDED_SEQUENTIAL_DCT,
} JpgProfile;

typedef enum {
	SET_JPG_SCALE_HOR,
	SET_JPG_SCALE_VER,
	SET_JPG_USE_STUFFING_BYTE_FF,
	SET_JPG_QUALITY_FACTOR,
	ENC_JPG_GET_HEADER,
	ENABLE_JPG_LOGGING,
	DISABLE_JPG_LOGGING,
	JPG_CMD_END
} JpgCommand;

typedef enum {
	JPG_RET_SUCCESS,
	JPG_RET_FAILURE,
	JPG_RET_BIT_EMPTY,
	JPG_RET_EOS,
	JPG_RET_INVALID_HANDLE,
	JPG_RET_INVALID_PARAM,
	JPG_RET_INVALID_COMMAND,
	JPG_RET_ROTATOR_OUTPUT_NOT_SET,
	JPG_RET_ROTATOR_STRIDE_NOT_SET,
	JPG_RET_FRAME_NOT_COMPLETE,
	JPG_RET_INVALID_FRAME_BUFFER,
	JPG_RET_INSUFFICIENT_FRAME_BUFFERS,
	JPG_RET_INVALID_STRIDE,
	JPG_RET_WRONG_CALL_SEQUENCE,
	JPG_RET_CALLED_BEFORE,
	JPG_RET_NOT_INITIALIZED,
	JPG_RET_INSUFFICIENT_RESOURCE,
	JPG_RET_INST_CTRL_ERROR,
	JPG_RET_NOT_SUPPORT,
	JPG_RET_INSUFFICIENT_BITSTREAM_BUFFER,
} JpgRet;

typedef enum {
	ENCODE_STATE_NEW_FRAME = 0,
	ENCODE_STATE_FRAME_DONE = 0,
	ENCODE_STATE_SLICE_DONE = 1
} EncodeState;

typedef enum {
	JPG_MIRDIR_NONE,
	JPG_MIRDIR_VER,
	JPG_MIRDIR_HOR,
	JPG_MIRDIR_HOR_VER
} JpgMirrorDirection;

typedef enum {
	JPG_FORMAT_420 = 0,
	JPG_FORMAT_422 = 1,
	JPG_FORMAT_440 = 2,
	JPG_FORMAT_444 = 3,
	JPG_FORMAT_400 = 4,
	JPG_FORMAT_MAX
} FrameFormat;

typedef enum {
	JPG_CBCR_SEPARATED = 0,
	JPG_CBCR_INTERLEAVE,
	JPG_CRCB_INTERLEAVE
} CbCrInterLeave;

typedef enum {
	PACKED_JPG_FORMAT_NONE,
	PACKED_JPG_FORMAT_422_YUYV,
	PACKED_JPG_FORMAT_422_UYVY,
	PACKED_JPG_FORMAT_422_YVYU,
	PACKED_JPG_FORMAT_422_VYUY,
	PACKED_JPG_FORMAT_444,
	PACKED_JPG_FORMAT_MAX
} PackedFormat;

enum JPU_PIXEL_JUSTIFICATION {
	PIXEL_16BIT_MSB_JUSTIFIED,
	PIXEL_16BIT_LSB_JUSTIFIED,
	PIXEL_16BIT_JUSTIFICATION_MAX,
};

typedef enum {
	INT_JPU_DONE = 0,
	INT_JPU_ERROR = 1,
	INT_JPU_BIT_BUF_EMPTY = 2,
	INT_JPU_BIT_BUF_FULL = 2,
#ifdef SUPPORT_STOP_CHECKING
	INT_JPU_STOP_DONE = 8,
#endif
	INT_JPU_SLICE_DONE = 9,
}InterruptJpu;

typedef enum {
	JPG_TBL_NORMAL,
} JpgTableMode;

typedef enum {
	ENC_HEADER_MODE_NORMAL,
	ENC_HEADER_MODE_SOS_ONLY
} JpgEncHeaderMode;

enum JPU_DOWNSCALE_VAL {
	JPG_SCALE_DOWN_NONE,
	JPG_SCALE_DOWN_ONE_HALF,
	JPG_SCALE_DOWN_ONE_QUARTER,
	JPG_SCALE_DOWN_ONE_EIGHTS,
	JPG_SCALE_DOWN_MAX
};

enum JPU_PRODUCT_ID {
	PRODUCT_ID_CODAJ12 = 12,
};

typedef struct {
	PhysicalAddress bufY;
	PhysicalAddress bufCb;
	PhysicalAddress bufCr;
	Uint32          stride;
	Uint32          strideC;
#ifdef SUPPORT_PADDING_UNALIGNED_YUV
	Uint32          fbLumaHeight;
	Uint32          fbChromaHeight;
#endif
	FrameFormat     format;
	Uint32          endian;
} JpgFrameBuffer;

struct JpgInst;

typedef struct JpgInst * JpgHandle;

typedef struct JpgInst JpgDecInst;

typedef JpgDecInst * JpgDecHandle;

typedef struct {
	Uint32  productId;
	Uint32  revisoin;
	BOOL    support12bit;
} JPUCap;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32          bitstreamBufferSize;
	BYTE            *pBitStream;
	Uint32          streamEndian;
	Uint32          frameEndian;
	CbCrInterLeave  chromaInterleave;
	PackedFormat    packedFormat;
	BOOL            roiEnable;
	Uint32          roiOffsetX;
	Uint32          roiOffsetY;
	Uint32          roiWidth;
	Uint32          roiHeight;
	Uint32          pixelJustification;
	Uint32          intrEnableBit;
	Uint32          rotation;
	JpgMirrorDirection  mirror;
	FrameFormat     outputFormat;
} JpgDecOpenParam;

typedef struct {
	int         picWidth;
	int         picHeight;
	int         minFrameBufferCount;
	FrameFormat sourceFormat;
	int         ecsPtr;
	int         roiFrameWidth;
	int         roiFrameHeight;
	int         roiFrameOffsetX;
	int         roiFrameOffsetY;
	Uint32      bitDepth;
} JpgDecInitialInfo;

typedef struct {
	int       scaleDownRatioWidth;
	int       scaleDownRatioHeight;
} JpgDecParam;

typedef struct {
	int     indexFrameDisplay;
	int     numOfErrMBs;
	int     decodingSuccess;
	int     decPicHeight;
	int     decPicWidth;
	int     consumedByte;
	int     bytePosFrameStart;
	int     ecsPtr;
	Uint32  frameCycle;
	Uint32  rdPtr;
	Uint32  wrPtr;
} JpgDecOutputInfo;

typedef struct JpgInst JpgEncInst;

typedef JpgEncInst * JpgEncHandle;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32          bitstreamBufferSize;
	Uint32          picWidth;
	Uint32          picHeight;
	FrameFormat     sourceFormat;
	Uint32          restartInterval;
	Uint32          streamEndian;
	Uint32          frameEndian;
	CbCrInterLeave  chromaInterleave;
	BYTE            huffVal[8][256];
	BYTE            huffBits[8][256];
	short           qMatTab[4][64];
	BOOL            jpg12bit;
	BOOL            q_prec0;
	BOOL            q_prec1;
	PackedFormat    packedFormat;
	Uint32          pixelJustification;
	Uint32          tiledModeEnable;
	Uint32          sliceHeight; 
	Uint32          intrEnableBit;
	BOOL            sliceInstMode;
	Uint32          rotation;
	Uint32          mirror;
} JpgEncOpenParam;

typedef struct {
	JpgFrameBuffer *sourceFrame;
} JpgEncParam;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32 bitstreamSize;
	PhysicalAddress streamRdPtr;
	PhysicalAddress streamWrPtr;
	Uint32 encodedSliceYPos;
	EncodeState encodeState;
	Uint32  frameCycle;
} JpgEncOutputInfo;

typedef struct {
	PhysicalAddress paraSet;
	BYTE *pParaSet;
	int size;
	int headerMode;
	int quantMode;
	int huffMode;
	int disableAPPMarker;
	int enableSofStuffing;
} JpgEncParamSet;

Uint32 JPU_IsInit(Int32 inst_idx);

Int32 JPU_WaitInterrupt(JpgHandle handle, int timeout);
int JPU_IsBusy(JpgHandle handle);
Uint32 JPU_GetStatus(JpgHandle handle);
void JPU_ClrStatus(JpgHandle handle, Uint32 val);

JpgRet JPU_Init(int chip_idx, int *inst_idx);
void JPU_DeInit(int inst_idx);
int JPU_GetOpenInstanceNum(int inst_idx);
JpgRet JPU_GetVersionInfo(Int32 inst_idx, Uint32 *apiVersion, Uint32 *hwRevision,
			  Uint32 *hwProductId);

JpgRet JPU_HWReset(void);
JpgRet JPU_SWReset(int inst_idx);

JpgRet JPU_DecOpen(int inst_idx, JpgDecHandle *handle, JpgDecOpenParam *openParam);
JpgRet JPU_DecClose(JpgDecHandle handle);
JpgRet JPU_DecGetInitialInfo(JpgDecHandle handle, JpgDecInitialInfo *info, u64 bs_buf);
JpgRet JPU_DecRegisterFrameBuffer(JpgDecHandle handle, JpgFrameBuffer *bufArray,
				  int num, int stride);
JpgRet JPU_DecGetBitstreamBuffer(JpgDecHandle handle, PhysicalAddress *prdPrt,
				 PhysicalAddress *pwrPtr, int *size);
JpgRet JPU_DecUpdateBitstreamBuffer(JpgDecHandle handle, int size);

JpgRet JPU_DecStartOneFrame(JpgDecHandle handle, JpgDecParam *param);
JpgRet JPU_DecGetOutputInfo(JpgDecHandle handle, JpgDecOutputInfo *info);
JpgRet JPU_DecGiveCommand(JpgDecHandle handle, JpgCommand cmd, void *parameter);
JpgRet JPU_DecSetRdPtr(JpgDecHandle handle, PhysicalAddress addr, BOOL updateWrPtr);
JpgRet JPU_DecSetRdPtrEx(JpgDecHandle handle, PhysicalAddress addr, BOOL updateWrPtr);

JpgRet JPU_EncOpen(JpgEncHandle *handle, JpgEncOpenParam *openParam);
JpgRet JPU_EncClose(JpgEncHandle handle);
JpgRet JPU_EncGetBitstreamBuffer(JpgEncHandle handle, PhysicalAddress *prdPrt,
				 PhysicalAddress *pwrPtr, int *size);
JpgRet JPU_EncUpdateBitstreamBuffer(JpgEncHandle handle, int size);
JpgRet JPU_EncStartOneFrame(JpgEncHandle handle, JpgEncParam *param);
JpgRet JPU_EncGetOutputInfo(JpgEncHandle handle, JpgEncOutputInfo *info);
JpgRet JPU_EncGiveCommand(JpgEncHandle handle, JpgCommand cmd, void *parameter);

#endif /* __MTJPU_API_H__ */
