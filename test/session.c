
#include <test.h>

inline static void __perform__(mv_session* state, char* cmd) {
	mv_command action;
	FAILFAST(mv_command_parse(&action, cmd));
	FAILFAST(mv_session_execute(state, &action));
	mv_command_release(&action);
}

TEST session_init() {
	mv_session state;
	state.clsnames.used = 12345;
	mv_session_init(&state);
	ASSERT_INT(state.clsnames.used, 0);
	mv_session_release(&state);
}

TEST execute_REQ1() {
	mv_session state;
	mv_session_init(&state);
	__perform__(&state, REQ1);
	ASSERT_INT(state.vars.used, 1);
	ASSERT_INT(state.entities.used, 1);
	
	mv_attrlist attrs = state.entities.items[0].data;
	ASSERT_INT(attrs.size, 1);
	ASSERT_INT(attrs.attrs[0].type, MVTYPE_STRING);
	ASSERT_STRING(attrs.attrs[0].name, "name");
	ASSERT_STRING(attrs.attrs[0].value.string, "Umberto Eco");
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

TEST execute_REQ12() {
	mv_command action;
	mv_session state;

	__prepare_for_REQ10_11(&state, 0);

	FAILFAST(mv_command_parse(&action, REQ12));
	FAILFAST(mv_session_execute(&state, &action));

	mv_session_release(&state);
	mv_command_release(&action);
}

TEST execute_REQ14() {
	mv_session state;
	mv_session_init(&state);
	__perform__(&state, REQ14);
	
	ASSERT_INT(state.vars.used, 1);
	ASSERT_INT(state.entities.used, 1);
	
	mv_attrlist attrs = state.entities.items[0].data;
	ASSERT_INT(attrs.size, 1);
	ASSERT_INT(attrs.attrs[0].type, MVTYPE_INTEGER);
	ASSERT_STRING(attrs.attrs[0].name, "height");
	ASSERT_INT(attrs.attrs[0].value.integer, 324);
	mv_session_release(&state);
}

TEST lookup_after_destroy() {
	mv_command lookup, destroy; 
	mv_session state;
	mv_intset result;

	__prepare_for_REQ10_11(&state, 1);

	mv_intset_alloc(&result, 8);

	FAILFAST(mv_command_parse(&lookup, REQ11));
	FAILFAST(mv_command_parse(&destroy, REQ12));
	FAILFAST(mv_session_lookup(&result, &state, &lookup));

	ASSERT_INT(result.used, 1);
	ASSERT_INT(result.items[0], 0);
	result.used = 0;

	FAILFAST(mv_session_execute(&state, &destroy));
	FAILFAST(mv_session_lookup(&result, &state, &lookup));

	ASSERT_INT(result.used, 0);

	mv_intset_release(&result);
	mv_command_release(&lookup);
	mv_command_release(&destroy);
	mv_session_release(&state);
}

TEST lookup_all_items() {
	mv_command lookup;
	mv_session state;
	mv_intset result;

	__prepare_for_REQ10_11(&state, 1);

	mv_intset_alloc(&result, 8);

	FAILFAST(mv_command_parse(&lookup, REQ13));
	FAILFAST(mv_session_lookup(&result, &state, &lookup));

	ASSERT_INT(result.used, 1);
	ASSERT_INT(result.items[0], 0);
	
	mv_intset_release(&result);
	mv_command_release(&lookup);
	mv_session_release(&state);
}

TEST numlookup() {
	mv_session session;
	mv_session_init(&session);
	mv_strarr script;
	mv_strarr_alloc(&script, 3);
	mv_strarr_append(&script, REQ14);
	mv_strarr_append(&script, REQ15);
	mv_strarr_append(&script, REQ16);
	FAILFAST(mv_session_perform(&session, &script));
	mv_strarr_release(&script);

	mv_intset result;
	mv_intset_alloc(&result, 8);

	mv_command lookup;
	FAILFAST(mv_command_parse(&lookup, REQ17));
	FAILFAST(mv_session_lookup(&result, &session, &lookup));

	ASSERT_INT(result.used, 1);
	ASSERT_INT(result.items[0], 0);

	mv_intset_release(&result);
	mv_command_release(&lookup);
	mv_session_release(&session);
}

TEST subquery() {
	mv_session session;
	mv_session_init(&session);
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, REQ19);
	mv_strarr_append(&script, REQ18);
	FAILFAST(mv_session_perform(&session, &script));
	ASSERT_INT(session.classes.used, 2);
	ASSERT_INT(session.classes.items[1].data.size, 1);

	mv_attrspec spc = session.classes.items[1].data.specs[0];
	ASSERT_INT(spc.type, MVSPEC_SUBQUERY);
	mv_query qr = spc.value.subquery;
	ASSERT_STRING(qr.classname, "book");
	
	mv_strarr_release(&script);
	mv_session_release(&session);
}

TEST update() {
	mv_session session;
	mv_session_init(&session);
	mv_strarr script;
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, REQ14);
	mv_strarr_append(&script, REQ20);
	FAILFAST(mv_session_perform(&session, &script));

	ASSERT_INT(session.entities.used, 1);
	mv_attrlist data = session.entities.items[0].data;
	ASSERT_INT(data.size, 2);

	mv_strarr_release(&script);
	mv_session_release(&session);
}

