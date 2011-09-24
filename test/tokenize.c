
#include <stdarg.h>
#include "test.h"

#define BEFORE(RQ) try { mvTokenizer tokens(RQ); 

#define AFTER } catch (mv_error* err) { FAIL(err); }

#define BEFOREBAD(RQ) mv_error* error; \
try { mvTokenizer tokens(RQ); DIE("Error expected"); }\
catch (mv_error* err) {error = err;}

#define AFTERBAD mv_error_release(error);

inline static void __validate__(mv_strarr& tokens, int ct, ...) {
	ASSERT_INT(tokens.size(), ct);
	va_list args;
	va_start(args, ct);
	int i;
	for (i=0; i<ct; i++) {
		fflush(stdout);
		char* expect = va_arg(args, char*);
		ASSERT_STRREF(tokens[i], expect);
	}
}

TESTREQ 1 {
	__validate__(tokens, 8,
		"create", "entity", "{", "name", "=", "'Umberto Eco", "}",
        "umberto_eco");
}

TESTREQ 2 {
	__validate__(tokens, 12,
		"create", "entity", "{", "country", "=", "italy", ",", "name",
        "=", "'Umberto Eco", "}", "umberto_eco");
}

TESTREQ 7 {
	__validate__(tokens, 8,
		"create", "class", "person", "{", "name", ":", "string", "}");
}

TESTREQ 18 {
	__validate__(tokens, 16,
		"create", "class", "writer", "{", "books", "=", "[", "book",
	    "with", "{", "author", "=", "$$", "}", "]", "}");
}

TESTBADREQ 1 {
	ASSERT_INT(error->code, MVERROR_SYNTAX);
}

