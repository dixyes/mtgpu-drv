// SPDX-License-Identifier: GPL-2.0
/*
 * PCI Peer 2 Peer DMA support.
 *
 * Copyright (c) 2016-2018, Logan Gunthorpe
 * Copyright (c) 2016-2017, Microsemi Corporation
 * Copyright (c) 2017, Christoph Hellwig
 * Copyright (c) 2018, Eideticom Inc.
 */

#define pr_fmt(fmt) "pci-p2pdma: " fmt
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/genalloc.h>
#include <linux/memremap.h>
#include <linux/pci.h>
#include <linux/percpu-refcount.h>
#include <linux/random.h>
#include <linux/seq_buf.h>
#include <linux/xarray.h>
#include <linux/pci.h>
#include "mtgpu-pci-p2pdma.h"
#include "mtgpu_p2p.h"

enum pci_p2pdma_map_type {
	PCI_P2PDMA_MAP_UNKNOWN = 0,
	PCI_P2PDMA_MAP_NOT_SUPPORTED,
	PCI_P2PDMA_MAP_BUS_ADDR,
	PCI_P2PDMA_MAP_THRU_HOST_BRIDGE,
};

static struct pci_bus *find_pci_root_bus(struct pci_bus *bus)
{
	while (bus->parent)
		bus = bus->parent;

	return bus;
}

struct pci_host_bridge *pci_find_host_bridge(struct pci_bus *bus)
{
	struct pci_bus *root_bus = find_pci_root_bus(bus);

	return to_pci_host_bridge(root_bus->bridge);
}

/*
 * Note this function returns the parent PCI device with a
 * reference taken. It is the caller's responsibility to drop
 * the reference.
 */
static struct pci_dev *find_parent_pci_dev(struct device *dev)
{
	struct device *parent;

	dev = get_device(dev);

	while (dev) {
		if (dev_is_pci(dev))
			return to_pci_dev(dev);

		parent = get_device(dev->parent);
		put_device(dev);
		dev = parent;
	}

	return NULL;
}

/*
 * Check if a PCI bridge has its ACS redirection bits set to redirect P2P
 * TLPs upstream via ACS. Returns 1 if the packets will be redirected
 * upstream, 0 otherwise.
 */
static int pci_bridge_has_acs_redir(struct pci_dev *pdev)
{
	int pos;
	u16 ctrl;

	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_ACS);
	if (!pos)
		return 0;

	pci_read_config_word(pdev, pos + PCI_ACS_CTRL, &ctrl);

	if (ctrl & (PCI_ACS_RR | PCI_ACS_CR | PCI_ACS_EC))
		return 1;

	return 0;
}

static bool cpu_supports_p2pdma(void)
{
#ifdef CONFIG_X86
	struct cpuinfo_x86 *c = &cpu_data(0);

	/* Any AMD CPU whose family ID is Zen or newer supports p2pdma */
	if (c->x86_vendor == X86_VENDOR_AMD && c->x86 >= 0x17)
		return true;
#endif

	return false;
}

static const struct pci_p2pdma_whitelist_entry {
	unsigned short vendor;
	unsigned short device;
	enum {
		REQ_SAME_HOST_BRIDGE	= 1 << 0,
	} flags;
} pci_p2pdma_whitelist[] = {
	/* Intel Xeon E5/Core i7 */
	{PCI_VENDOR_ID_INTEL,	0x3c00, REQ_SAME_HOST_BRIDGE},
	{PCI_VENDOR_ID_INTEL,	0x3c01, REQ_SAME_HOST_BRIDGE},
	/* Intel Xeon E7 v3/Xeon E5 v3/Core i7 */
	{PCI_VENDOR_ID_INTEL,	0x2f00, REQ_SAME_HOST_BRIDGE},
	{PCI_VENDOR_ID_INTEL,	0x2f01, REQ_SAME_HOST_BRIDGE},
	/* Intel SkyLake-E */
	{PCI_VENDOR_ID_INTEL,	0x2030, 0},
	{PCI_VENDOR_ID_INTEL,	0x2031, 0},
	{PCI_VENDOR_ID_INTEL,	0x2032, 0},
	{PCI_VENDOR_ID_INTEL,	0x2033, 0},
	{PCI_VENDOR_ID_INTEL,	0x2020, 0},
	{PCI_VENDOR_ID_INTEL,	0x09a2, 0},
	{}
};

/*
 * This lookup function tries to find the PCI device corresponding to a given
 * host bridge.
 *
 * It assumes the host bridge device is the first PCI device in the
 * bus->devices list and that the devfn is 00.0. These assumptions should hold
 * for all the devices in the whitelist above.
 *
 * This function is equivalent to pci_get_slot(host->bus, 0), however it does
 * not take the pci_bus_sem lock seeing __host_bridge_whitelist() must not
 * sleep.
 *
 * For this to be safe, the caller should hold a reference to a device on the
 * bridge, which should ensure the host_bridge device will not be freed
 * or removed from the head of the devices list.
 */
