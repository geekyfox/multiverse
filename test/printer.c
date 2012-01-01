
#include "test.h"

TEST error_unmatched() {
	try {
		mvAst ast(BADREQ1);
		DIE("Error expected");
	} catch (mvError* error) {
		ASSERT_STRING(error->message, ERRRESP1);
		mvError_release(error);
	}
}

static void __showcmdimpl(mvStrArray& script, char* ref, char* expect) {
	mvSession state;
	char* target;

	try
	{
		state.perform(script);
		target = state.show(ref);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}

	ASSERT_STRING(target, expect);

	free(target);
}

TEST showcmd1() {
	mvStrArray script(2);
	script.append(REQ1);
	script.append(REQ5);
	printf("FOO\n"); fflush(stdout);

	__showcmdimpl(script, "name_of_the_rose", RESP2);
	printf("BAR\n"); fflush(stdout);
}

TEST showcmd2() {
	mvStrArray script(1);
	script.append(REQ7);

	__showcmdimpl(script, "person", RESP3);
}

TEST showcmd3() {
	mvStrArray script(1);
	script.append(REQ18);

	__showcmdimpl(script, "writer", RESP4);
}

