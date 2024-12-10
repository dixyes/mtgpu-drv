/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef __MTGPU_MTLINK_H__
#define __MTGPU_MTLINK_H__

struct mtgpu_device;
struct mtlink_device;
struct mtlink_port;

enum mtlink_ace_status {
	MTLINK_ACE_STATUS_OK = 0,
	MTLINK_ACE_STATUS_NEED_RESET_GPU,
};

int mtlink_driver_init(void);
void mtlink_driver_exit(void);

int mtlink_device_init(struct mtgpu_device *mtdev);
void mtlink_device_exit(struct mtgpu_device *mtdev);

struct mtlink_device *mtlink_get_link_device_by_chip_id(int chip_id);
void mtlink_rebuild(struct mtlink_port *llink_port,
		    struct mtlink_port *rlink_port,
		    bool remove);

int mtlink_link_device_reset(struct mtlink_device *link_device);
void mtlink_topo_rebuild(u32 cmd);

#if !defined(NO_HARDWARE)
bool mtlink_supported_between_devices(struct device *dev1, struct device *dev2);
int mtlink_get_card_id(struct device *dev, int *card_id, u64 *per_upa_space);
int mtlink_is_enabled(void);
bool mtlink_is_init_finished(void);
int mtlink_notify_device_check_status_finish(struct device *dev, u32 result);
#else

static inline bool mtlink_supported_between_devices(struct device *dev1, struct device *dev2)
{
	return false;
}

static inline int mtlink_get_card_id(struct device *dev, int *card_id, u64 *per_upa_space)
{
	return 0;
}

static inline int mtlink_is_enabled(void)
{
	return 0;
}

static inline bool mtlink_is_init_finished(void)
{
	return true;
}

static inline int mtlink_notify_device_check_status_finish(struct device *dev, u32 result)
{
	return 0;
}

#endif

#endif /*__MTGPU_MTLINK_H__*/
