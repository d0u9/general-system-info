#include <stdio.h>
#include <disk.h>

int main(void)
{
	struct disks *disk_root = NULL;

	printf("hello world!\n");
	printf("---------------------------------------\n");

	disk_root = disk_init();

	printf("---------------------------------------\n");
	disk_update(disk_root);

	printf("---------------------------------------\n");
	disk_update(disk_root);

	printf("---------------------------------------\n");
	disk_update(disk_root);

	printf("---------------------------------------\n");
	disk_exit(disk_root);

	return 0;
}
