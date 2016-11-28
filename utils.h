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

static inline char *strip_newline(char *buff, int len)
{
	//binary search
	int left = 0, right = len - 1, mid = (left + right) / 2;

	while (mid != left && mid != right) {
		if (buff[mid] == '\0')
			right = mid;
		else
			left = mid;
		mid = (left + right) / 2;
	}

	if (buff[left] == '\n')
		buff[left] = 0;

	return buff;
}

static inline char *first_line(const char *path, char *buff, int len)
{
	FILE *fp = fopen(path, "r");
	char *ret = fgets(buff, len, fp);
	fclose(fp);
	return ret;
}

#define first_line_no_nl(path, buff, len)				\
	strip_newline(first_line(path, buff, len), len)

#endif /* _TRI_UTILS_H */
