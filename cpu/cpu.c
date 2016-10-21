#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../trilib/log.h"
#include "../utils.h"
#include "../trilib/bitmap.h"
#include "cpu.h"

#define FILE_PROC_STAT		"/proc/stat"
#define FILE_PROC_CPUINFO	"/proc/cpuinfo"

struct cpu_sys_file_fps {
	FILE *proc_stat;
	FILE *proc_cpuinfo;
};

struct raw_core_info {
	unsigned int processor;
	char vendor_id[VENDOR_ID_STR_MAX];
	u32 cpu_family;
	u32 model;
	char model_name[MODEL_NAME_STR_MAX];
	u32 stepping;
	u32 microcode;
	u32 cpu_freq;
	char cache_size[CACHE_SIZE_STR_MAX];
	u32 physical_id;
	u32 siblings;
	u32 core_id;
	u32 cpu_cores;
	u32 apicid;
	u32 initial_apicid;
	u32 bogomips;
};

static struct cpu_sys_file_fps *cpu_sys_files = NULL;

static struct core_desc *find_core(struct list_head *head, unsigned long index)
{
	struct core_desc *core = list_entry(head->next, struct core_desc, list);
	list_for_each_entry_from(core, head, list) {
		printl_debug("find %d : %p\n", index, core);
		if (core->processor_index == index)
			return core;
	}
	return NULL;
}

static void update_core_from_cpuinfo(struct core_desc *core,
				    struct raw_core_info *raw_coreinfo)
{
	core->processor_index = raw_coreinfo->processor;
	core->core_index = raw_coreinfo->core_id;
	core->freq = raw_coreinfo->cpu_freq;
}

static void update_cpu_from_cpuinfo(struct cpu_desc *cpu,
				    struct raw_core_info *raw_coreinfo)
{
	strncpy(cpu->cpu_info.vendor_id, raw_coreinfo->vendor_id, VENDOR_ID_STR_MAX);
	cpu->cpu_info.cpu_family = raw_coreinfo->cpu_family;
	cpu->cpu_info.model = raw_coreinfo->model;
	strncpy(cpu->cpu_info.model_name, raw_coreinfo->model_name, MODEL_NAME_STR_MAX);
	cpu->cpu_info.bogomips = raw_coreinfo->bogomips;
	cpu->cpu_info.stepping = raw_coreinfo->stepping;
	strncpy(cpu->cpu_info.cache_size, raw_coreinfo->cache_size, CACHE_SIZE_STR_MAX);
	cpu->core_num = raw_coreinfo->cpu_cores;
	cpu->thread_num = raw_coreinfo->siblings;
}

static void pre_update_cpu_from_cpuinfo(struct cpus *cpu_root,
					struct raw_core_info *raw_coreinfo,
					unsigned long *old_core_bitmap)
{
	struct core_desc *core = NULL;
	struct cpu_desc *cpu = NULL;
	unsigned long core_index = raw_coreinfo->processor;
	unsigned long cpu_index = raw_coreinfo->physical_id;

	if (test_bit(core_index, old_core_bitmap)) {
		printl_debug("cpu_index = %d\n", cpu_index);
		cpu = cpu_root->cpus[cpu_index];
		assert(cpu);
		core = find_core(&cpu_root->cores, core_index);
		assert(core);
	} else {
		if (cpu_root->cpus[cpu_index] == NULL) {
			cpu = calloc(1, sizeof(struct cpu_desc));
			printl_debug("new created cpu_index = %d at %p\n",
				     cpu_index, cpu);
			cpu_root->cpus[cpu_index] = cpu;
			INIT_LIST_HEAD(&cpu->cores);
			printl_debug("Create new cpu %d\n", cpu_index);
		} else {
			cpu = cpu_root->cpus[cpu_index];
			printl_debug("cpu_index = %d, add %p\n", cpu_index, cpu);
		}
		core = calloc(1, sizeof(struct core_desc));
		INIT_LIST_HEAD(&core->list);
		INIT_LIST_HEAD(&core->sibling_list);
		printl_debug("Create new core %d, %p\n", core_index, core);
		list_add_tail(&core->list, &cpu_root->cores);
		list_add_tail(&core->sibling_list, &cpu->cores);
	}

	core->p_cpu = cpu;
	set_bit(core_index, cpu_root->core_bitmap);
	set_bit(cpu_index, cpu_root->cpu_bitmap);

	update_cpu_from_cpuinfo(cpu, raw_coreinfo);
	update_core_from_cpuinfo(core, raw_coreinfo);
}

