
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

void mv_error_display(mv_error* error, FILE* dst) {
	char* message = mv_error_show(error);
	fputs(message, dst);
	fflush(stderr);
	free(message);
	mv_error_release(error);
}

char* mv_error_show(mv_error* error) {
	char* prefix = NULL;
	switch(error->code) {
		case MVERROR_SYNTAX:
			prefix = "Syntax error";
			break;
		case MVERROR_INTERNAL:
			prefix = "Internal error";
			break;
		case MVERROR_BADVAR:
			prefix = "Bad variable";
			break;
		default: 
			fprintf(stderr, "Unknown error code %d\n", error->code);
			abort();
	}
	char* result = (char*)malloc(sizeof(char) * (strlen(error->message) + strlen(prefix) + 30));
	sprintf(result, "%s: %s\n", prefix, error->message);
	return result;
}

mv_error* mv_error_raise(int code, char* message) {
	mv_error* result = (mv_error*)malloc(sizeof(mv_error));
	result->code = code;
	result->message = strdup(message);
	return result;
}

mv_error* mv_error_raiseform(int code, char* format, char* message) {
	char* buffer = (char*)malloc(sizeof(char)*(strlen(format) + strlen(message)));
	sprintf(buffer, format, message);
	mv_error* result = (mv_error*)malloc(sizeof(mv_error));
	result->code = code;
	result->message = buffer;
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
	char* result = (char*)malloc(sizeof(char) * (len + 1));
	strncpy(result, source + first, len);
	result[len] = '\0';
	return result;
}
