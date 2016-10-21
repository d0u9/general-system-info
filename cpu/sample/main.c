#include <stdio.h>

#include "../cpu.h"

int main(void)
{
	struct cpus *cpu_root = NULL;

	cpu_root = cpu_init();

	cpu_update(cpu_root);
	cpu_update(cpu_root);

	cpu_exit(cpu_root);

	return 0;
}
