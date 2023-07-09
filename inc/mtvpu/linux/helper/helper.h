/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _HELPER_H_
#define _HELPER_H_

#include "vpuapi.h"

RetCode AllocateDecFrameBuffer(DecHandle hdl, DecInitialInfo *info, RenderTargetInfo *render);
RetCode AllocateEncFrameBuffer(EncHandle hdl, EncInitialInfo *info);
RetCode AllocateEncAuxBuffer(EncHandle hdl, EncInitialInfo *info);

Int32 GetW6EncOpenParamDefault(EncOpenParam *pEncOP);
void setPvricValue(DecParam *decParam);
void get_fbc_info(int bpp, int width, int height, int *pFormat, int *pHdrSize, int *pDataSize);
#endif
