#include <stdlib.h>

#include <utils.h>
#include <trilib/log.h>
#include <mem.h>

#define PROC_MEMINFO_FILE	"/proc/meminfo"

static unsigned long fetch_line(struct mem_desc *mem, char *line, int len)
{
	if (!strncmp("MemTotal", line, strlen("MemTotal"))) {
		mem->total = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->total: %lu\n", mem->total);
	}

	if (!strncmp("MemFree", line, strlen("MemFree"))) {
		mem->free = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->free: %lu\n", mem->free);
	}

	if (!strncmp("MemAvailable", line, strlen("MemAvailable"))) {
		mem->available = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->available: %lu\n", mem->available);
	}

	if (!strncmp("Shmem", line, strlen("Shmem"))) {
		mem->shared = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->shared: %lu\n", mem->shared);
	}

	if (!strncmp("Buffers", line, strlen("Buffers"))) {
		mem->buffers = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->buffers: %lu\n", mem->buffers);
	}

	if (!strncmp("Cached", line, strlen("Cached"))) {
		mem->cached = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->cached: %lu\n", mem->cached);
	}

	if (!strncmp("SwapTotal", line, strlen("SwapTotal"))) {
		mem->swap_total = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->swap_total: %lu\n", mem->swap_total);
	}

	if (!strncmp("SwapFree", line, strlen("SwapFree"))) {
		mem->swap_free = stoul(split_to_key_val(':', line, len));
		printl_debug("mem->swap_free: %lu\n", mem->swap_free);
	}

	return 0;
}

struct mem_desc *mem_init(void)
{
	return calloc(1, sizeof(struct mem_desc));
}

void mem_update(struct mem_desc *mem)
{
           FILE *fp;
           char *line = NULL;
           size_t len = 0;
           ssize_t read;

           fp = fopen(PROC_MEMINFO_FILE, "r");
           if (!fp) {
		   printl_err("open "PROC_MEMINFO_FILE" error!\n");
		   return;
	   }

	   memset(mem, 0, sizeof(struct mem_desc));
           while ((read = getline(&line, &len, fp)) != -1) {
		   fetch_line(mem, line, read);
           }

	   mem->used = mem->total - mem->free;
	   mem->swap_used = mem->swap_total - mem->swap_free;

           free(line);
           fclose(fp);
}

void mem_exit(struct mem_desc *mem)
{
	free(mem);
}
