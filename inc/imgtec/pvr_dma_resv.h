/*************************************************************************/ /*!
@Title          Kernel reservation object compatibility header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Per-version macros to allow code to seamlessly use older kernel
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#ifndef __PVR_DMA_RESV_H__
#define __PVR_DMA_RESV_H__

#if defined(OS_LINUX_FENCE_H_EXIST)
#include <linux/fence.h>
#else
#include <linux/dma-fence.h>
#endif

#if defined(OS_LINUX_DMA_RESV_H_EXIST)
#include <linux/dma-resv.h>
#else
#include <linux/reservation.h>
#endif

#if defined(OS_LINUX_FENCE_H_EXIST)
#define dma_fence fence
#endif

#if !defined(OS_LINUX_DMA_RESV_H_EXIST)
#define dma_resv reservation_object
#endif

static inline int pvr_dma_resv_reserve_shared_fences(struct dma_resv *obj,
						     unsigned int num_fences)
{
#if defined(OS_FUNC_DMA_RESV_RESERVE_FENCES_EXIST)
	return dma_resv_reserve_fences(obj, num_fences);
#elif defined(OS_LINUX_DMA_RESV_H_EXIST)
	return dma_resv_reserve_shared(obj, num_fences);
#elif defined(OS_RESERVATION_OBJECT_RESERVE_SHARED_HAS_NUM_FENCES_ARG)
	return reservation_object_reserve_shared(obj, num_fences);
#else
	unsigned int i;
	int err;
	for (i = 0; i < num_fences; i++) {
		err = reservation_object_reserve_shared(obj);
		if (err)
			return err;
	}
	return 0;
#endif
}

static inline void pvr_dma_resv_add_excl_fence(struct dma_resv *obj,
					       struct dma_fence *fence)
{
#if defined(OS_FUNC_DMA_RESV_ADD_FENCE_EXIST)
	dma_resv_reserve_fences(obj, 1);

	dma_resv_add_fence(obj, fence, DMA_RESV_USAGE_WRITE);
#elif defined(OS_LINUX_DMA_RESV_H_EXIST)
	dma_resv_add_excl_fence(obj, fence);
#else
	reservation_object_add_excl_fence(obj, fence);
#endif
}

static inline void pvr_dma_resv_add_shared_fence(struct dma_resv *obj,
						 struct dma_fence *fence)
{
#if defined(OS_FUNC_DMA_RESV_ADD_FENCE_EXIST)
	dma_resv_add_fence(obj, fence, DMA_RESV_USAGE_READ);
#elif defined(OS_LINUX_DMA_RESV_H_EXIST)
	dma_resv_add_shared_fence(obj, fence);
#else
	reservation_object_add_shared_fence(obj, fence);
#endif
}

static inline int pvr_dma_resv_get_fences(struct dma_resv *obj,
					  struct dma_fence **pfence_excl,
					  unsigned int *num_fences,
					  struct dma_fence ***pfences,
					  bool usage_write,
					  bool *fence_overall)
{
	*fence_overall = false;

#if defined(OS_ENUM_DMA_RESV_USAGE_EXIST)
	*fence_overall = true;

	return dma_resv_get_fences(obj, usage_write ? DMA_RESV_USAGE_READ :
				   DMA_RESV_USAGE_WRITE, num_fences, pfences);
#elif defined(OS_FUNC_DMA_RESV_GET_FENCES_EXIST)
	return dma_resv_get_fences(obj, pfence_excl, num_fences, pfences);
#elif defined(OS_LINUX_DMA_RESV_H_EXIST)
	return dma_resv_get_fences_rcu(obj, pfence_excl, num_fences, pfences);
#else
	return reservation_object_get_fences_rcu(obj, pfence_excl, num_fences, pfences);
#endif
}

#if !defined(OS_LINUX_DMA_RESV_H_EXIST)
/* Reservation object functions */
#define dma_resv_add_excl_fence reservation_object_add_excl_fence
#define dma_resv_add_shared_fence reservation_object_add_shared_fence
#define dma_resv_fini reservation_object_fini
#define dma_resv_get_excl reservation_object_get_excl
#define dma_resv_get_list reservation_object_get_list
#define dma_resv_held reservation_object_held
#define dma_resv_init reservation_object_init
#define dma_resv_reserve_shared reservation_object_reserve_shared
#define dma_resv_test_signaled_rcu reservation_object_test_signaled_rcu
#define dma_resv_wait_timeout_rcu reservation_object_wait_timeout_rcu
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))

#define dma_resv_shared_list dma_resv_get_list
#define dma_resv_excl_fence dma_resv_get_excl
#define dma_resv_wait_timeout dma_resv_wait_timeout_rcu
#define dma_resv_test_signaled dma_resv_test_signaled_rcu
#define dma_resv_get_fences dma_resv_get_fences_rcu

#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)) */

#endif /* __PVR_DMA_RESV_H__ */
