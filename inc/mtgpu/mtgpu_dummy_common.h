/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */
#ifndef _MTGPU_DUMMY_COMMON_H_
#define _MTGPU_DUMMY_COMMON_H_

#include "mtgpu_display_debug.h"
#include "os-interface-drm.h"


struct mtgpu_dummy_ctx {
	u8 id;
	u16 max_hres;
	u16 max_vres;
};

struct mtgpu_dummy {
	struct device *dev;
	struct drm_encoder *encoder;
	struct drm_connector *connector;
	struct mtgpu_dummy_ctx ctx;
};

static inline
struct mtgpu_dummy *connector_to_mtgpu_dummy(struct drm_connector *connector)
{
	return os_get_drm_connector_drvdata(connector);
}

#endif /* _MTGPU_DUMMY_COMMON_H_ */
