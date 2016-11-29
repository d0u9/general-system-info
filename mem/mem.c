#include <stdlib.h>

#include <utils.h>
#include <trilib/log.h>
#include <mem.h>

struct mem_desc *mem_init(void)
{
	return calloc(1, sizeof(struct mem_desc));
}

void mem_update(struct mem_desc *mem)
{

}

void mem_exit(struct mem_desc *mem)
{
	free(mem)
}
