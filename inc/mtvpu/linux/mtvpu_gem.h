/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef _MTVPU_GEM_H_
#define _MTVPU_GEM_H_

#include "mtvpu_drv.h"

enum vram_allocater {
	LINUX_NATIVE,
	WIN_GUEST,
	LINUX_GUEST,
	LINUX_HOST,
};

struct mt_node *gem_malloc_node(struct mt_chip *chip, int idx,
					 struct drm_device *drm, u64 size, u32 type);
void gem_free_node(struct mt_chip *chip, struct mt_node *node);

#ifdef SUPPORT_ION
/*
struct mt_node *ion_malloc_node(struct mt_chip *chip, int idx, int drm_id, u64 size);
int ion_free_node(struct mt_node *);
*/
u64 get_dev_addr_dma_buf(struct dma_buf *psDmaBuf);
#endif

#endif /* _MTVPU_GEM_H_ */
