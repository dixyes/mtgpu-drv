/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_IPC_TTY_
#define _MTGPU_IPC_TTY_

#include "mtgpu.h"

struct mtgpu_ipc_info;
struct mtgpu_ipc_tty;

struct mtgpu_ipc_tty_data {
	int dsize; //sizeof(data)
	u8 tty_idx;
	bool is_console;
	u8 reserved[6];
	u8 data[];
};

#define IPC_TTY_COUNT			4
#define IPC_TTY_MAX_CONSOLE_COUNT	2
#define TTY_IPC_MSG_MAX_DATA_SIZE	128

int mtgpu_ipc_tty_create(struct device *dev, struct mtgpu_ipc_tty **ipc_tty_out);
void mtgpu_ipc_tty_destroy(struct device *dev, struct mtgpu_ipc_tty *ipc_tty);
void mtgpu_ipc_tty_handler(struct mtgpu_ipc_tty *ipctty, void *data);
int mtgpu_fec_tty_do_write(struct device *dev, const u8 *buf, int count, u8 tty_idx);

#endif //_MTGPU_IPC_TTY_
