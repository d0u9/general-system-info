#define _GNU_SOURCE
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/if_link.h>
#include <trilib/log.h>
#include <errno.h>
#include <net.h>
#include <utils.h>
#include <libgen.h>

struct tmp_lists {
	struct list_head devs;
	struct list_head addrs;
};

static void _get_vendor(char *dev_path, struct dev_desc *dev, enum if_type ift)
{
	char fvendor[PATH_NAME_MAX] = {0};
	char fdevice[PATH_NAME_MAX] = {0};
	char tmp[16] = {0};

	dev->if_type = ift;
	for (char *dname = dirname(dev_path);
	     strncmp(dname, "/sys/devices", PATH_NAME_MAX) != 0;
	     dname = dirname(dname)) {

		if (ift == USB) {
			snprintf(fvendor, PATH_NAME_MAX, "%s/idVendor", dname);
			snprintf(fdevice, PATH_NAME_MAX, "%s/idProduct", dname);
		} else if (ift == PCI) {
			snprintf(fvendor, PATH_NAME_MAX, "%s/vendor", dname);
			snprintf(fdevice, PATH_NAME_MAX, "%s/device", dname);
		}

		if (access(fvendor, R_OK) != -1) {
			dev->vendor = stoulx(first_line(fvendor, tmp, 16));
		}

		if (access(fdevice, R_OK) != -1) {
			dev->device = stoulx(first_line(fdevice, tmp, 16));
		}

		if (dev->vendor != 0 || dev->device != 0)
			break;
	}
	printl_debug("type: %d, vendor: %lx, product: %lx\n",
		     ift, dev->vendor, dev->device);

}

static void get_vendor(struct net *net)
{
	struct dev_desc *dev = NULL;
	char pathname[PATH_NAME_MAX] = {0};
	char rpath[PATH_NAME_MAX] = {0};

	list_for_each_entry(dev, &net->devs, devs) {
		dev->vendor = dev->device = dev->class = 0;

		snprintf(pathname, PATH_NAME_MAX,
			 "/sys/class/net/%s", dev->name);
		realpath(pathname, rpath);

		char *substr1 = NULL, *substr2 = NULL;
		if ((substr1 = strstr(rpath, "pci")) == NULL) {
			printl_debug("%s is virtual device\n", dev->name);
			continue;
		}

		if ((substr2 = strstr(substr1, "usb")) != NULL) {
			printl_debug("%s is usb device\n", dev->name);
			_get_vendor(rpath, dev, USB);
			continue;
		}

		_get_vendor(rpath, dev, PCI);
	}
}

static struct dev_desc *find_dev_by_index(struct list_head *head,
					  unsigned int index)
{
	struct dev_desc *cur = NULL;
	list_for_each_entry(cur, head, devs) {
		if (cur->index == index)
			return cur;
	}

	return NULL;
}

static void net_device(struct net *net, struct tmp_lists *tmp)
{
	struct ifreq ifr;
	struct dev_desc *dev = NULL;
	memset(&ifr, 0, sizeof(struct ifreq));

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sockfd < 0) {
		printl_err("Create socket fd: %s\n", strerror(errno));
		return;
	}

	list_for_each_entry(dev, &net->devs, devs) {
		strncpy(ifr.ifr_name, dev->name, IFNAMSIZ);

		if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
			printl_err("Get SIOCGIFNAME: %s\n", strerror(errno));
		} else {
			memcpy(dev->hwaddr, ifr.ifr_hwaddr.sa_data, sizeof(u64));
			printl_debug("%-8s hwaddr: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->name,
				     dev->hwaddr[0], dev->hwaddr[1], dev->hwaddr[2],
				     dev->hwaddr[3], dev->hwaddr[4], dev->hwaddr[5]);
		}

		if (ioctl(sockfd, SIOCGIFMTU, &ifr) < 0) {
			printl_err("Get SIOCGIFMTU: %s\n", strerror(errno));
		} else {
			dev->mtu = (u32)ifr.ifr_mtu;
			printl_debug("%-8s mtu: %lu\n", dev->name, dev->mtu);
		}

		if (dev->flags & IFF_LOWER_UP) {
			dev->online = TRUE;
			net->online_dev_num ++;
			list_add_tail(&dev->online_devs, &net->online_devs);
			printl_debug("%-8s lower_up: true\n");
		} else {
			dev->online = FALSE;
			printl_debug("%-8s lower_up: false\n");
		}

	}
}

static void assign_dev_data(struct dev_desc *dev, struct ifaddrs *ifa)
{
	struct rtnl_link_stats *stats = ifa->ifa_data;

	dev->flags = ifa->ifa_flags;
	dev->collions = stats->collisions;
	dev->tx_errors = stats->tx_errors;
	dev->rx_errors = stats->rx_errors;
	dev->tx_dropped = stats->tx_dropped;
	dev->rx_dropped = stats->rx_dropped;
	dev->tx_bytes = stats->tx_bytes;
	printl_debug("%-8s tx bytes: %lu\n", dev->name, dev->tx_bytes);
	dev->rx_bytes = stats->rx_bytes;
	printl_debug("%-8s rx bytes: %lu\n", dev->name, dev->rx_bytes);
	dev->tx_packages = stats->tx_packets;
	dev->rx_packages = stats->rx_packets;
}

