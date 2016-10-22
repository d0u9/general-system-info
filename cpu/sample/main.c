#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#include "../cpu.h"

#define INTERVAL	2

int main(void)
{
	struct cpus *cpu_root = NULL;

	cpu_root = cpu_init();
	cpu_update(cpu_root);
	u64 p_used = cpu_root->stat.used;
	u64 p_total = cpu_root->stat.total;
	printf("used = %"PRIu64" total = %"PRIu64"\n", p_used, p_total);

	sleep(INTERVAL);
	cpu_update(cpu_root);
	u64 c_used = cpu_root->stat.used;
	u64 c_total = cpu_root->stat.total;
	u64 i_used = c_used - p_used;
	u64 i_total = c_total - p_total;
	p_used = c_used;
	p_total = c_total;
	printf("used = %"PRIu64" total = %"PRIu64"\n", c_used, c_total);
	printf("load %f%%\n", ((float)(i_used * 100)) / ((float)i_total));

	sleep(INTERVAL);
	cpu_update(cpu_root);
	c_used = cpu_root->stat.used;
	c_total = cpu_root->stat.total;
	i_used = c_used - p_used;
	i_total = c_total - p_total;
	printf("used = %"PRIu64" total = %"PRIu64"\n", c_used, c_total);
	printf("load %f%%\n", ((float)(i_used * 100)) / ((float)i_total));

	sleep(INTERVAL);
	cpu_update(cpu_root);
	c_used = cpu_root->stat.used;
	c_total = cpu_root->stat.total;
	i_used = c_used - p_used;
	i_total = c_total - p_total;
	printf("used = %"PRIu64" total = %"PRIu64"\n", c_used, c_total);
	printf("load %f%%\n", ((float)i_used) / ((float)i_total));
	printf("load %f%%\n", ((float)(i_used * 100)) / ((float)i_total));

	cpu_exit(cpu_root);

	return 0;
}