static void update_raw_coreinfo(struct raw_core_info *raw_coreinfo,
				const char *key, const char *val)
{
	if        (strncmp("processor", key, strlen("processor")) == 0) {
		memset(raw_coreinfo, 0, sizeof(struct cpu_info));
		raw_coreinfo->processor = stoul(val);
	} else if (strncmp("vendor_id", key, strlen("vendor_id")) == 0) {
		strncpy(raw_coreinfo->vendor_id, val, VENDOR_ID_STR_MAX);
	} else if (strncmp("cpu family", key, strlen("cpu family")) == 0) {
		raw_coreinfo->cpu_family = stoul(val);
	} else if (strncmp("model", key, strlen("model")) == 0) {
		raw_coreinfo->model = stoul("model");
	} else if (strncmp("model name", key, strlen("model name")) == 0) {
		strncpy(raw_coreinfo->model_name, key, MODEL_NAME_STR_MAX);
	} else if (strncmp("stepping", key, strlen("stepping")) == 0) {
		raw_coreinfo->stepping = stoul(val);
	} else if (strncmp("microcode", key, strlen("microcode")) == 0) {
		raw_coreinfo->microcode = strtoul(val, NULL, 16);
	} else if (strncmp("cpu MHz", key, strlen("cpu MHz")) == 0) {
		double freq = strtod(val, NULL) * 1000;
		raw_coreinfo->cpu_freq = (int)freq;
	} else if (strncmp("cache size", key, strlen("cache size")) == 0) {
		strncpy(raw_coreinfo->cache_size, key, CACHE_SIZE_STR_MAX);
	} else if (strncmp("physical id", key, strlen("physical id")) == 0) {
		raw_coreinfo->physical_id = stoul(val);
	} else if (strncmp("siblings", key, strlen("siblings")) == 0) {
		raw_coreinfo->siblings = stoul(val);
	} else if (strncmp("core id", key, strlen("core id")) == 0) {
		raw_coreinfo->core_id = stoul(val);
	} else if (strncmp("cpu cores", key, strlen("cpu cores")) == 0) {
		raw_coreinfo->cpu_cores = stoul(val);
	} else if (strncmp("apicid", key, strlen("apicid")) == 0) {
		raw_coreinfo->apicid = stoul(val);
	} else if (strncmp("initial apicid", key, strlen("initial apicid")) ==0) {
		raw_coreinfo->initial_apicid = stoul(val);
	} else if (strncmp("bogomips", key, strlen("bogomips")) == 0) {
		double bogomips = strtod(val, NULL) * 1000;
		raw_coreinfo->bogomips = (int)bogomips;
	}
}

static void shrink_cpu_array(struct cpus *cpu_root)
{
	//TODO: here

}

static void shrink_core_list(struct cpus *cpu_root)
{
	//TODO: here
}

static int parse_proc_cpuinfo(struct cpus *cpu_root,
			      struct cpu_sys_file_fps *cpu_sys_files,
			      unsigned long *old_core_bitmap)
{
	char *buf = NULL;
	struct raw_core_info raw_coreinfo;
	size_t len = 128;
	ssize_t read_len;

	while ((read_len = getline(&buf ,&len, cpu_sys_files->proc_cpuinfo)) != -1) {
		if (buf[0] == '\n')
			pre_update_cpu_from_cpuinfo(cpu_root, &raw_coreinfo,
						    old_core_bitmap);

		char *val = split_to_key_val(':', buf, read_len + 1);
		char *key = trim_whitespaces(buf, read_len);
		val = trim_whitespaces(val, read_len);

		update_raw_coreinfo(&raw_coreinfo, key, val);
	}

	free(buf);

	return 0;
}

