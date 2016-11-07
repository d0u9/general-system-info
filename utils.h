#ifndef _TRI_UTILS_H
#define _TRI_UTILS_H

#include <stdlib.h>
#include <stdio.h>

extern int is_char_in_str(char ch, const char *str, int max_len);
extern char *split_to_key_val(char delimter, char *str, int max_len);
extern char *trim_leadings(const char *char_list, int list_len,
			   char *str, int max_len);
extern char *trim_trailings(const char *ch_list, int list_len,
			    char *str, int max_len);
extern char *trim(const char *ch_list, int list_len,
		  char *str, int max_len);

#define trim_whitespaces(str, len)	trim(" \n\t", 4, str, len)
#define stoul(str)	strtoul(str, NULL, 0)
#define stoulx(str)	strtoul(str, NULL, 16)

static inline char *get_first_line(const char *path, char *buff, int len)
{
	FILE *fp = fopen(path, "r");
	char *ret = fgets(buff, len, fp);
	fclose(fp);
	return ret;
}

#endif
