/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_OB_RES_H_
#define _MTGPU_OB_RES_H_

#include "mtgpu.h"

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

#endif /* _MTGPU_OB_RES_H_ */
