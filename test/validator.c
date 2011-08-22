
#include "test.h"

TEST validity_assign() {
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, strdup(REQ23));
	mv_strarr_append(&script, strdup(REQ6));

	mv_session session;
	FAILFAST(mv_session_perform(&session, &script));
	
	mv_command cmd;
	mv_command_parse(&cmd, REQ24);

	mv_error* err = session.execute(&cmd);
	ASSERT_ERROR(err, MVERROR_INVALID);

	mv_error_release(err);
	mv_command_release(&cmd);
	mv_strarr_release(&script);
}

