
#include <stdio.h>
#include "consts.h"
#include "error.h"

inline static char* __prefix__(int code) {
	switch(code) {
	case MVERROR_BADCMD:   return "Bad command";
	case MVERROR_BADVAR:   return "Bad variable";
	case MVERROR_INTERNAL: return "Internal error";
	case MVERROR_SYNTAX:   return "Syntax error";
	}
	DIE("Unknown error code (%d)\n", code);
}

void mv_error_display(mv_error* error, FILE* dst) {
	char* prefix = __prefix__(error->code);
	fprintf(dst, "%s: %s\n", prefix, error->message);
	mv_error_release(error);
}

char* mv_error_show(mv_error* error) {
	char *result, *prefix = __prefix__(error->code);
	asprintf(&result, "%s: %s\n", prefix, error->message);
	return result;
}

void mv_error_release(mv_error* error) {
	free(error->message);
	free(error);
}

mv_error* mv_error_unmatched(int objcode, char* command) {
	char* meaning = NULL;

	switch (objcode) {
	case MVAST_TEMPAPOSTROPHE:
		meaning = "\"'\"";
		break;
	case MVAST_TEMPOPENBRACE:
		meaning = "'{'";
		break;
	}

	if (meaning != NULL) {
		THROW(SYNTAX, "Unmatched %s in \"%s\"", meaning, command);
	}

	THROW(SYNTAX, "Unmatched temporary objects of type %d in %s",
		objcode, command);
}

