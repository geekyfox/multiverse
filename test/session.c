
#include <test.h>

inline static void __perform__(mvSession* state, const char* cmd) {
	mv_command action;
	try
	{
		mv_command_parse(action, cmd);
		state->execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
}

TEST execute_REQ1() {
	mvSession state;
	__perform__(&state, REQ1);
	ASSERT_INT(state.varcount(), 1);
	ASSERT_INT(state.entities.size(), 1);
	
	mv_attrlist& attrs = state.entities[0].data;
	ASSERT_INT(attrs.size(), 1);
	ASSERT_INT(attrs[0].type, STRING);
	ASSERT_STRING(attrs[0].name, "name");
	ASSERT_STRING(attrs[0].value.string, "Umberto Eco");
}

static void __prepare_for_REQ10_11(mvSession* session, int bind) {
	mv_strarr script(2);
	script.append(REQ1);
	script.append(REQ6);
	if (bind) script.append(REQ10);
	try
	{
		session->perform(script);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}	
}

TEST execute_REQ10() {
	mv_command action;
	mvSession state;
	__prepare_for_REQ10_11(&state, 0);

	try
	{
		mv_command_parse(action, REQ10);
		state.execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	ASSERT_INT(state.entities[0].classes.size(), 1);
	ASSERT_STRREF(state.entities[0].classes[0], "person");

}

TEST fail_REQ10() {
	mv_command action;
	mvSession state;

	try
	{
		mv_command_parse(action, REQ10);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	try
	{
		state.execute(action);
		DIE("Error expected");
	}
	catch (mv_error* err)
	{
		ASSERT_INT(err->code, MVERROR_BADVAR);
		mv_error_release(err);
	}
}

TEST execute_REQ11() {
	mv_command action;
	mvSession state;
	mvIntset result(8);

	__prepare_for_REQ10_11(&state, 1);

	try
	{
		mv_command_parse(action, REQ11);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, action));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.get(0), 0);
}

TEST execute_REQ12() {
	mv_command action;
	mvSession state;

	__prepare_for_REQ10_11(&state, 0);

	try
	{
		mv_command_parse(action, REQ12);
		state.execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
}

TEST execute_REQ14() {
	mvSession state;
	__perform__(&state, REQ14);
	
	ASSERT_INT(state.varcount(), 1);
	ASSERT_INT(state.entities.size(), 1);
	
	mv_attrlist& attrs = state.entities[0].data;
	ASSERT_INT(attrs.size(), 1);
	ASSERT_INT(attrs[0].type, INTEGER);
	ASSERT_STRING(attrs[0].name, "height");
	ASSERT_INT(attrs[0].value.integer, 324);
}

TEST lookup_after_destroy() {
	mv_command lookup, destroy; 
	mvSession state;
	mvIntset result(8);

	__prepare_for_REQ10_11(&state, 1);

	try
	{
		mv_command_parse(lookup, REQ11);
		mv_command_parse(destroy, REQ12);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.get(0), 0);
	result.clear();

	try
	{
		state.execute(destroy);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 0);
}

TEST lookup_all_items() {
	mv_command lookup;
	mvSession state;
	mvIntset result(8);

	__prepare_for_REQ10_11(&state, 1);

	try
	{
		mv_command_parse(lookup, REQ13);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.contains(0), 1);
}

TEST numlookup() {
	mvSession session;
	mv_strarr script(3);
	script.append(REQ14);
	script.append(REQ15);
	script.append(REQ16);
	try
	{
		session.perform(script);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	mvIntset result(8);

	mv_command lookup;
	try
	{
		mv_command_parse(lookup, REQ17);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(session.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.contains(0), 1);
}

TEST subquery() {
	mvSession session;
	mv_strarr script(2);
	script.append(REQ19);
	script.append(REQ18);
	try
	{
		session.perform(script);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	ASSERT_INT(session.classes.size(), 2);
	ASSERT_INT(session.classes[1].data.size(), 1);

	mv_attrspec& spc = session.classes[1].data[0];
	ASSERT_INT(spc.get_type(), SUBQUERY);
	const mvQuery& qr = spc.subquery();
	ASSERT_STRING(qr.classname, "book");
}

TEST update() {
	mvSession session;
	mv_strarr script(2);
	script.append(REQ14);
	script.append(REQ20);
	try
	{
		session.perform(script);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	ASSERT_INT(session.entities.size(), 1);
	mv_attrlist& data = session.entities[0].data;
	ASSERT_INT(data.size(), 2);
}

TEST badupdate() {
	mvSession session;
	mv_strarr script(2);
	script.append(REQ23);
	script.append(BADREQ4);
	try
	{
		session.perform(script);
		ASSERT_ERROR(((mv_error*)NULL), MVERROR_SYNTAX);
	}
	catch (mv_error* err)
	{
		ASSERT_ERROR(err, MVERROR_SYNTAX);
		mv_error_release(err);
	}
}


