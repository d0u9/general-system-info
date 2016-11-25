#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <utils.h>
#include <inttypes.h>
#include <sys/statvfs.h>

#include <disk.h>
#include <trilib/list.h>

#define SYSFS_DIR	"/sys/block"

struct mount_wrap_desc {
	struct list_head list;
	char devname[DEV_NAME_LEN_MAX];
	struct mount_desc mount_info;
};

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
	INIT_LIST_HEAD(&ret->mount_list);
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

void get_io(struct io_desc *io, const char *path)
{
	char buff[1024] = {0};
	get_first_line(path, buff, 1024);

	sscanf(buff, "%"SCNu64" %"SCNu64" %"SCNu64" %*"SCNu64" "
	       "%"SCNu64" %"SCNu64" %"SCNu64" %*"SCNu64" "
	       "%*"SCNu64" %*"SCNu64" %*"SCNu64,
	       &io->read_io, &io->read_io_merged, &io->read_sector,
	       &io->write_io, &io->write_io_merged, &io->write_sector);
}

void get_disk_info(struct disk_desc *disk)
{
	char file_path[1024] = {0};
	char buff[1024] = {0};

	snprintf(file_path, 1024, SYSFS_DIR"/%s/size", disk->devname);
	disk->size = stoul(get_first_line(file_path, buff, 1024));

	snprintf(file_path, 1024, SYSFS_DIR"/%s/device/model", disk->devname);
	get_first_line(file_path, disk->model, MODEL_NAME_LEN_MAX);

	snprintf(file_path, 1024, SYSFS_DIR"/%s/device/vendor", disk->devname);
	get_first_line(file_path, disk->vendor, VENDOR_NAME_LEN_MAX);

	snprintf(file_path, 1024, SYSFS_DIR"/%s/stat", disk->devname);
	get_io(&disk->io, file_path);
}

void get_mount_info(struct partition_desc *part, struct list_head *mount_wrap_list)
{
	printf("========== %s\n", part->devname);
	struct mount_wrap_desc *cur = NULL, *tmp = NULL;
	list_for_each_entry_safe(cur, tmp, mount_wrap_list, list) {
		if (strncmp(cur->devname, part->devname, DEV_NAME_LEN_MAX))
			continue;

		list_add_tail(&cur->mount_info.mount_list, &part->mount_list);
		part->mounted = 1;
	}
}

void get_vfs_stat(struct partition_desc *part)
{
	printf("part %s\n", part->devname);
	struct statvfs vfs_stat;

	memset(&vfs_stat, 0, sizeof(vfs_stat));
	printf("mount_point %s\n", first_mount_point(part)->mount_point);

	statvfs(first_mount_point(part)->mount_point, &vfs_stat);

	printf("bs=%lu, f_b=%lu\n", vfs_stat.f_bsize, vfs_stat.f_bfree);
	printf("inodes=%lu, f_inodes=%lu\n", vfs_stat.f_files, vfs_stat.f_ffree);

	part->bsize = vfs_stat.f_bsize;
	part->frsize = vfs_stat.f_frsize;
	part->blocks = vfs_stat.f_blocks;
	part->bfree = vfs_stat.f_bfree;
	part->bavail = vfs_stat.f_bavail;
	part->files = vfs_stat.f_files;
	part->ffree = vfs_stat.f_ffree;
	part->favail = vfs_stat.f_bavail;
	part->fsid = vfs_stat.f_fsid;
	part->flag = vfs_stat.f_flag;
	part->namemax = vfs_stat.f_namemax;
}

void get_partition_info(struct partition_desc *part, const char *parent_path,
			struct list_head *mount_wrap_list)
{
	char file_path[1024] = {0};
	char buff[1024] = {0};

	snprintf(file_path, 1024, "%s/%s/size", parent_path, part->devname);
	part->size = stoul(get_first_line(file_path, buff, 1024));

	snprintf(file_path, 1024, "%s/%s/partition", parent_path, part->devname);
	part->part_num = stoul(get_first_line(file_path, buff, 1024));

	snprintf(file_path, 1024, "%s/%s/stat", parent_path, part->devname);
	get_io(&part->io, file_path);

