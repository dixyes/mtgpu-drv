/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_PCI_P2PDMA_H__
#define __MTGPU_PCI_P2PDMA_H__

struct device;
struct pci_dev;

int mtgpu_pci_p2pdma_distance_many(struct pci_dev *provider, struct device **clients,
				   int num_clients, bool verbose);

#endif /* __MTGPU_PCI_P2PDMA_H__ */
