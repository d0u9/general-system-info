#ifndef _TRI_CPU_H
#define _TRI_CPU_H

#include <linux/version.h>
#include "../trilib/core.h"
#include "../trilib/list.h"

#define VENDOR_ID_STR_MAX	128
#define MODEL_NAME_STR_MAX	128

struct cpu_stat {
	u64 user;
	u64 nice;
	u64 system;
	u64 idle;
	u64 iowait;
	u64 irq;
	u64 softirq;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
	u64 steal;
	u64 guest;
	u64 guest_nice;
#endif
	u64 total;
	u64 used;
	u64 load;
};

struct core_struct {
	struct list_head list;
	unsigned long index;
	struct cpu_struct *p_cpu;
	/* current frequency of the core, KHZ */
	u32 freq;
	struct cpu_stat stat;
};

struct cpu_info {
	char vendor_id[VENDOR_ID_STR_MAX];
	unsigned long cpu_family;
	unsigned long model;
	char model_name[MODEL_NAME_STR_MAX];
	u32 bogomips;
};

struct cpu_struct {
	struct cpu_info	cpu_info;
	unsigned long core_num;
	struct list_head cores;
};

struct cpu_head {
	unsigned long total_sockets;
	unsigned long total_cores;
	unsigned long total_threads;
	struct list_head cpus;
	struct list_head cores;
};

extern struct cpu_head *cpu_init(void);

#endif
