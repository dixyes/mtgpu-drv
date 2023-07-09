/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_DRV_COMMON_H__
#define __MTGPU_DRV_COMMON_H__

struct pci_device_id;

extern struct device_attribute dev_attr_mtgpu_info;

bool mtgpu_pstate_is_enabled(void);
bool mtgpu_sriov_enabled(struct pci_dev *pdev);
bool mtgpu_display_is_dummy(void);
bool mtgpu_display_is_none(void);
bool mtgpu_card_support_display(struct mtgpu_device *mtdev);
bool mtgpu_card_is_server(struct pci_dev *pdev);
bool mtgpu_device_restore_pci_state(struct pci_dev *pdev);
bool mtgpu_mpc_is_supported(struct mtgpu_device *mtdev);
void mtgpu_mpc_dump(struct mtgpu_device *mtdev);
int mtgpu_get_driver_mode(void);
int mtgpu_pm_resume(struct device *dev);
int mtgpu_pm_resume_early(struct device *dev);
int mtgpu_pm_suspend(struct device *dev);
int mtgpu_probe(struct pci_dev *pdev, const struct pci_device_id *id);
int mtgpu_kick_out_firmware_fb(struct pci_dev *pdev);

void mtgpu_remove(struct pci_dev *pdev);

#endif /*__MTGPU_DRV_COMMON_H__*/
