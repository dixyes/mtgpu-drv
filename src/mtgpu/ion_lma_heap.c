/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include "mtgpu_ion.h"

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/genalloc.h>
#include <linux/scatterlist.h>

#include "ion_lma_heap.h"
#include "ion.h"
#include "pvrsrv.h"
#include "mtgpu_segment.h"

#define ION_CARVEOUT_ALLOCATE_FAIL -1
#define ion_phys_addr_t phys_addr_t

/* Ion heap for LMA allocations. This heap is identical to CARVEOUT except
 * that it does not do any CPU cache maintenance nor does it zero the memory
 * using the CPU.
 */

struct ion_lma_heap {
	struct ion_heap heap;
	ion_phys_addr_t base;
	bool allow_cpu_map;
	ion_phys_addr_t vram_base;
};

void ion_heap_private_init(struct ion_device *idev, struct device *dev)
{
	struct ion_heap *heap;
	struct drm_device *ddev = dev_get_drvdata(dev);

	plist_for_each_entry(heap, &idev->heaps, node) {
		heap->priv = ddev;
	}
}

static ion_phys_addr_t ion_lma_allocate(struct ion_heap *heap,
					unsigned long size,
					void **handle)
{
	struct drm_device *ddev = heap->priv;
	dma_addr_t offset;
	int err, segmentid;
	int heap_name_len = strlen(heap->name);

	if (!strncmp(heap->name, ION_VPU_HEAP_NAME, heap_name_len - 1)) {
		segmentid = heap->name[heap_name_len - 1] - '0';
		err = mtgpu_vram_alloc(ddev, segmentid, size, &offset, handle);
	} else {
		err = mtgpu_vram_alloc(ddev, MTGPU_SEGMENT_ID_DEFAULT, size, &offset,
				       handle);
	}

	if (err) {
		pr_err("mtgpu_vram_alloc() failed\n");
		return ION_CARVEOUT_ALLOCATE_FAIL;
	}

	return (ion_phys_addr_t)offset;
}

static int ion_lma_heap_allocate(struct ion_heap *heap,
				 struct ion_buffer *buffer,
				 unsigned long size,
				 unsigned long flags)
{
	struct sg_table *table;
	ion_phys_addr_t paddr;
	int ret;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret)
		goto err_free;

	paddr = ion_lma_allocate(heap, size, &buffer->priv);
	if (paddr == ION_CARVEOUT_ALLOCATE_FAIL) {
		ret = -ENOMEM;
		goto err_free_table;
	}

	sg_dma_address(table->sgl) = paddr;
	sg_dma_len(table->sgl) = size;
	buffer->sg_table = table;

	return 0;

err_free_table:
	sg_free_table(table);
err_free:
	kfree(table);
	return ret;
}

static void ion_lma_heap_free(struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;

	/* Do not zero the LMA heap from the CPU. This is very slow with
	 * the current TCF (w/ no DMA engine). We will use the TLA to clear
	 * the memory with Rogue in another place.
	 *
	 * We also skip the CPU cache maintenance for the heap space, as we
	 * statically know that the TCF PCI memory bar has UC/WC set by the
	 * MTRR/PAT subsystem.
	 */

	if (buffer->priv)
		mtgpu_vram_free(buffer->priv);

	sg_free_table(table);
	kfree(table);
}

static int ion_lma_heap_map_user(struct ion_heap *mapper,
				 struct ion_buffer *buffer,
				 struct vm_area_struct *vma)
{
	struct sg_table *table = buffer->sg_table;
	ion_phys_addr_t paddr = sg_dma_address(table->sgl);
	struct ion_lma_heap *lma_heap = container_of(mapper, struct ion_lma_heap, heap);

	if (!lma_heap->allow_cpu_map) {
		pr_err("Trying to map_user fake secure ION handle\n");
		return -EPERM;
	}

	/* add the base addr of vram to get the real host cpu physical address */
	paddr += lma_heap->vram_base;

	return remap_pfn_range(vma, vma->vm_start,
			       PFN_DOWN(paddr) + vma->vm_pgoff,
			       vma->vm_end - vma->vm_start,
			       pgprot_writecombine(vma->vm_page_prot));
}

static void *ion_lma_heap_map_kernel(struct ion_heap *heap, struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	ion_phys_addr_t paddr = sg_dma_address(table->sgl);
	struct ion_lma_heap *lma_heap = container_of(heap, struct ion_lma_heap, heap);

	/* add the base addr of vram to get the real host cpu physical address */
	paddr += lma_heap->vram_base;

	if (!lma_heap->allow_cpu_map) {
		pr_err("Trying to map_kernel fake secure ION handle\n");
		return ERR_PTR(-EPERM);
	}

	return ioremap_wc(paddr, buffer->size);
}

static void ion_lma_heap_unmap_kernel(struct ion_heap *heap, struct ion_buffer *buffer)
{
	iounmap(buffer->vaddr);
}

