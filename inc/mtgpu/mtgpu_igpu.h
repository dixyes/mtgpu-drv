/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_IGPU_H__
#define __MTGPU_IGPU_H__

/* IGPU's default delay suspend time */
#define MTGPU_IGPU_SUSPEND_DELAY_MS 20

struct device;
struct platform_device;
struct mtgpu_device;

struct mtgpu_igpu_dvfs {
	u32 freq_min;		/* MHz */
	u32 freq_max;		/* MHz */
	u32 freq_store;		/* MHz */
	u32 freq_points_count;	/* Supported frequency points count */
	void __iomem *share_mem;/* Write frequency into shared memory,
				 * actual configuration will be completed
				 * by scp-firmware.
				 */
};

int mtgpu_igpu_probe(struct platform_device *pdev);
int mtgpu_igpu_remove(struct platform_device *pdev);
int mtgpu_igpu_frequency_set(struct mtgpu_device *mtdev, u32 freq);
int mtgpu_igpu_get_frequency_limits(struct mtgpu_device *mtdev,
				    u32 *min_freq, u32 *max_freq);
int mtgpu_igpu_get_frequency_points_count(struct mtgpu_device *mtdev,
					  u32 *count);
int mtgpu_igpu_pm_runtime_get(struct mtgpu_device *mtdev);
int mtgpu_igpu_pm_runtime_put(struct mtgpu_device *mtdev);
int mtgpu_igpu_pm_suspend(struct device *dev);
int mtgpu_igpu_pm_resume(struct device *dev);
void mtgpu_igpu_pm_force_reset(struct device *dev);

#endif /* __MTGPU_IGPU_H__ */
