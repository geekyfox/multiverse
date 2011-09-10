
#include "test.h"

TEST validity_assign() {
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, strdup(REQ23));
	mv_strarr_append(&script, strdup(REQ6));

	mv_session session;
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

	mv_error* err = session.execute(&cmd);
	ASSERT_ERROR(err, MVERROR_INVALID);

	mv_error_release(err);
	mv_strarr_release(&script);
}

