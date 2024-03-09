/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/tty_driver.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/console.h>
#include <linux/platform_device.h>

#include "mtgpu_ipc_tty.h"

#define IPC_TTY_COUNT	4

struct mtgpu_ipc_tty {
	struct tty_port port;
	struct tty_driver *tty;
	struct console console;
	struct device *dev;
};

static int ipc_tty_open(struct tty_struct *tty, struct file *filp)
{
	struct mtgpu_ipc_tty *ipc_tty = tty->driver_data;

	return tty_port_open(&ipc_tty->port, tty, filp);
}

static void ipc_tty_close(struct tty_struct *tty, struct file *filp)
{
	struct mtgpu_ipc_tty *ipc_tty = tty->driver_data;

	tty_port_close(&ipc_tty->port, tty, filp);
}

static void ipc_tty_hangup(struct tty_struct *tty)
{
	struct mtgpu_ipc_tty *ipc_tty = tty->driver_data;

	tty_port_hangup(&ipc_tty->port);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
static ssize_t ipc_tty_write(struct tty_struct *tty, const u8 *buf, size_t count)
#else
static int ipc_tty_write(struct tty_struct *tty, const u8 *buf, int count)
#endif
{
	struct mtgpu_ipc_tty *ipc_tty = tty->driver_data;

	return mtgpu_fec_tty_do_write(ipc_tty->dev, buf, (int) count, tty->index);
}

#if defined(OS_TTY_OPERATIONS_USE_INT_WRITE_ROOM)
static int ipc_tty_write_room(struct tty_struct *tty)
#else
static unsigned int ipc_tty_write_room(struct tty_struct *tty)
#endif
{
	return TTY_IPC_MSG_MAX_DATA_SIZE;
}

#if defined(OS_TTY_OPERATIONS_USE_INT_CHARS_IN_BUFFER)
static int ipc_tty_chars_in_buffer(struct tty_struct *tty)
#else
static unsigned int ipc_tty_chars_in_buffer(struct tty_struct *tty)
#endif
{
	return 0;
}

static int ipc_tty_install(struct tty_driver *driver, struct tty_struct *tty)
{
	struct mtgpu_ipc_tty *ipc_tty = driver->driver_state;

	tty->driver_data = &ipc_tty[tty->index];

	return tty_port_install(&ipc_tty[tty->index].port, driver, tty);
}

static const struct tty_operations ipc_tty_ops = {
	.install = ipc_tty_install,
	.open = ipc_tty_open,
	.close = ipc_tty_close,
	.hangup = ipc_tty_hangup,
	.write = ipc_tty_write,
	.write_room = ipc_tty_write_room,
	.chars_in_buffer = ipc_tty_chars_in_buffer,
};

void mtgpu_ipc_tty_handler(struct mtgpu_ipc_tty *ipctty, void *data)
{
	struct mtgpu_ipc_tty_data *d = data;
	struct mtgpu_ipc_tty *ipc_tty = ipctty;
	int cnt;

	if (!ipc_tty)
		return;

	if (d->dsize <= 0 || d->dsize > TTY_IPC_MSG_MAX_DATA_SIZE) {
		dev_err(ipctty->dev, "%s err size 0x%x\n", __func__, d->dsize);
		return;
	}

	/* find my ipc tty port */
	ipc_tty += d->tty_idx;

	cnt = tty_insert_flip_string(&ipc_tty->port, d->data, d->dsize);
	if (cnt < d->dsize)
		dev_err(ipctty->dev, "data dropped %d bytes\n", d->dsize - cnt);

	tty_flip_buffer_push(&ipc_tty->port);

	/* Filter extra tty format info and output. */
	dev_info(ipctty->dev, "[FEC] %.*s", d->dsize, d->data);
}

static void ipc_tty_console_write(struct console *co, const char *b,
				  unsigned int count)
{
	struct mtgpu_ipc_tty *ipc_tty = co->data;

	mtgpu_fec_tty_do_write(ipc_tty->dev, b, count, ipc_tty->console.index);
}

static struct tty_driver *ipc_tty_console_device(struct console *c,
						 int *index)
{
	struct mtgpu_ipc_tty *ipc_tty = c->data;

	*index = ipc_tty->console.index;
	return ipc_tty->tty;
}

static int ipc_tty_console_setup(struct console *co, char *options)
{
	return 0;
}

static int ipc_tty_activate(struct tty_port *port, struct tty_struct *tty)
{
	return 0;
}

static void ipc_tty_shutdown(struct tty_port *port)
{
}

static const struct tty_port_operations ipc_tty_port_ops = {
	.activate = ipc_tty_activate,
	.shutdown = ipc_tty_shutdown
};

int mtgpu_ipc_tty_create(struct device *dev, struct mtgpu_ipc_tty **ipc_tty_out)
{
	int ret, tty_idx;
	struct tty_driver *tty;
	struct mtgpu_ipc_tty *ipc_tty_p, *ipc_tty;
	struct device *ttydev;
	char tty_name[32] = {0};

	ipc_tty = kcalloc(IPC_TTY_COUNT, sizeof(*ipc_tty), GFP_KERNEL);
	if (!ipc_tty)
		return -ENOMEM;

	tty = tty_alloc_driver(IPC_TTY_COUNT,
			       TTY_DRIVER_RESET_TERMIOS | TTY_DRIVER_REAL_RAW |
			       TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(tty))
		return PTR_ERR(tty);

	tty->driver_name = "mtgpu_ipc_tty";
	sprintf(tty_name, "ipctty.%s.", dev_name(dev));
	tty->name = tty_name;
	tty->type = TTY_DRIVER_TYPE_SERIAL;
	tty->subtype = SERIAL_TYPE_NORMAL;
	tty->init_termios = tty_std_termios;
	tty->driver_state = ipc_tty;
	tty_set_operations(tty, &ipc_tty_ops);
	ret = tty_register_driver(tty);
	if (ret) {
		dev_err(dev, "failed to register tty driver\n");
		goto err_tty_register_driver;
	}

	for (tty_idx = 0; tty_idx < IPC_TTY_COUNT; tty_idx++) {
		ipc_tty_p = &ipc_tty[tty_idx];
		ipc_tty_p->tty = tty;
		tty_port_init(&ipc_tty_p->port);
		ipc_tty_p->port.ops = &ipc_tty_port_ops;
		ttydev = tty_port_register_device(&ipc_tty_p->port, tty,
						  tty_idx, dev);
		if (IS_ERR(ttydev)) {
			dev_err(dev, "fec tty port[%d] register failed.\n", tty_idx);
			/* Also unregister earlier successful ports */
			for (tty_idx--; tty_idx >= 0; tty_idx--) {
				unregister_console(&ipc_tty[tty_idx].console);
				tty_unregister_device(tty, tty_idx);
				dev_err(dev, "fec tty port[%d] unregistered\n", tty_idx);
			}
			ret = PTR_ERR(ttydev);
			goto err_tty_register_driver;
		}

		strcpy(ipc_tty_p->console.name, "ttyMTIPC");

		ipc_tty_p->console.write = ipc_tty_console_write;
		ipc_tty_p->console.device = ipc_tty_console_device;
		ipc_tty_p->console.setup = ipc_tty_console_setup;
		ipc_tty_p->console.flags = CON_PRINTBUFFER;
		ipc_tty_p->console.index = tty_idx;
		ipc_tty_p->console.data = ipc_tty_p;
		ipc_tty_p->dev = dev;
		register_console(&ipc_tty_p->console);
	}

	*ipc_tty_out = ipc_tty;

	return 0;

err_tty_register_driver:
	tty_unregister_driver(tty);
	tty_driver_kref_put(tty);
	kfree(ipc_tty);
	*ipc_tty_out = NULL;

	return ret;
}

void mtgpu_ipc_tty_destroy(struct device *dev, struct mtgpu_ipc_tty *ipc_tty)
{
	int tty_idx;
	struct tty_driver *tty;

	if (!ipc_tty)
		return;

	tty = ipc_tty->tty;
	if (!tty)
		return;

	for (tty_idx = 0; tty_idx < IPC_TTY_COUNT; tty_idx++) {
		unregister_console(&ipc_tty[tty_idx].console);
		tty_unregister_device(tty, ipc_tty[tty_idx].console.index);
	}

	tty_unregister_driver(tty);
	tty_driver_kref_put(tty);
	kfree(ipc_tty);
}
