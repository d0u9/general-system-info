#include <stdio.h>

#include <mem.h>

int main(void)
{
	struct mem_desc *mem;
	mem = mem_init();

	printf("------------------------------------\n");
	mem_update(mem);
	printf("------------------------------------\n");
	mem_update(mem);
	printf("------------------------------------\n");
	mem_update(mem);
	printf("------------------------------------\n");
	mem_update(mem);
	printf("------------------------------------\n");
	mem_update(mem);
	printf("------------------------------------\n");
	mem_exit(mem);
	return 0;
}