static struct pci_dev *pci_host_bridge_dev(struct pci_host_bridge *host)
{
	struct pci_dev *root;

	root = list_first_entry_or_null(&host->bus->devices,
					struct pci_dev, bus_list);

	if (!root)
		return NULL;
	if (root->devfn != PCI_DEVFN(0, 0))
		return NULL;

	return root;
}

static bool __host_bridge_whitelist(struct pci_host_bridge *host,
				    bool same_host_bridge)
{
	struct pci_dev *root = pci_host_bridge_dev(host);
	const struct pci_p2pdma_whitelist_entry *entry;
	unsigned short vendor, device;

	if (!root)
		return false;

	vendor = root->vendor;
	device = root->device;

	for (entry = pci_p2pdma_whitelist; entry->vendor; entry++) {
		if (vendor != entry->vendor || device != entry->device)
			continue;
		if (entry->flags & REQ_SAME_HOST_BRIDGE && !same_host_bridge)
			return false;

		return true;
	}

	return false;
}

/*
 * If we can't find a common upstream bridge take a look at the root
 * complex and compare it to a whitelist of known good hardware.
 */
static bool host_bridge_whitelist(struct pci_dev *a, struct pci_dev *b)
{
	struct pci_host_bridge *host_a = pci_find_host_bridge(a->bus);
	struct pci_host_bridge *host_b = pci_find_host_bridge(b->bus);

	if (host_a == host_b)
		return __host_bridge_whitelist(host_a, true);

	if (__host_bridge_whitelist(host_a, false) &&
	    __host_bridge_whitelist(host_b, false))
		return true;

	return false;
}

static enum pci_p2pdma_map_type
__upstream_bridge_distance(struct pci_dev *provider, struct pci_dev *client,
		int *dist, bool *acs_redirects, struct seq_buf *acs_list)
{
	struct pci_dev *a = provider, *b = client, *bb;
	int dist_a = 0;
	int dist_b = 0;
	int acs_cnt = 0;

	if (acs_redirects)
		*acs_redirects = false;

	/*
	 * Note, we don't need to take references to devices returned by
	 * pci_upstream_bridge() seeing we hold a reference to a child
	 * device which will already hold a reference to the upstream bridge.
	 */

	while (a) {
		dist_b = 0;

		if (pci_bridge_has_acs_redir(a))
			acs_cnt++;

		bb = b;

		while (bb) {
			if (a == bb)
				goto check_b_path_acs;

			bb = pci_upstream_bridge(bb);
			dist_b++;
		}

		a = pci_upstream_bridge(a);
		dist_a++;
	}

	if (dist)
		*dist = dist_a + dist_b;

	return PCI_P2PDMA_MAP_THRU_HOST_BRIDGE;

check_b_path_acs:
	bb = b;

	while (bb) {
		if (a == bb)
			break;

		if (pci_bridge_has_acs_redir(bb))
			acs_cnt++;

		bb = pci_upstream_bridge(bb);
	}

	if (dist)
		*dist = dist_a + dist_b;

	if (acs_cnt) {
		if (acs_redirects)
			*acs_redirects = true;

		return PCI_P2PDMA_MAP_THRU_HOST_BRIDGE;
	}

	return PCI_P2PDMA_MAP_BUS_ADDR;
}

/*
 * Find the distance through the nearest common upstream bridge between
 * two PCI devices.
 *
 * If the two devices are the same device then 0 will be returned.
 *
 * If there are two virtual functions of the same device behind the same
 * bridge port then 2 will be returned (one step down to the PCIe switch,
 * then one step back to the same device).
 *
 * In the case where two devices are connected to the same PCIe switch, the
 * value 4 will be returned. This corresponds to the following PCI tree:
 *
 *     -+  Root Port
 *      \+ Switch Upstream Port
 *       +-+ Switch Downstream Port
 *       + \- Device A
 *       \-+ Switch Downstream Port
 *         \- Device B
 *
 * The distance is 4 because we traverse from Device A through the downstream
 * port of the switch, to the common upstream port, back up to the second
 * downstream port and then to Device B.
 *
 * Any two devices that cannot communicate using p2pdma will return
 * PCI_P2PDMA_MAP_NOT_SUPPORTED.
 *
 * Any two devices that have a data path that goes through the host bridge
 * will consult a whitelist. If the host bridges are on the whitelist,
 * this function will return PCI_P2PDMA_MAP_THRU_HOST_BRIDGE.
 *
 * If either bridge is not on the whitelist this function returns
 * PCI_P2PDMA_MAP_NOT_SUPPORTED.
 *
 * If a bridge which has any ACS redirection bits set is in the path,
 * acs_redirects will be set to true. In this case, a list of all infringing
 * bridge addresses will be populated in acs_list (assuming it's non-null)
 * for printk purposes.
 */
