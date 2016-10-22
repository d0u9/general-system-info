#ifndef _TRI_CPU_H
#define _TRI_CPU_H

#include <linux/version.h>
#include <trilib/core.h>
#include <trilib/list.h>
#include <trilib/bitmap.h>

#define CPUS_NUM_MAX		32
#define CORES_NUM_PER_CPU	32
#define CORES_NUM_MAX		(CPUS_NUM_MAX * CORES_NUM_PER_CPU)
#define VENDOR_ID_STR_MAX	128
#define MODEL_NAME_STR_MAX	128
#define CACHE_SIZE_STR_MAX	64

struct core_stat {
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
};

struct core_desc {
	struct list_head list;
	struct list_head sibling_list;
	unsigned long processor_index;
	unsigned long core_index;
	struct cpu_desc *p_cpu;
	/* current frequency of the core, KHZ */
	u32 freq;
	struct core_stat stat;
};

struct cpu_info {
	char vendor_id[VENDOR_ID_STR_MAX];
	unsigned long cpu_family;
	unsigned long model;
	char model_name[MODEL_NAME_STR_MAX];
	u32 bogomips;
	u32 stepping;
	char cache_size[CACHE_SIZE_STR_MAX];
};

struct cpu_desc {
	struct cpu_info	cpu_info;
	unsigned long core_num;
	unsigned long thread_num;
	struct list_head cores;
};

struct cpus {
	unsigned long total_sockets;
	unsigned long total_cores;
	unsigned long cpu_bitmap[BITS_TO_LONGS(CPUS_NUM_MAX)];
	struct cpu_desc *cpus[CPUS_NUM_MAX];
	unsigned long core_bitmap[BITS_TO_LONGS(CORES_NUM_MAX)];
	struct list_head cores;
	struct core_stat stat;
};

extern struct cpus *cpu_init(void);
extern void cpu_exit(struct cpus *cpu_root);
extern void cpu_update(struct cpus *cpu_root);

#endif
