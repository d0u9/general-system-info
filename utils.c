#include <stdio.h>
#include <string.h>

#include "trilib/core.h"
#include "utils.h"

int is_char_in_str(char ch, const char *str, int max_len)
{
	for (int i = 0; i < max_len; ++i) {
		if (str[i] == ch)
			return i;
		if (str[i] == '\0')
			return -1;
	}

	return -1;
}


char *split_to_key_val(char delimiter, char *str, int max_len)
{
	char *ret = NULL;

	for (int i = 0; i < max_len; ++i) {
		if (str[i] == '\0') {
			ret = str + i;
			break;
		}

		if (str[i] == delimiter) {
			str[i] = '\0';
			ret = str + i + 1;
			break;
		}
	}

	return ret;
}

char *trim_leadings(const char *char_list, int list_len,
		    char *str, int max_len)
{
	char *ret = NULL;
	for (int i = 0; i < max_len; ++i) {
		if (str[i] == '\0')
			break;
		if (is_char_in_str(str[i], char_list, list_len) == -1) {
			ret = str + i;
			break;
		}
	}

	return (ret != NULL) ? ret : str;
}

char *trim_trailings(const char *ch_list, int list_len,
		     char *str, int max_len)
{
	char *cursor = NULL;
	bool flag = FALSE;
	for (int i = 0; i < max_len; ++i) {
		if (str[i] == '\0')
			break;
		if (is_char_in_str(str[i], ch_list, list_len) != -1) {
			if (!flag) {
				cursor = str + i;
				flag = TRUE;
			}
		} else {
			flag = FALSE;
		}
	}

	if (flag == TRUE)
		*cursor = '\0';

	return str;
}

char *trim(const char *ch_list, int list_len,
	   char*str, int max_len)
{
	char *ret = NULL, *cursor = NULL;
	/*
	 * 0: initial value
	 * 1: the right side has been trimed
	 * 2: find possible starting point of right side.
	 */
	int flag = 0;
	for (int i = 0; i < max_len; ++i) {
		if (str[i] == '\0')
			break;

		if (is_char_in_str(str[i], ch_list, list_len) != -1) {
			if (flag == 0)
				ret = str + i + 1;
			if (flag == 1) {
				cursor = str + i;
				flag = 2;
			}
		} else {
			if (flag == 0)
				flag = 1;
			if (flag == 2)
				flag = 1;
		}
	}

	if (flag == 2)
		*cursor = '\0';

	return (ret != NULL) ? ret : str;
}