static int ion_lma_dma_buf_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct ion_buffer *buffer = dmabuf->priv;
	int err;

	mutex_lock(&buffer->lock);
	if (!(buffer->flags & ION_FLAG_CACHED))
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	err = ion_lma_heap_map_user(buffer->heap, buffer, vma);
	if (err)
		pr_err("%s: Failed to map buffer to userspace\n", __func__);
	mutex_unlock(&buffer->lock);

	return err;
}

static int ion_lma_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					    enum dma_data_direction direction)
{
	struct ion_buffer *buffer = dmabuf->priv;
	struct sg_table *table;
	void *vaddr;
	int err;

	table = buffer->sg_table;

	mutex_lock(&buffer->lock);
	if (buffer->kmap_cnt) {
		buffer->kmap_cnt++;
		err = 0;
		goto unlock;
	}

	vaddr = ion_lma_heap_map_kernel(buffer->heap, buffer);
	if (IS_ERR(vaddr)) {
		err = PTR_ERR(vaddr);
		pr_err("%s: Failed to map buffer to kernel space\n", __func__);
		goto unlock;
	}

	buffer->vaddr = vaddr;
	buffer->kmap_cnt++;

	err = 0;
	
unlock:
	mutex_unlock(&buffer->lock);
	return err;
}

static int ion_lma_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
					  enum dma_data_direction direction)
{
	struct ion_buffer *buffer = dmabuf->priv;
	struct sg_table *table;

	table = buffer->sg_table;

	mutex_lock(&buffer->lock);
	buffer->kmap_cnt--;

	if (!buffer->kmap_cnt) {
		ion_lma_heap_unmap_kernel(buffer->heap, buffer);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static struct sg_table* ion_lma_map_dma_buf(struct dma_buf_attachment *attach,
					    enum dma_data_direction dir)
{
	struct ion_buffer *buffer = attach->dmabuf->priv;

	return buffer->sg_table;
}

/*
 * We leave these empty callback functions to avoid falling back to ion
 * default implementations.
 */
static void ion_lma_unmap_dma_buf(struct dma_buf_attachment *attach, 
				  struct sg_table *sgt, 
				  enum dma_data_direction dir)
{
}

static int ion_lma_dma_buf_attach(struct dma_buf *dma_buf, struct dma_buf_attachment *attach)
{
	return 0;
}

static void ion_lma_dma_buf_detach(struct dma_buf *dma_buf, struct dma_buf_attachment *attach)
{
}

static struct ion_heap_ops lma_heap_ops = {
	.allocate = ion_lma_heap_allocate,
	.free = ion_lma_heap_free,
};

static struct dma_buf_ops lma_heap_dma_buf_ops = {
	.mmap               = ion_lma_dma_buf_mmap,
	.begin_cpu_access   = ion_lma_dma_buf_begin_cpu_access,
	.end_cpu_access     = ion_lma_dma_buf_end_cpu_access,
	.map_dma_buf        = ion_lma_map_dma_buf,
	.unmap_dma_buf      = ion_lma_unmap_dma_buf,
	.attach             = ion_lma_dma_buf_attach,
	.detach             = ion_lma_dma_buf_detach,	
};

struct ion_heap *ion_lma_heap_create(struct ion_platform_heap *heap_data,
				     bool allow_cpu_map)
{
	struct ion_lma_heap *lma_heap;

	/* Do not zero the LMA heap from the CPU. This is very slow with
	 * the current TCF (w/ no DMA engine). We will use the TLA to clear
	 * the memory with Rogue in another place.
	 *
	 * We also skip the CPU cache maintenance for the heap space, as we
	 * statically know that the TCF PCI memory bar has UC/WC set by the
	 * MTRR/PAT subsystem.
	 */

	lma_heap = kzalloc(sizeof(*lma_heap), GFP_KERNEL);
	if (!lma_heap)
		return ERR_PTR(-ENOMEM);

	/* Manage the heap in device local physical address space. This is so
	 * the GPU/PDP gets the local view of the memory. Host access will be
	 * adjusted by adding the base addr of vram again.
	 */
	lma_heap->vram_base = heap_data->vram_base;

	lma_heap->heap.id = heap_data->id;
	lma_heap->heap.ops = &lma_heap_ops;
	lma_heap->heap.name = heap_data->name;
	lma_heap->heap.type = ION_HEAP_TYPE_CUSTOM;
	lma_heap->heap.flags = ION_HEAP_FLAG_DEFER_FREE;

	lma_heap->allow_cpu_map = allow_cpu_map;
	lma_heap->heap.buf_ops = lma_heap_dma_buf_ops;

	return &lma_heap->heap;
}

void ion_lma_heap_destroy(struct ion_heap *heap)
{
	struct ion_lma_heap *lma_heap = container_of(heap, struct ion_lma_heap, heap);

	ion_device_remove_heap(heap);
	kfree(lma_heap);
	lma_heap = NULL;
}
