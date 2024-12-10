/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_OB_RES_H_
#define _MTGPU_OB_RES_H_

#include "mtgpu.h"

/* 20 represents 1M Byte, 1 << 20 converted to byte unit */
#define REBAR_CAP_SIZE_TO_BYTE(size) (1ULL << ((size) + 20))

struct pci_dev;

struct mtgpu_pci_region {
	struct mtgpu_region region;
	struct pci_dev *pdev;
};

struct mtgpu_ob_resource {
	int sys_res_nr;
	int pci_res_nr;
	struct mtgpu_region *sys_res;
	struct mtgpu_pci_region *pci_res;
};

extern struct mtgpu_ob_resource *mtgpu_global_ob_res;

int mtgpu_ob_res_init(void);
void mtgpu_ob_res_deinit(void);

struct mtgpu_ob_resource *mtgpu_ob_res_dup(struct mtgpu_ob_resource *ob_res);
void free_mtgpu_ob_res(struct mtgpu_ob_resource *ob_res);
int mtgpu_ob_res_remove_pci_res(struct mtgpu_ob_resource *ob_res, struct pci_dev *pdev);
int mtgpu_split_ob_res_range(struct mtgpu_ob_resource *ob_res, u64 split_point);
u32 mtgpu_pci_rebar_get_possible_sizes(struct pci_dev *pdev, int bar);

#endif /* _MTGPU_OB_RES_H_ */