static struct dev_desc *get_next_dev(struct net *net, const char *ifa_name,
				     struct list_head *tmp_devs)
{
	unsigned int index = if_nametoindex(ifa_name);
	struct dev_desc *dev = NULL;

	if (!test_bit(index, net->dev_btmp))  {
		if (list_empty(tmp_devs)) {
			dev = calloc(1, sizeof(struct dev_desc));
			net->total_dev_num++;
		} else {
			dev = list_entry(tmp_devs->next,
					 struct dev_desc, devs);
			list_del(&dev->devs);
		}
		INIT_LIST_HEAD(&dev->devs);
		INIT_LIST_HEAD(&dev->online_devs);
		INIT_LIST_HEAD(&dev->addrs);
		list_add_tail(&dev->devs, &net->devs);
		set_bit(index, net->dev_btmp);
		strncpy(dev->name, ifa_name, IFNAMSIZ);
		dev->index = index;
	} else {
		dev = find_dev_by_index(&net->devs, index);
	}
	return dev;
}

static struct addr_desc *get_next_addr(struct list_head *tmp_addrs)
{
	struct addr_desc *addr = NULL;
	if (list_empty(tmp_addrs)) {
		addr = calloc(1, sizeof(struct addr_desc));
	} else {
		addr = list_entry(tmp_addrs->next, struct addr_desc, all_addrs);
		list_del(&addr->all_addrs);
	}

	INIT_LIST_HEAD(&addr->all_addrs);
	INIT_LIST_HEAD(&addr->addrs);

	return addr;
}

static void assign_addr(int family, struct addr_desc *addr, struct ifaddrs *ifa)
{
	addr->family = family;
	addr->addr = ifa->ifa_addr;
	addr->netmask = ifa->ifa_netmask;
	if (ifa->ifa_flags & IFF_BROADCAST) {
		addr->broadaddr = ifa->ifa_ifu.ifu_broadaddr;
	} else {
		addr->dstaddr = ifa->ifa_ifu.ifu_dstaddr;
	}
}

static void update_net_total(struct net *net, struct dev_desc *dev)
{
	net->total_tx_bytes += dev->tx_bytes;
	net->total_rx_bytes += dev->rx_bytes;
	net->total_tx_dropped += dev->tx_dropped;
	net->total_rx_dropped += dev->rx_dropped;
	net->total_tx_packages += dev->tx_packages;
	net->total_rx_packages += dev->rx_packages;
	net->total_collions += dev->collions;
}

static void get_if_addrs(struct net *net, struct tmp_lists *tmp)
{
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) < 0) {
		printl_err("hello world\n");
		return;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		int family = ifa->ifa_addr->sa_family;

		struct dev_desc *dev = get_next_dev(net, ifa->ifa_name, &tmp->devs);

		if (family == AF_PACKET && ifa->ifa_data != NULL) {
			assign_dev_data(dev, ifa);
			update_net_total(net, dev);
		} else if (family == AF_INET6 || family == AF_INET) {
			struct addr_desc *addr = get_next_addr(&tmp->addrs);
			assign_addr(family, addr, ifa);

			list_add_tail(&addr->all_addrs, &net->all_addrs);
			list_add_tail(&addr->addrs, &dev->addrs);
		}
	}

	struct dev_desc *dev = NULL, *tmp_dev = NULL;
	list_for_each_entry_safe(dev, tmp_dev, &tmp->devs, devs) {
		list_del(&dev->devs);
		free(dev);
	}

	struct addr_desc *addr = NULL, *tmp_addr = NULL;
	list_for_each_entry_safe(addr, tmp_addr, &tmp->addrs, all_addrs) {
		list_del(&addr->all_addrs);
		free(addr);
	}

	freeifaddrs(ifaddr);
}

struct net *net_init(void)
{
	struct net *ret = calloc(1, sizeof(struct net));
	INIT_LIST_HEAD(&ret->devs);
	INIT_LIST_HEAD(&ret->all_addrs);
	INIT_LIST_HEAD(&ret->online_devs);

	return ret;
}

static void free_addrs(struct net *net)
{
	struct addr_desc *cur = NULL, *tmp = NULL;
	list_for_each_entry_safe(cur, tmp, &net->all_addrs, all_addrs) {
		free(cur);
	}
}

static void free_devs(struct net *net)
{
	struct dev_desc *cur = NULL, *tmp = NULL;
	list_for_each_entry_safe(cur, tmp, &net->devs, devs) {
		free(cur);
	}
}

void net_exit(struct net *net)
{
	free_addrs(net);
	free_devs(net);
	free(net);
}

void net_update(struct net *net)
{
	/*
	 * In order to reduce the overhead of re-allocing memory, here we reuse
	 * the memory that we have already alloced before. we temporialy move
	 * all the devs to a new list, and pick them from there, assign
	 * values to them, finally add them to the proper list.
	 */
	struct tmp_lists tmp;
	INIT_LIST_HEAD(&tmp.devs);
	INIT_LIST_HEAD(&tmp.addrs);
	list_splice_init(&net->devs, &tmp.devs);
	list_splice_init(&net->all_addrs, &tmp.addrs);
	INIT_LIST_HEAD(&net->online_devs);
	bitmap_zero(net->dev_btmp, BITS_TO_LONGS(NET_DEVICE_NUM_MAX));
	memset(net, 0, offsetof(struct net, devs));

	get_if_addrs(net, &tmp);
	net_device(net, &tmp);
	get_vendor(net);
}

