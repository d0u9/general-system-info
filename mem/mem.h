#ifndef _TRI_MEM_H
#define _TRI_MEM_H

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

extern struct mem_desc *mem_init(void);
extern void mem_update(struct mem_desc *mem);
extern void mem_exit(struct mem_desc *mem);

#endif /* _TRI_MEM_H */
