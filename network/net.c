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
#include <linux/if_link.h>
#include <trilib/log.h>
#include <net.h>

struct tmp_lists {
	struct list_head devs;
	struct list_head addrs;
};

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

static void assign_from_if_addrs(struct dev_desc *dev, struct ifaddrs *ifa)
{
	struct rtnl_link_stats *stats = ifa->ifa_data;

	dev->collions = stats->collisions;
	dev->tx_errors = stats->tx_errors;
	dev->rx_errors = stats->rx_errors;
	dev->tx_dropped = stats->tx_dropped;
	dev->rx_dropped = stats->rx_dropped;
	dev->tx_bytes = stats->tx_bytes;
	dev->rx_bytes = stats->rx_bytes;
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

static struct addr_desc *get_next_addr(struct net *net, struct list_head *tmp_addrs)
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

static void get_if_addrs(struct net *net, struct tmp_lists *tmp)
{
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) < 0) {
		printl_err("hello world\n");
		return;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		struct dev_desc *dev = get_next_dev(net, ifa->ifa_name, &tmp->devs);

		if (ifa->ifa_addr->sa_family == AF_PACKET &&
		    ifa->ifa_data != NULL) {
			assign_from_if_addrs(dev, ifa);
		} else if (ifa->ifa_addr->sa_family == AF_INET6) {
			/*
			 * IPv4 addr is obtained via netdevice. man netdevice
			 * for more.
			 */
			LIST_HEAD(tmp_head);
			struct addr_desc *addr = get_next_addr(net, &tmp->addrs);
			addr->family = AF_INET6;
			addr->addr6 =
			      ((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			list_add_tail(&addr->all_addrs, &net->all_addrs);
			list_add_tail(&addr->addrs, &dev->addrs);
		}
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

void net_exit(struct net *net)
{
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
	list_cut_position(&tmp.devs, &net->devs, (&net->devs)->prev);
	list_cut_position(&tmp.addrs, &net->all_addrs, (&net->all_addrs)->prev);
	bitmap_zero(net->dev_btmp, BITS_TO_LONGS(NET_DEVICE_NUM_MAX));

	get_if_addrs(net, &tmp);
}

