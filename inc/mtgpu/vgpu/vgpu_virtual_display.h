/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __VGPU_VIRTUAL_DISPLAY_H__
#define __VGPU_VIRTUAL_DISPLAY_H__

#include "linux-list.h"
#include "linux-types.h"
#include "os-interface.h"

#define VDISP_VSYNC_STATUS_ON		(0x1U)
#define VDISP_VSYNC_STATUS_IRQ		(0x2U)
#define VDISP_VSYNC_STATUS_SHIFT	(8)

#define VDISP_IS_VSYNC_ON(status, id)		\
	(((status) & (VDISP_VSYNC_STATUS_ON << (VDISP_VSYNC_STATUS_SHIFT * id))) != 0)

#define VDISP_SET_VSYNC_ON(status, id)		\
	((status) |= (VDISP_VSYNC_STATUS_ON << (VDISP_VSYNC_STATUS_SHIFT * id)))

#define VDISP_SET_VSYNC_OFF(status, id)		\
	((status) &= ~(VDISP_VSYNC_STATUS_ON << (VDISP_VSYNC_STATUS_SHIFT * id)))

#define VDISP_IS_VSYNC_IRQ(status, id)		\
	(((status) & (VDISP_VSYNC_STATUS_IRQ << (VDISP_VSYNC_STATUS_SHIFT * id))) != 0)

#define VDISP_VSYNC_SET_IRQ(status, id)		\
	((status) |= (VDISP_VSYNC_STATUS_IRQ << (VDISP_VSYNC_STATUS_SHIFT * id)))

#define VDISP_VSYNC_CLEAR_IRQ(status, id)	\
	((status) &= ~(VDISP_VSYNC_STATUS_IRQ << (VDISP_VSYNC_STATUS_SHIFT * id)))

struct device;

#if (RGX_NUM_OS_SUPPORTED > 1)
struct virtual_display {
	u32 id;
	u32 instance_id;
	ktime_t interval;
	ktime_t next_time;

	struct rb_node *node;
};

int vgpu_vdisplay_vsync_balancing(struct virtual_display **vdisps, u32 count);

int vgpu_vdisplay_register(struct virtual_display *vdisps, u32 data);
void vgpu_vdisplay_unregister(struct virtual_display *vdisps, u32 disp_id);
int vgpu_virtual_display_init(void);
void vgpu_virtual_display_exit(void);

void vgpu_vdisplay_vsync_register(struct device *dev, u32 disp_id, u32 vrefresh);
void vgpu_vdisplay_vsync_unregister(struct device *dev, u32 disp_id);
#else
#ifndef MT_UNREFERENCED_PARAMETER
#define MT_UNREFERENCED_PARAMETER(param) ((void)(param))
#endif

struct virtual_display;
static inline int vgpu_vdisplay_register(struct virtual_display *vdisps, u32 data)
{
	MT_UNREFERENCED_PARAMETER(vdisps);
	MT_UNREFERENCED_PARAMETER(data);

	return 0;
}

static inline void vgpu_vdisplay_unregister(struct virtual_display *vdisps, u32 disp_id)
{
	MT_UNREFERENCED_PARAMETER(vdisps);
	MT_UNREFERENCED_PARAMETER(disp_id);
}

static inline int vgpu_virtual_display_init(void)
{
	return 0;
}

static inline void vgpu_virtual_display_exit(void)
{
}

static inline void vgpu_vdisplay_vsync_register(struct device *dev, u32 disp_id, u32 vrefresh)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(disp_id);
	MT_UNREFERENCED_PARAMETER(vrefresh);

}

static inline void vgpu_vdisplay_vsync_unregister(struct device *dev, u32 disp_id)
{
	MT_UNREFERENCED_PARAMETER(dev);
	MT_UNREFERENCED_PARAMETER(disp_id);
}

#endif /* (RGX_NUM_OS_SUPPORTED > 1) */

#endif /* __VGPU_VIRTUAL_DISPLAY_H__ */
