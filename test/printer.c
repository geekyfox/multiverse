
#include "test.h"

TEST error_unmatched() {
	try {
		mv_ast ast(BADREQ1);
		DIE("Error expected");
	} catch (mv_error* error) {
		ASSERT_STRING(error->message, ERRRESP1);
		mv_error_release(error);
	}
}

static void __showcmdimpl(mv_strarr& script, char* ref, char* expect) {
	mvSession state;
	char* target;

	try
	{
		state.perform(script);
		target = state.show(ref);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	ASSERT_STRING(target, expect);

	free(target);
}

TEST showcmd1() {
	mv_strarr script(2);
	script.append(REQ1);
	script.append(REQ5);
	printf("FOO\n"); fflush(stdout);

	__showcmdimpl(script, "name_of_the_rose", RESP2);
	printf("BAR\n"); fflush(stdout);
}

TEST showcmd2() {
	mv_strarr script(1);
	script.append(REQ7);

	__showcmdimpl(script, "person", RESP3);
}

TEST showcmd3() {
	mv_strarr script(1);
	script.append(REQ18);

	__showcmdimpl(script, "writer", RESP4);
}

