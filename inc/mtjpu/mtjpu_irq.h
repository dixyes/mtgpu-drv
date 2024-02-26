/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTJPU_IRQ_H__
#define __MTJPU_IRQ_H__

struct jpu_drv_context {
	u32 interrupt_reason;
	struct device *dev;
};

struct platform_device;
struct device;

void mtjpu_irq_handle(void *data);
void mtjpu_irq_data_init(struct device *dev);
int mtjpu_irq_init(struct platform_device *plat_dev, int device_id);
int mtjpu_irq_free(struct platform_device *plat_dev, int device_id);
int mtjpu_wait_interrupt(int timeout, int inst_idx);

#endif /* __MTJPU_IRQ_H__ */