	get_mount_info(part, mount_wrap_list);

	if (part->mounted)
		get_vfs_stat(part);
}

void scan_partitions(struct disks *disk_root, struct list_head *mount_wrap_list,
		     struct disk_desc *cur_disk, struct list_head *tmp_partitions)
{
	DIR *disk_dir = NULL;
	struct dirent *dirent = NULL;
	struct partition_desc *part = NULL;
	char part_path[1024] = {0};
	int dev_name_len = 0;

	snprintf(part_path, 1024, SYSFS_DIR"/%s", cur_disk->devname);
	disk_dir = opendir(part_path);

	dev_name_len = strlen(cur_disk->devname);
	while ((dirent = readdir(disk_dir))) {
		if (!strcmp(".", dirent->d_name) || !strcmp("..", dirent->d_name))
			continue;

		if (strncmp(cur_disk->devname, dirent->d_name, dev_name_len))
			continue;

		part = get_next_partition(tmp_partitions);
		strncpy(part->devname, dirent->d_name, DEV_NAME_LEN_MAX);
		get_partition_info(part, part_path, mount_wrap_list);
		list_add_tail(&part->partitions, &disk_root->partitions);
		list_add_tail(&part->d_partitions, &cur_disk->d_partitions);
	}

}

void scan_disks(struct disks *disk_root, struct list_head *mount_wrap_list,
		struct list_head *tmp_disk_list, struct list_head *tmp_partitions)
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
		get_disk_info(disk);
		scan_partitions(disk_root, mount_wrap_list, disk, tmp_partitions);
		list_add_tail(&disk->disk_list, &disk_root->disk_list);
	}

	// TODO: free all the unused list nodes in tmp_partitions and
	// tmp_disk_list
	//
}

struct mount_wrap_desc *get_next_mount_wrapper(struct list_head *mount_wrap_list)
{
	struct mount_wrap_desc *ret = NULL;
	if (list_empty(mount_wrap_list)) {
		ret = calloc(1, sizeof(struct mount_wrap_desc));
	} else {
		ret = list_first_entry(mount_wrap_list, struct mount_wrap_desc, list);
		list_del(&ret->list);
	}

	INIT_LIST_HEAD(&ret->list);

	return ret;
}



void parse_mount_file(struct list_head *mount_wrap_list)
{

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	struct mount_wrap_desc *cur_mount = NULL;
	LIST_HEAD(new_wrap_list);


	fp = fopen("/proc/mounts", "r");
	if (fp == NULL)
		return;

	while (getline(&line, &len, fp) != -1) {
		if (strncmp("/dev", line, 4))
			continue;
		cur_mount = get_next_mount_wrapper(mount_wrap_list);
		sscanf(line + 5, "%s %s %s %s", cur_mount->devname,
					cur_mount->mount_info.mount_point,
		                        cur_mount->mount_info.fs_type,
					cur_mount->mount_info.mount_parm);
		INIT_LIST_HEAD(&cur_mount->mount_info.mount_list);
		list_add_tail(&cur_mount->list, &new_wrap_list);
	}

	free(line);
	fclose(fp);

	//TODO: free unused elements

	list_splice(&new_wrap_list, mount_wrap_list);
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
	list_splice_init(&disk_root->partitions, &tmp_partitions);
	list_splice_init(&disk_root->disk_list, &tmp_disk_list);

	// declare mount_list as static to prevent free from each iteration.
	static LIST_HEAD(mount_wrap_list);
	parse_mount_file(&mount_wrap_list);

	scan_disks(disk_root, &mount_wrap_list, &tmp_disk_list, &tmp_partitions);

}

void disk_exit(struct disks *disk_root)
{
	struct partition_desc *cur_part = NULL, *tmp_part = NULL;
	list_for_each_entry_safe(cur_part, tmp_part, &disk_root->partitions, partitions) {
		free(cur_part);
	}

	struct disk_desc *cur_disk = NULL, *tmp_disk = NULL;
	list_for_each_entry_safe(cur_disk, tmp_disk, &disk_root->disk_list, disk_list) {
		free(cur_disk);
	}

	free(disk_root);
}


