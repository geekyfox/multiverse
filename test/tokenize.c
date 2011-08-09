
#include <stdarg.h>
#include "test.h"

#define BEFORE(RQ) \
mv_strarr tokens; \
FAILFAST(mv_tokenize(&tokens, RQ)); \
ASSERT_NOTNULL(tokens.items)

#define AFTER mv_strarr_release(&tokens)

#define BEFOREBAD(RQ) \
mv_strarr tokens; \
mv_error* error = mv_tokenize(&tokens, RQ); \
ASSERT_NOTNULL(error)

#define AFTERBAD mv_error_release(error);

inline static void __validate__(mv_strarr* tokens, int ct, ...) {
	ASSERT_INT(tokens->size, ct);
	ASSERT_INT(tokens->used, ct);
	va_list args;
	va_start(args, ct);
	int i;
	for (i=0; i<ct; i++) {
		ASSERT_STRING(tokens->items[i], va_arg(args, char*));
	}
}

TESTREQ 1 {
	__validate__(&tokens, 8,
		"create", "entity", "{", "name", "=", "'Umberto Eco", "}",
        "umberto_eco");
}

TESTREQ 2 {
	__validate__(&tokens, 12,
		"create", "entity", "{", "country", "=", "italy", ",", "name",
        "=", "'Umberto Eco", "}", "umberto_eco");
}

TESTREQ 7 {
	__validate__(&tokens, 8,
		"create", "class", "person", "{", "name", ":", "string", "}");
}

TESTREQ 18 {
	__validate__(&tokens, 16,
		"create", "class", "writer", "{", "books", "=", "[", "book",
	    "with", "{", "author", "=", "$$", "}", "]", "}");
}

TESTBADREQ 1 {
	ASSERT_INT(error->code, MVERROR_SYNTAX);
}

