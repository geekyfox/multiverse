
#include "mvParser.h"
#include "test.h"

static void __tokenizeimpl(char* request, char** expect, int count) {
	try
	{
		mvTokenizer tokens(request);
		ASSERT_INT(tokens.size(), count);
		for (int i=0; i<count; i++) ASSERT_STRREF(tokens[i], expect[i]);
	}
	catch (mvError* err)
	{ 
		FAIL(err);
	}
}

TEST attr_test1() {
	mvAttr pair;
	singletonParser.parse(pair, "name", "'Umberto Eco");
	ASSERT_INT(pair.type, STRING);
	ASSERT_STRREF(pair.name, "name");
	ASSERT_STRING(pair.value.string, "Umberto Eco");
}

TEST tokenize_fails() {
	try
	{
		mvTokenizer tokens(BADREQ1);
	}
	catch (mvError* error)
	{
		ASSERT_NOTNULL(error);
		ASSERT_INT(error->code, MVERROR_SYNTAX);
		mvError_release(error);
	}
}

TEST tokenize_test() {
	try
	{
		mvTokenizer tokens("\nquit");
		ASSERT_INT(tokens.size(), 1);
	}
	catch (mvError* error)
	{
		FAIL(error);
	}
}

static void __astparse_fail(const char* request) {
	try {
		mvAst ast(request);
		DIE("Error expected");
	} catch (mvError* error) {
		ASSERT_NOTNULL(error);
		ASSERT_INT(error->code, MVERROR_SYNTAX);
		mvError_release(error);
	}
}

TEST astparse_failures() {
	__astparse_fail(BADREQ2);
}

static void __cmdparse_fail(const char* request) {
	mvCommand action;
	try
	{
		singletonParser.parse(action, request);
		DIE("Parsing not failed '%s'", request);
	}
	catch (mvError* error)
	{
		if (error->code != MVERROR_SYNTAX)
		{
			mvError_display(error, stderr);
			DIE("Error is not syntactic");
		}
		mvError_release(error);
	}
}

TEST cmdparse_failures() {
	__cmdparse_fail(BADREQ1);
	__cmdparse_fail(BADREQ2);
	__cmdparse_fail(BADREQ3);
}

