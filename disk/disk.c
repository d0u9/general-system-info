#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <disk.h>
#include <trilib/list.h>

#define SYSFS_DIR	"/sys/block"

void scan_sysfs(struct disks *disk_root, struct list_head *tmp_disk_list,
		struct list_head *tmp_partitions)
{
	DIR *sys_dir = NULL;
	struct dirent *dirent = NULL;

	sys_dir = opendir(SYSFS_DIR);

	while((dirent = readdir(sys_dir))) {
		if(!strcmp(".", dirent->d_name) || !strcmp("..", dirent->d_name))
			continue;

		printf("%s\n", dirent->d_name);
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


	scan_sysfs(disk_root, &tmp_disk_list, &tmp_partitions);

}

void disk_exit(struct disks *disk_root)
{

}


