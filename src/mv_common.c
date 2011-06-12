
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

char* __mv_error_prefix(int code) {
	switch(code) {
	case MVERROR_BADCMD:   return "Bad command";
	case MVERROR_BADVAR:   return "Bad variable";
	case MVERROR_INTERNAL: return "Internal error";
	case MVERROR_SYNTAX:   return "Syntax error";
	}
	DIE("Unknown error code (%d)\n", code);
}

void mv_error_display(mv_error* error, FILE* dst) {
	char* prefix = __mv_error_prefix(error->code);
	fprintf(dst, "%s: %s\n", prefix, error->message);
	mv_error_release(error);
}

char* mv_error_show(mv_error* error) {
	char *result, *prefix = __mv_error_prefix(error->code);
	asprintf(&result, "%s: %s\n", prefix, error->message);
	return result;
}

void mv_error_release(mv_error* error) {
	free(error->message);
	free(error);
}

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
