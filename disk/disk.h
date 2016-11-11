#ifndef _TRI_DISK_H
#define _TRI_DISK_H

#include <trilib/core.h>
#include <trilib/list.h>

#define MODEL_NAME_LEN_MAX	128

struct io_desc {
	u64 read_io;
	u64 write_io;
	u64 read_io_merged;
	u64 write_io_merged;
	u64 read_sector;
	u64 write_sector;
};

struct disk_desc {
	unsigned long size;				//in 512bytes sectors
	char model[MODEL_NAME_LEN_MAX];
	struct io_desc io;
	struct list_head disks;
	struct list_head partitions;
};


struct partition_desc {
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
	struct io_desc io;
	struct disk_desc *p_disk;
	struct list_head partitions;
	struct list_head all_partitions;
};


struct disks {
	u64 total_read;
	u64 total_write;
	struct list_head disks;
	struct list_head all_partitions;
};

#endif /* _TRI_DISK_H */
