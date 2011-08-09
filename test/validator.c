
#include "test.h"

TEST validity_assign() {
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, REQ23);
	mv_strarr_append(&script, REQ6);

	mv_session session;
	mv_session_init(&session);
	FAILFAST(mv_session_perform(&session, &script));
	
	mv_command cmd;
	mv_command_parse(&cmd, REQ24);

	mv_error* err = mv_session_execute(&session, &cmd);
	ASSERT_ERROR(err, MVERROR_INVALID);

	mv_error_release(err);
	mv_command_release(&cmd);
	mv_strarr_release(&script);
	mv_session_release(&session);
}

