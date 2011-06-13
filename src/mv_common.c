
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

int mv_strhash(char* str) {
	int result = 0;
	while (*str != '\0') {
		result = result * 7 + (*str);
		str++;
	}
	if (result < 0) result = -result;
	return result;
}

char* mv_strslice(char* source, int first, int last) {
	int len = last - first;
	char* result = malloc(sizeof(char) * (len + 1));
	strncpy(result, source + first, len);
	result[len] = '\0';
	return result;
}
