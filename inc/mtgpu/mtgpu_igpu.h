/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_IGPU_H__
#define __MTGPU_IGPU_H__

struct platform_device;

int mtgpu_igpu_probe(struct platform_device *pdev);
int mtgpu_igpu_remove(struct platform_device *pdev);

#endif /* __MTGPU_IGPU_H__ */