static int open_sys_files(struct cpu_sys_file_fps *cpu_sys_files)
{
	int ret = 0;
	cpu_sys_files->proc_stat = fopen(FILE_PROC_STAT, "r");
	if (!cpu_sys_files->proc_stat) {
		printl_err("can't open file: %s\n", FILE_PROC_STAT);
		ret = -1;
	}
	printl_debug("opened file: %s\n", FILE_PROC_STAT);

	cpu_sys_files->proc_cpuinfo = fopen(FILE_PROC_CPUINFO, "r");
	if (!cpu_sys_files->proc_cpuinfo) {
		printl_err("can't open file: %s\n", FILE_PROC_CPUINFO);
		ret = -1;
	}
	printl_debug("opened file: %s\n", FILE_PROC_CPUINFO);

	return ret;
}

static void update_sys_files(struct cpu_sys_file_fps *cpu_sys_files)
{
	/*
	 * For easy handling, we regard struct cpu_sys_file_fps as 
	 * an array of FILE*
	 */
	FILE **fp_array = (FILE **)cpu_sys_files;
	int len = sizeof(struct cpu_sys_file_fps) / sizeof(FILE *);

	for (int i = 0; i < len; ++i) {
		if (fp_array[i]) {
			fflush(fp_array[i]);
			rewind(fp_array[i]);
		}
	}
}

struct cpus *cpu_init(void)
{
	struct cpus *cpu_root = NULL;
	int ret = 0;

	cpu_root = calloc(1, sizeof(struct cpus));
	if (!cpu_root) {
		printl_crit("can't alloc memory for struct cpus\n");
		return NULL;
	}
	printl_debug("alloc memory for struct cpus\n");

	INIT_LIST_HEAD(&cpu_root->cores);

	/* cpu_sys_files is declared as a global variable */
	cpu_sys_files = calloc(1, sizeof(struct cpu_sys_file_fps));
	if (!cpu_sys_files) {
		printl_crit("can't alloc memory for struct cpu_sys_file_fps\n");
		return NULL;
	}
	printl_debug("alloc memory for struct cpu_sys_file_fps\n");

	ret = open_sys_files(cpu_sys_files);
	if (ret < 0) {
		return NULL;
	}

	return cpu_root;
}

void cpu_update(struct cpus *cpu_root)
{
	DECLARE_BITMAP(old_core_bitmap, CORES_NUM_MAX);
	bitmap_copy(old_core_bitmap, cpu_root->core_bitmap, CORES_NUM_MAX);
	bitmap_zero(cpu_root->core_bitmap, CORES_NUM_MAX);
	bitmap_zero(cpu_root->cpu_bitmap, CPUS_NUM_MAX);
	update_sys_files(cpu_sys_files);
	parse_proc_cpuinfo(cpu_root, cpu_sys_files, old_core_bitmap);
	shrink_cpu_array(cpu_root);
	shrink_core_list(cpu_root);
}

static void free_cpu_sys_file_fps(struct cpu_sys_file_fps *cpu_sys_files)
{
	/*
	 * For easy handling, we regard struct cpu_sys_file_fps as 
	 * an array of FILE*
	 */
	FILE **fp_array = (FILE **)cpu_sys_files;
	int len = sizeof(struct cpu_sys_file_fps) / sizeof(FILE *);

	for (int i = 0; i < len; ++i) {
		if (fp_array[i])
			fclose(fp_array[i]);
	}
}

static void free_cores(struct cpus *cpu_root)
{
	struct core_desc *pos, *tmp;
	pos = list_entry(cpu_root->cores.next, struct core_desc, list);
	printl_debug("core head = %p, pre = %p, net = %p\n", &cpu_root->cores,
		     cpu_root->cores.prev, cpu_root->cores.next);
	list_for_each_entry_safe_from(pos, tmp, &cpu_root->cores, list) {
		printl_debug("pos = %p, pre = %p, net = %p\n", &pos->list,
			     pos->list.prev, pos->list.next);
		list_del(&pos->list);
		free(pos);
	}
}

static void free_cpus(struct cpus *cpu_root)
{
	for (int i =0; i < CPUS_NUM_MAX; ++i) {
		if (cpu_root->cpus[i])
			free(cpu_root->cpus[i]);
	}
}

void cpu_exit(struct cpus *cpu_root)
{
	if (cpu_root) {
		free_cores(cpu_root);
		free_cpus(cpu_root);
		free(cpu_root);
	}

	if (cpu_sys_files) {
		free_cpu_sys_file_fps(cpu_sys_files);
	}
	printl_info("memory freed\n");
}


