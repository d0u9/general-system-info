#ifndef _TRI_UTILS_H
#define _TRI_UTILS_H

#include <stdlib.h>

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

#endif