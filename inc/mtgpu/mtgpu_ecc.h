/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTGPU_ECC_H__
#define __MTGPU_ECC_H__

#include "mtgpu.h"
#include "mtgpu_ras.h"
#include "os-interface.h"
#include "lock_types.h"

#define GET_ECC_INFO_FROM_CMC(mtdev)	(DEVICE_IS_PINGHU1(mtdev))
#define MAX_MANAGE_ECC_ENTRY		512
#define ECC_FLASH_BUFFER_SIZE		(OS_VAL(PAGE_SIZE) << 2)

#define GET_REGISTER_VALUE(register, shift, bits)   (((register) >> (shift)) & ((1 << (bits)) - 1))

struct device;
struct amt_config;

enum mtgpu_ecc_error_type {
	MTGPU_ECC_ERR_TYPE_SBE   = 1,
	MTGPU_ECC_ERR_TYPE_DBE   = 2,

	MTGPU_ECC_ERR_TYPE_CLEAN = 0xFF,
};

enum mtgpu_ecc_parity_data_mem_layout {
	MTGPU_ECC_PARITY_INLINE    = 0,
	MTGPU_ECC_PARITY_SIDE_BAND = 1,
};

enum mtgpu_ecc_cmd {
	MTGPU_ECC_CMD_ENABLE_ECC		= 0,
	MTGPU_ECC_CMD_DISABLE_ECC		= 1,
	MTGPU_ECC_CMD_READ_INFO			= 2,
	MTGPU_ECC_CMD_WRITE_INFO		= 3,
	MTGPU_ECC_CMD_GET_AMT_CONFIG		= 4,
	MTGPU_ECC_CMD_GET_SWITCH_STATUS		= 5,
	MTGPU_ECC_CMD_CLEAR_INFO		= 6,
	MTGPU_ECC_CMD_CLEAR_SWITCH_STATUS	= 7,
	MTGPU_ECC_CMD_GET_AGGREGATE_ERROR_CNT	= 8,
	MTGPU_ECC_CMD_SET_AGGREGATE_ERROR_CNT	= 9,
	MTGPU_ECC_CMD_ERR_INJECT		= 10,

	MTGPU_ECC_CMD_MAX = 127,
};

enum mtgpu_ecc_flash_db {
	MTGPU_ECC_FLASH_DB_GET	= 0,
	MTGPU_ECC_FLASH_DB_SET	= 1,
	MTGPU_ECC_FLASH_DB_DEL	= 2,

	MTGPU_ECC_FLASH_DB_MAX	= 127,
};

#pragma pack(push, 1)

struct mtgpu_ecc_flash_db_msg {
	u64 data_addr;	/* pa */
	u8 op;
	u8 key_length;
	u8 data_length;
	char key[];
};

struct mtgpu_ecc_flash_db_msg_reply {
	u32 status;
	u32 reply_size;
};

#pragma pack(pop)

struct mtgpu_ecc_detail {
	u64 ecc_type		: 8;
	u64 ecc_lane_info	: 32;
	u64 reserved		: 24;
	u64 ddr_channel_index	: 8;
	u64 ecc_cell_phyoffset	: 56;
};

struct mtgpu_ecc_detail_entry {
	struct mtgpu_device *mtdev;
	struct mtgpu_ecc_detail ecc_detail;
	struct list_head entry;
};

struct mtgpu_ecc_ras {
	u64 ecc_type				: 8;
	u64 ddr_channel_index			: 8;
	u64 reserved				: 48;
	u64 vram_addr;
};

struct mtgpu_ecc_hdr {
	u32 mtgpu_ecc_data_size		: 8;
	u32 mtgpu_ecc_cmd_id		: 8;
	u32 rsv				: 16;
	union{
		u64 host_addr;
		u32 switch_status;
		struct mtgpu_ecc_detail detail;
		struct mtgpu_ecc_ras ras;
		u8 amt_config[0];
		u32 error_type		: 8;
		u32 error_cnt		: 24;
	} data;
};

