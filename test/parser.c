
#include "test.h"

static void __tokenizeimpl(char* request, char** expect, int count) {
	mv_strarr tokens(10);
	mv_error* error = mv_tokenize(&tokens, request);
	FAIL(error);
	ASSERT_INT(tokens.size(), count);
	int i;
	for (i=0; i<count; i++) ASSERT_STRREF(tokens[i], expect[i]);
}

TEST attr_test1() {
	mv_attr pair;
	mv_attr_parse(&pair, "name", "'Umberto Eco");
	ASSERT_INT(pair.type, STRING);
	ASSERT_STRING(pair.name, "name");
	ASSERT_STRING(pair.value.string, "Umberto Eco");
	mv_attr_release(&pair);
}

TEST tokenize_fails() {
	mv_strarr tokens(10);
	mv_error* error = mv_tokenize(&tokens, BADREQ1);
	ASSERT_NOTNULL(error);
	ASSERT_INT(error->code, MVERROR_SYNTAX);
	mv_error_release(error);
}

TEST tokenize_test() {
	mv_strarr tokens(5);
	mv_error* error = mv_tokenize(&tokens, "\nquit");
	FAIL(error);
	ASSERT_INT(tokens.size(), 1);
}

static void __astparse_fail(char* request) {
	try {
		mv_ast ast(request);
		DIE("Error expected");
	} catch (mv_error* error) {
		ASSERT_NOTNULL(error);
		ASSERT_INT(error->code, MVERROR_SYNTAX);
		mv_error_release(error);
	}
}

TEST astparse_failures() {
	__astparse_fail(BADREQ2);
}

static void __cmdparse_fail(char* request) {
	mv_command action;
	try
	{
		mv_command_parse(action, request);
		DIE("Parsing not failed '%s'", request);
	}
	catch (mv_error* error)
	{
		if (error->code != MVERROR_SYNTAX)
		{
			mv_error_display(error, stderr);
			DIE("Error is not syntactic");
		}
		mv_error_release(error);
	}
}

TEST cmdparse_failures() {
	__cmdparse_fail(BADREQ1);
	__cmdparse_fail(BADREQ2);
	__cmdparse_fail(BADREQ3);
}

