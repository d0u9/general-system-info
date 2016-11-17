#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <disk.h>
#include <trilib/list.h>

#define SYSFS_DIR	"/sys/block"

struct partition_desc *get_next_partition(struct list_head *tmp_partitions)
{
	struct partition_desc *ret = NULL;
	if (list_empty(tmp_partitions)) {
		ret = calloc(1, sizeof(struct partition_desc));
	} else {
		ret = list_entry(tmp_partitions->next, struct partition_desc, partitions);
		list_del_init(tmp_partitions->next);
	}

	INIT_LIST_HEAD(&ret->d_partitions);
	INIT_LIST_HEAD(&ret->partitions);
	return ret;
}

struct disk_desc *get_next_disk(struct list_head *tmp_disk_list)
{
	struct disk_desc *ret = NULL;
	if (list_empty(tmp_disk_list)) {
		ret = calloc(1, sizeof(struct disk_desc));
	} else {
		ret = list_entry(tmp_disk_list->next, struct disk_desc, disk_list);
		list_del_init(tmp_disk_list->next);
	}

	INIT_LIST_HEAD(&ret->disk_list);
	INIT_LIST_HEAD(&ret->d_partitions);
	return ret;
}

void scan_sysfs(struct disks *disk_root, struct list_head *tmp_disk_list,
		struct list_head *tmp_partitions)
{
	DIR *sys_dir = NULL;
	struct dirent *dirent = NULL;
	struct disk_desc *disk = NULL;

	sys_dir = opendir(SYSFS_DIR);

	while ((dirent = readdir(sys_dir))) {
		if(!strcmp(".", dirent->d_name) || !strcmp("..", dirent->d_name))
			continue;

		disk = get_next_disk(tmp_disk_list);
		strncpy(disk->devname, dirent->d_name, DEV_NAME_LEN_MAX);
		printf("%s\n", dirent->d_name);
		list_add_tail(&disk->disk_list, &disk_root->disk_list);
	}

}

struct disks *disk_init(void)
{
	struct disks *ret = NULL;
	ret = (struct disks*)calloc(1, sizeof(struct disks));
	INIT_LIST_HEAD(&ret->disk_list);
	INIT_LIST_HEAD(&ret->partitions);

	return ret;
}

void disk_update(struct disks *disk_root)
{
	LIST_HEAD(tmp_disk_list);
	LIST_HEAD(tmp_partitions);
	/*list_cut_position(&tmp_disk_list, &disk_root->disk_list, (&disk_root->disk_list)->prev);*/
	list_splice_init(&disk_root->partitions, &tmp_partitions);
	list_splice_init(&disk_root->disk_list, &tmp_disk_list);

	scan_sysfs(disk_root, &tmp_disk_list, &tmp_partitions);

}

void disk_exit(struct disks *disk_root)
{

}