struct mtgpu_ecc_sbe_info {
	u32 occur_count;
	u64 timestamp;
	u64 mtgpu_vram_addr;
	struct mtgpu_ecc_detail ecc_detail;
	struct list_head sbe_entry;
	struct list_head sbe_retired_entry;
	struct list_head sbe_pending_entry;
};

struct mtgpu_ecc_dbe_info {
	u64 timestamp;
	u64 mtgpu_vram_addr;
	struct mtgpu_ecc_detail ecc_detail;
	struct list_head dbe_retired_entry;
	struct list_head dbe_pending_entry;
};

struct mtgpu_ecc {
	u8 parity_mem_layout;
	u32 sbe_total;
	u32 dbe_total;
	u32 sbe_retired_pages_total;
	u32 dbe_retired_pages_total;
	u32 retired_pages_total;
	u32 sbe_pending_pages_total;
	u32 dbe_pending_pages_total;
	u32 pending_pages_total;
	u32 retired_pages_cnt;
	u32 error_total;
	u32 sbe_volatile_cnt;
	u32 dbe_volatile_cnt;
	u32 sbe_aggregate_cnt;
	u32 dbe_aggregate_cnt;
	/* save the value of vram addr */
	u32 *addr_hash_table;
	u64 *retired_pages_addr;
	dma_addr_t dma_addr;
	dma_addr_t ob_dma_addr;
	dma_addr_t flash_data_addr_pa;
	void *flash_data_addr_va;
	void *amt_private;

	bool hw_enabled;
	bool is_pages_pending;
	bool is_switch_pending;
	bool is_err_occurred;

	struct mtgpu_device *mtdev;
	struct list_head sbe_head;
	struct list_head ecc_entry;
	struct list_head sbe_retired_pages_head;
	struct list_head dbe_retired_pages_head;
	struct list_head sbe_pending_pages_head;
	struct list_head dbe_pending_pages_head;

	atomic_t occurred_count;
};

struct mtgpu_ecc_global_data {
	struct workqueue_struct *workqueue;
	struct work_struct *work;
	struct list_head entry_head;
	struct list_head dev_head;
	struct mutex *mutex_lock;
	bool init_state;
};

extern struct mtgpu_ecc_global_data ecc_global_data;

struct mtgpu_ecc_ops {
	int (*ddr_amt_config_get)(struct mtgpu_ecc *mtgpu_ecc);
	void (*ddr_amt_config_clear)(struct amt_config *configs);
	u64 (*convert_address)(struct mtgpu_device *mtdev,
			       struct amt_config *configs,
			       struct mtgpu_ecc_detail *detail);
	int (*mem_err_inject)(struct mtgpu_device *mtdev,
			      RAS_ERROR ras_err,
			      mtgpu_ras_param ras_param);
};

#define ECC_IPC_HEADER_SZIE	(offsetof(struct mtgpu_ecc_hdr, data))

int mtgpu_ecc_init(struct mtgpu_device *mtdev);
void mtgpu_ecc_exit(struct mtgpu_device *mtdev);
int mtgpu_ecc_global_data_init(void);
void mtgpu_ecc_global_data_deinit(void);
int mtgpu_ecc_mode(struct mtgpu_device *mtdev, u32 mode);
int mtgpu_ecc_get_switch_status(struct mtgpu_device *mtdev, u32 *ecc_switch_status);
int mtgpu_ecc_handle_aggregate_error_cnt(struct mtgpu_device *mtdev,
					 const char *key_name,
					 enum mtgpu_ecc_flash_db flash_op,
					 u32 *error_cnt);
int mtgpu_ecc_ipc_transmit(struct mtgpu_device *mtdev,
			   struct ipc_msg *ipc_msg,
			   u8 event_id,
			   u8 data_size,
			   bool choice_cmc);
void mtgpu_ecc_disable_ioctl_access(struct mtgpu_ecc *mtecc);
void mtgpu_ecc_enable_ioctl_access(struct mtgpu_ecc *mtecc);

#endif /* __MTGPU_ECC_H__ */
