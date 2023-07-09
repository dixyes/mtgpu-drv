/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_SND_H__
#define __MTGPU_SND_H__

#ifdef SND_ENABLE
int mtsnd_init(void);
void mtsnd_deinit(void);
#else
static inline int mtsnd_init(void)
{
	return 0;
}

static inline void mtsnd_deinit(void)
{
}
#endif

#endif /* __MTGPU_SND_H__ */
