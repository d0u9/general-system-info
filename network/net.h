#ifndef _TRI_NET_H
#define _TRI_NET_H

#include <arpa/inet.h>
#include <net/if.h>
#include <trilib/core.h>
#include <trilib/list.h>
#include <trilib/bitmap.h>

#define NET_DEVICE_NUM_MAX	64
#define IFF_LOWER_UP		(1<<16)
#define PATH_NAME_MAX		256

enum if_type {
	PCI,
	USB,
};

struct addr_desc {
	int family;
	struct sockaddr *addr;
	struct sockaddr *netmask;
	union {
		/* Broadcast address of interface */
		struct sockaddr *broadaddr;
		/* Point-to-point destination address */
		struct sockaddr *dstaddr;
	};
	struct list_head addrs;
	struct list_head all_addrs;
};

struct dev_desc {
	char name[IFNAMSIZ];
	unsigned int index;
	bool online;
	/* more about flags, see SIOCGIFFLAGS in netdevice(7) */
	unsigned int flags;
	u64 collions;
	u64 tx_errors;
	u64 rx_errors;
	u64 tx_dropped;
	u64 rx_dropped;
	u64 tx_bytes;
	u64 rx_bytes;
	u64 tx_packages;
	u64 rx_packages;
	u32 mtu;
	unsigned char hwaddr[6];
	enum if_type if_type;
	u16 vendor;
	u16 device;
	u32 class;
	struct list_head addrs;
	struct list_head devs;
	struct list_head online_devs;
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
	struct list_head devs;
	struct list_head online_devs;
	unsigned long dev_btmp[BITS_TO_LONGS(NET_DEVICE_NUM_MAX)];
	unsigned long online_btmp[BITS_TO_LONGS(NET_DEVICE_NUM_MAX)];
	struct list_head all_addrs;
};

extern struct net *net_init(void);
extern void net_exit(struct net *net);
extern void net_update(struct net *net);

#endif /* _TRI_NET_H */
