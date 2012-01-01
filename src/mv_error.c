
#include <stdio.h>
#include "error.h"
#include "parser.h"

inline static char* __prefix__(int code) {
	switch(code) {
	case MVERROR_BADCMD:   return "Bad command";
	case MVERROR_BADVAR:   return "Bad variable";
	case MVERROR_INTERNAL: return "Internal error";
	case MVERROR_SYNTAX:   return "Syntax error";
	}
	DIE("Unknown error code (%d)\n", code);
}

void mvError_display(mvError* error, FILE* dst) {
	char* prefix = __prefix__(error->code);
	fprintf(dst, "%s: %s\n", prefix, error->message);
	fprintf(dst, "at %s\n", error->location);
	mvError_release(error);
}

char* mvError_show(mvError* error) {
	char *result, *prefix = __prefix__(error->code);
	asprintf(&result, "%s: %s\n", prefix, error->message);
	return result;
}

void mvError_release(mvError* error) {
	free(error->message);
	free(error->location);
	free(error);
}

mvError* mvError_unmatched(int objcode, const char* command) {
	char* meaning = NULL;

	switch (objcode) {
	case MVAST_TEMPAPOSTROPHE:
		meaning = "\"'\"";
		break;
	case OpenBrace:
		meaning = "'{'";
		break;
	}

	if (meaning != NULL) {
		THROW(SYNTAX, "Unmatched %s in \"%s\"", meaning, command);
	}

	THROW(SYNTAX, "Unmatched temporary objects of type %d in %s",
		objcode, command);
}

