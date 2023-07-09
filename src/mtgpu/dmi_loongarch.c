/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#include "dmi_loongarch.h"

#include <linux/efi.h>
#include <linux/slab.h>

#define dmi_early_remap early_ioremap
#define dmi_early_unmap early_iounmap
#define dmi_alloc(l)    alloc_bootmem(l)

void __init __iomem *dmi_remap(u64 phys_addr, unsigned long size)
{
        return ((void *)TO_CAC(phys_addr));
}
void dmi_unmap(volatile void __iomem *addr)
{
}

