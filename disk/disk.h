#ifndef _TRI_DISK_H
#define _TRI_DISK_H

#include <trilib/core.h>
#include <trilib/list.h>

#define VENDOR_NAME_LEN_MAX	128
#define MODEL_NAME_LEN_MAX	128
#define DEV_NAME_LEN_MAX	16
#define PATH_NAME_LEN_MAX	512
#define MOUNT_PARM_LEN_MAX	512

struct io_desc {
	u64 read_io;
	u64 write_io;
	u64 read_io_merged;
	u64 write_io_merged;
	u64 read_sector;
	u64 write_sector;
};

struct mount_desc {
	struct list_head mount_list;
	char mount_point[PATH_NAME_LEN_MAX];
	char fs_type[64];
	char mount_parm[MOUNT_PARM_LEN_MAX];
};

struct disk_desc {
	char devname[DEV_NAME_LEN_MAX];
	unsigned long size;				//in 512bytes sectors
	char vendor[VENDOR_NAME_LEN_MAX];
	char model[MODEL_NAME_LEN_MAX];
	struct io_desc io;
	struct list_head disk_list;
	struct list_head d_partitions;
};

struct partition_desc {
	char devname[DEV_NAME_LEN_MAX];
	int mounted;
	unsigned long part_num;
	unsigned long bsize;
	unsigned long frsize;
	unsigned long blocks;
	unsigned long bfree;
	unsigned long bavail;
	unsigned long files;
	unsigned long ffree;
	unsigned long favail;
	unsigned long fsid;
	unsigned long flag;
	unsigned long namemax;
	unsigned long size;
	struct io_desc io;
	struct list_head mount_list;
	struct disk_desc p_disk;
	struct list_head d_partitions;
	struct list_head partitions;
};

struct disks {
	u64 total_read;
	u64 total_write;
	struct list_head disk_list;
	struct list_head partitions;
};

extern struct disks *disk_init(void);
extern void disk_update(struct disks *disk_root);
extern void disk_exit(struct disks *disk_root);

#endif /* _TRI_DISK_H */
