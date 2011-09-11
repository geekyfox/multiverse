
#include "test.h"

TEST validity_assign() {
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, strdup(REQ23));
	mv_strarr_append(&script, strdup(REQ6));

	mvSession session;
	mv_command cmd;
	try
	{
		session.perform(&script);
		mv_command_parse(cmd, REQ24);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	try
	{
		session.execute(cmd);
	}
	catch (mv_error* err)
	{
		ASSERT_ERROR(err, MVERROR_INVALID);
		mv_error_release(err);
	}

	mv_strarr_release(&script);
}

