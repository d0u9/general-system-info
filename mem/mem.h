#ifndef _TRI_MEM
#define _TRI_MEM

struct mem_desc {
	unsigned long total;
	unsigned long used;
	unsigned long free;
	unsigned long shared;
	unsigned long buffers;
	unsigned long cached;
	unsigned long swap_total;
	unsigned long swap_used;
	unsigned long swap_free;
};

#endif /* _TRI_MEM */
