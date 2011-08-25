
#include "test.h"

TEST error_unmatched() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(ast, BADREQ1);
	ASSERT_STRING(error->message, ERRRESP1);
	mv_error_release(error);
}

static void __showcmdimpl(mv_strarr* script, char* ref, char* expect) {
	mv_session state;
	char* target;
	mv_error* error;

	error = mv_session_perform(&state, script);
	FAIL(error);

	error = mv_session_show(&target, &state, ref);
	FAIL(error);
	ASSERT_STRING(target, expect);

	mv_strarr_release(script);
	free(target);
}

TEST showcmd1() {
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, strdup(REQ1));
	mv_strarr_append(&script, strdup(REQ5));

	__showcmdimpl(&script, "name_of_the_rose", RESP2);
}

TEST showcmd2() {
	mv_strarr script;

	mv_strarr_alloc(&script, 1);
	mv_strarr_append(&script, strdup(REQ7));

	__showcmdimpl(&script, "person", RESP3);
}

TEST showcmd3() {
	mv_strarr script;
	mv_strarr_alloc(&script, 1);
	mv_strarr_append(&script, strdup(REQ18));

	__showcmdimpl(&script, "writer", RESP4);
}

