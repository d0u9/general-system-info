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

static void get_if_addrs(struct net *net)
{
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) < 0) {
		printl_err("hello world\n");
		return;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_PACKET &&
		    ifa->ifa_data != NULL) {
			unsigned int index = if_nametoindex(ifa->ifa_name);
			struct rtnl_link_stats *stats = ifa->ifa_data;
			struct dev_desc *dev = NULL;

			if (!test_bit(index, net->dev_btmp))  {
				dev = calloc(1, sizeof(struct dev_desc));
				INIT_LIST_HEAD(&dev->devs);
				INIT_LIST_HEAD(&dev->online_devs);
				list_add_tail(&dev->devs, &net->devs);
			} else {
				dev = find_dev_by_index(&net->devs, index);
			}

			strncpy(dev->name, ifa->ifa_name, IFNAMSIZ);
			dev->index = index;
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
	}
}

struct net *net_init(void)
{
	struct net *ret = calloc(1, sizeof(struct net));
	INIT_LIST_HEAD(&ret->devs);
	INIT_LIST_HEAD(&ret->online_devs);

	return ret;
}

void net_exit(struct net *net)
{
	free(net);
}

void net_update(struct net *net)
{
	get_if_addrs(net);
}

