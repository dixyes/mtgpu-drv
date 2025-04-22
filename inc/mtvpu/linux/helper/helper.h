/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _HELPER_H_
#define _HELPER_H_

#include "vpuapi.h"

RetCode SetDecBufPool(DecHandle hdl, DecInitialInfo *info, RenderTargetInfo *render);
RetCode AllocateEncFrameBufferV2(EncHandle hdl, EncInitialInfo *info);

Int32 GetW6EncOpenParamDefault(EncOpenParam *pEncOP);
void setPvricValue(DecParam *decParam);
int FillBufferInfo(vpu_buffer_t* vb, Uint64 handle, Uint64 size,
		  struct drm_file *file, struct mt_virm *vm, struct mt_chip *chip);

#endif