static enum pci_p2pdma_map_type
upstream_bridge_distance(struct pci_dev *provider, struct pci_dev *client,
		int *dist, bool *acs_redirects, struct seq_buf *acs_list)
{
	enum pci_p2pdma_map_type map_type;

	map_type = __upstream_bridge_distance(provider, client, dist,
					      acs_redirects, acs_list);

	if (map_type == PCI_P2PDMA_MAP_THRU_HOST_BRIDGE) {
		if (!cpu_supports_p2pdma() && !host_bridge_whitelist(provider, client))
			map_type = PCI_P2PDMA_MAP_NOT_SUPPORTED;
	}

	return map_type;
}

static enum pci_p2pdma_map_type
upstream_bridge_distance_warn(struct pci_dev *provider, struct pci_dev *client,
			      int *dist)
{
	struct seq_buf acs_list;
	bool acs_redirects;
	int ret;

	seq_buf_init(&acs_list, kmalloc(PAGE_SIZE, GFP_KERNEL), PAGE_SIZE);
	if (!acs_list.buffer)
		return -ENOMEM;

	ret = upstream_bridge_distance(provider, client, dist, &acs_redirects,
				       &acs_list);
	if (acs_redirects) {
		pci_warn(client, "ACS redirect is set between the client and provider (%s)\n",
			 pci_name(provider));
		/* Drop final semicolon */
		acs_list.buffer[acs_list.len-1] = 0;
		pci_warn(client, "to disable ACS redirect for this path, add the kernel parameter: pci=disable_acs_redir=%s\n",
			 acs_list.buffer);
	}

	if (ret == PCI_P2PDMA_MAP_NOT_SUPPORTED) {
		pci_warn(client, "cannot be used for peer-to-peer DMA as the client and provider (%s) do not share an upstream bridge or whitelisted host bridge\n",
			 pci_name(provider));
	}

	kfree(acs_list.buffer);

	return ret;
}

/**
 * mtgpu_pci_p2pdma_distance_many - Determine the cumulative distance between
 *	a p2pdma provider and the clients in use.
 * @provider: p2pdma provider to check against the client list
 * @clients: array of devices to check (NULL-terminated)
 * @num_clients: number of clients in the array
 * @verbose: if true, print warnings for devices when we return -1
 *
 * Returns -1 if any of the clients are not compatible, otherwise returns a
 * positive number where a lower number is the preferable choice. (If there's
 * one client that's the same as the provider it will return 0, which is best
 * choice).
 *
 * "compatible" means the provider and the clients are either all behind
 * the same PCI root port or the host bridges connected to each of the devices
 * are listed in the 'pci_p2pdma_whitelist'.
 */
int mtgpu_pci_p2pdma_distance_many(struct pci_dev *provider, struct device **clients,
				   int num_clients, bool verbose)
{
	bool not_supported = false;
	struct pci_dev *pci_client;
	int total_dist = 0;
	int distance;
	int i, ret;

	if (num_clients == 0)
		return -1;

	for (i = 0; i < num_clients; i++) {
#if defined(OS_GLOBAL_VARIABLE_DMA_VIRT_OPS_EXIST)
		if (IS_ENABLED(CONFIG_DMA_VIRT_OPS) &&
		    clients[i]->dma_ops == &dma_virt_ops) {
			if (verbose)
				dev_warn(clients[i],
					 "cannot be used for peer-to-peer DMA because the driver makes use of dma_virt_ops\n");
			return -1;
		}
#endif

		pci_client = find_parent_pci_dev(clients[i]);
		if (!pci_client) {
			if (verbose)
				dev_warn(clients[i],
					 "cannot be used for peer-to-peer DMA as it is not a PCI device\n");
			return -1;
		}

		if (verbose)
			ret = upstream_bridge_distance_warn(provider,
					pci_client, &distance);
		else
			ret = upstream_bridge_distance(provider, pci_client,
						       &distance, NULL, NULL);

		pci_dev_put(pci_client);

		if (ret == PCI_P2PDMA_MAP_NOT_SUPPORTED)
			not_supported = true;

		if (not_supported && !verbose)
			break;

		total_dist += distance;
	}

	if (not_supported)
		return -1;

	return total_dist;
}

EXPORT_SYMBOL(mtgpu_p2p_get_pages);
EXPORT_SYMBOL(mtgpu_p2p_put_pages);
EXPORT_SYMBOL(mtgpu_p2p_dma_map_pages);
EXPORT_SYMBOL(mtgpu_p2p_dma_unmap_pages);

