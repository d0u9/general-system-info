#ifndef _TRI_NET_H
#define _TRI_NET_H

#include <arpa/inet.h>
#include <net/if.h>
#include <trilib/core.h>
#include <trilib/list.h>
#include <trilib/bitmap.h>

#define NET_DEVICE_NUM_MAX	16

struct addr_desc {
	int family;
	struct in_addr addr4;
	struct in6_addr addr6;
	struct list_head addr;
};

struct dev_desc {
	char name[IFNAMSIZ];
	bool online;
	u64 collions;
	u64 tx_errors;
	u64 rx_errors;
	u64 tx_dropped;
	u64 rx_dropped;
	u64 tx_bytes;
	u64 rx_bytes;
	u64 tx_packages;
	u64 rx_packages;
	u64 mac;
	u32 mtu;
	u16 vendor;
	u16 device;
	struct addr_desc addrs;
};

struct net {
	u32 total_dev_num;
	u32 online_dev_num;
	u64 total_tx_bytes;
	u64 total_rx_bytes;
	u64 total_tx_dropped;
	u64 total_rx_dropped;
	u64 total_tx_packages;
	u64 total_rx_packages;
	u64 total_collions;
	struct dev_desc devs[NET_DEVICE_NUM_MAX];
	unsigned long dev_btmp[BITS_TO_LONGS(NET_DEVICE_NUM_MAX)];
	unsigned long online_btmp[BITS_TO_LONGS(NET_DEVICE_NUM_MAX)];
};

extern struct net *net_init(void);
extern void net_exit(struct net *net);
extern void net_update(struct net *net);

#endif /* _TRI_NET_H */
