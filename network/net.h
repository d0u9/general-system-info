#ifndef _TRI_NET_H
#define _TRI_NET_H

#include <arpa/inet.h>
#include <trilib/core.h>
#include <trilib/list.h>

struct addr_desc {
	struct in_addr addr4;
	struct in6_addr addr6;
	struct list_head addr;
};

struct dev_desc {
	u64 mac;
	bool online;
	u32 mtu;
	u16 vendor;
	u16 device;
	struct addr_desc addrs;
	struct list_head list;
	struct list_head online_list;
};

struct net {
	u32	total_dev_num;
	u32	online_dev_num;
	u64	total_tx_bytes;
	u64	total_rx_bytes;
	u64	total_tx_packages;
	u64	total_rx_packages;
	struct list_head devs;
	struct list_head online_devs;
};

#endif /* _TRI_NET_H */
