/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_IRQ_H__
#define __MTGPU_IRQ_H__

struct mtgpu_device;

int mtgpu_interrupt_init(struct mtgpu_device *mtgpu);
void mtgpu_interrupt_exit(struct mtgpu_device *mtgpu);
int mtgpu_interrupt_resume(struct mtgpu_device *mtgpu);
int mtgpu_interrupt_suspend(struct mtgpu_device *mtgpu);

int mtgpu_enable_interrupt(struct device *mtgpu, int int_vec);
int mtgpu_disable_interrupt(struct device *mtdev, int int_vec);
int mtgpu_register_interrupt(struct device *mtdev, int int_vec,
			     void (*handler)(void *),
			     void *data, const char *name);
int mtgpu_unregister_interrupt(struct device *mtdev, int int_vec);

/* for compatibility with other drivers, this interface is retained */
int mtgpu_set_interrupt_handler(struct device *mtdev, int interrupt_id,
				void (*handler_function)(void *),
				void *handler_data);

#endif /* __MTGPU_IRQ_H__ */
