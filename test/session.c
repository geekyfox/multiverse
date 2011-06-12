
#include <test.h>

TEST session_init() {
	mv_session state;
	state.clsnames.used = 12345;
	mv_session_init(&state);
	ASSERT_INT(state.clsnames.used, 0);
	mv_session_release(&state);
}

TEST execute_REQ1() {
	mv_command action;
	mv_session state;
	mv_session_init(&state);
	FAILFAST(mv_command_parse(&action, REQ1));
	FAILFAST(mv_session_execute(&state, &action));
	ASSERT_INT(state.vars.used, 1);
	ASSERT_INT(state.entities.used, 1);
	
	mv_attrlist attrs = state.entities.items[0].data;
	ASSERT_INT(attrs.size, 1);
	ASSERT_INT(attrs.attrs[0].type, MVTYPE_STRING);
	ASSERT_STRING(attrs.attrs[0].name, "name");
	ASSERT_STRING(attrs.attrs[0].value.string, "Umberto Eco");
	mv_command_release(&action);
	mv_session_release(&state);
}

static void __prepare_for_REQ10_11(mv_session* session, int bind) {
	mv_session_init(session);
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, REQ1);
	mv_strarr_append(&script, REQ6);
	if (bind) mv_strarr_append(&script, REQ10);
	FAILFAST(mv_session_perform(session, &script));
	mv_strarr_release(&script);
}

TEST execute_REQ10() {
	mv_command action;
	mv_session state;
	__prepare_for_REQ10_11(&state, 0);

	FAILFAST(mv_command_parse(&action, REQ10));
	FAILFAST(mv_session_execute(&state, &action));

	ASSERT_INT(state.entities.items[0].classes.used, 1);
	ASSERT_STRING(state.entities.items[0].classes.items[0], "person");

	mv_session_release(&state);
	mv_command_release(&action);
}

TEST fail_REQ10() {
	mv_command action;
	mv_session state;
	mv_session_init(&state);

	FAILFAST(mv_command_parse(&action, REQ10));
	mv_error* err = mv_session_execute(&state, &action);
	ASSERT_INT(err->code, MVERROR_BADVAR);

	mv_error_release(err);
	mv_session_release(&state);
	mv_command_release(&action);
}

TEST execute_REQ11() {
	mv_command action;
	mv_session state;
	mv_intset result;

	__prepare_for_REQ10_11(&state, 1);

	mv_intset_alloc(&result, 8);

	FAILFAST(mv_command_parse(&action, REQ11));
	FAILFAST(mv_session_lookup(&result, &state, &action));

	ASSERT_INT(result.used, 1);
	ASSERT_INT(result.items[0], 0);

	mv_intset_release(&result);
	mv_session_release(&state);
	mv_command_release(&action);
}

