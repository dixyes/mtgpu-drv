/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _HELPER_H_
#define _HELPER_H_

#include "vpuapi.h"

#define UMD_ALLOC_BUFFER(x)     (x == DEC_FBCC_TBL || x == DEC_FBCY_TBL || x == DEC_FBC || x == DEC_MV || \
                                 x == DEC_WORK || x == DEC_TASK || x == DEC_ETC || x == DEC_VA_PARAM || \
                                 x == ENC_DEF_CDF || x == ENC_WORK)

RetCode AllocateDecFrameBuffer(DecHandle hdl, DecInitialInfo *info, RenderTargetInfo *render);
RetCode AllocateEncFrameBuffer(EncHandle hdl, EncInitialInfo *info);
RetCode AllocateEncFrameBufferV2(EncHandle hdl, EncInitialInfo *info);
RetCode AllocateEncAuxBuffer(EncHandle hdl, EncInitialInfo *info);

Int32 GetW6EncOpenParamDefault(EncOpenParam *pEncOP);
void setPvricValue(DecParam *decParam);

#endif
