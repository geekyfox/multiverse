
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

int mv_strhash(const char* str) {
	int result = 0;
	while (*str != '\0') {
		result = result * 7 + (*str);
		str++;
	}
	if (result < 0) result = -result;
	return result;
}

