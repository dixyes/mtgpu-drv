/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTJPU_DEC_H__
#define __MTJPU_DEC_H__

#include "mtjpu_config.h"
#include "mtjpu_types.h"
#include "mtjpu_api.h"
#include "mtjpu_buf.h"

struct drm_ioctl_desc;

enum JPU_DEC_CMD {
	DEC_OPEN = 0,
	DEC_REGISTER_FRAME,
	DEC_REGISTER_BUFPOOL,
	DEC_START,
	DEC_UNREGISTER_BUFPOOL,
	DEC_GET_INITIAL_INFO,
	DEC_GET_OUTPUT,
	DEC_CLOSE
};

struct misc_param {
	u32 stride;
	u32 int_reason;
	u32 buf_count;
	u32 bs_size;
};

struct mtjpu_dec_param {
	int cmd;
	u32 core_idx;
	u32 inst_idx;
	JpgDecHandle handle;
	struct misc_param misc;
	JpgFrameBuffer framebuf[MAX_FRAME];
	JpgDecOpenParam decop;
	JpgDecParam dec_param;
	JpgDecOutputInfo output;
	JpgDecInitialInfo initialInfo;
	struct jpu_buffer jdb_bs;
	struct jpu_buffer jdb_yuv;
};

#define DRM_IOCTL_MT_JPU_DEC_CMD DRM_IOWR(DRM_COMMAND_BASE + 0x4d, struct mtjpu_dec_param)

int mtjpu_dec_cmd(struct drm_device *drm, void *data, struct drm_file *file);

#endif /* _MTJPU_DEC_H__ */
