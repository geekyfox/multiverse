
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

mv_strref mv_strslice(const char* source, int first, int last) {
	int len = last - first;
	mv_strref ref = mv_strref_alloc(len);
	strncpy(ref.ptr, source + first, len);
	ref.ptr[len] = '\0';
	return ref;
}

