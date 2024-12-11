/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_IGPU_H__
#define __MTGPU_IGPU_H__

/* IGPU's default delay suspend time */
#define MTGPU_IGPU_SUSPEND_DELAY_MS 100

struct platform_device;
struct mtgpu_device;

struct mtgpu_igpu_dvfs {
	u32 freq_min;		/* MHz */
	u32 freq_max;		/* MHz */
	void __iomem *share_mem;/* Write frequency into shared memory,
				 * actual configuration will be completed
				 * by scp-firmware.
				 */
};

int mtgpu_igpu_probe(struct platform_device *pdev);
int mtgpu_igpu_remove(struct platform_device *pdev);
int mtgpu_igpu_frequency_set(struct mtgpu_device *mtdev, u32 freq);
int mtgpu_igpu_pm_runtime_get(struct mtgpu_device *mtdev);
int mtgpu_igpu_pm_runtime_put(struct mtgpu_device *mtdev);

#endif /* __MTGPU_IGPU_H__ */
