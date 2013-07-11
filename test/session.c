
#include <test.h>
#include "mvParser.h"

inline static void __perform__(mvSession* state, const char* cmd) {
	mvCommand action;
	try
	{
		singletonParser.parse(action, cmd);
		state->execute(action);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
}

TEST execute_REQ1() {
	mvSession state;
	__perform__(&state, REQ1);
	ASSERT_INT(state.varcount(), 1);
	ASSERT_INT(state.entities.size(), 1);
	
	mvAttrlist& attrs = state.entities[0].data;
	ASSERT_INT(attrs.size(), 1);
	ASSERT_INT(attrs[0].type, STRING);
	ASSERT_STRREF(attrs[0].name, "name");
	ASSERT_STRING(attrs[0].value.string, "Umberto Eco");
}

static void __prepare_for_REQ10_11(mvSession& session, int bind) {
	mvStrArray script(2);
	script.append(REQ1);
	script.append(REQ6);
	if (bind) script.append(REQ10);
	try
	{
		session.perform(script);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}	
}

TEST execute_REQ10() {
	mvCommand action;
	mvSession state;
	__prepare_for_REQ10_11(state, 0);

	try
	{
		singletonParser.parse(action, REQ10);
		state.execute(action);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}

	ASSERT_INT(state.entities[0].classes.size(), 1);
	ASSERT_STRREF(state.entities[0].classes[0], "person");

}

TEST fail_REQ10() {
	mvCommand action;
	mvSession state;

	try
	{
		singletonParser.parse(action, REQ10);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}

	try
	{
		state.execute(action);
		DIE("Error expected");
	}
	catch (mvError* err)
	{
		ASSERT_INT(err->code, MVERROR_BADVAR);
		mvError_release(err);
	}
}

TEST execute_REQ11() {
	mvCommand action;
	mvSession state;
	mvIntset result(8);

	__prepare_for_REQ10_11(state, 1);

	try
	{
		singletonParser.parse(action, REQ11);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, action));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.get(0), 0);
}

TEST execute_REQ12() {
	mvCommand action;
	mvSession state;

	__prepare_for_REQ10_11(state, 0);

	try
	{
		singletonParser.parse(action, REQ12);
		state.execute(action);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
}

TEST execute_REQ14() {
	mvSession state;
	__perform__(&state, REQ14);
	
	ASSERT_INT(state.varcount(), 1);
	ASSERT_INT(state.entities.size(), 1);
	
	mvAttrlist& attrs = state.entities[0].data;
	ASSERT_INT(attrs.size(), 1);
	ASSERT_INT(attrs[0].type, INTEGER);
	ASSERT_STRREF(attrs[0].name, "height");
	ASSERT_INT(attrs[0].value.integer, 324);
}

TEST lookup_after_destroy() {
	mvCommand lookup, destroy; 
	mvSession state;
	mvIntset result(8);

	__prepare_for_REQ10_11(state, 1);

	try
	{
		singletonParser.parse(lookup, REQ11);
		singletonParser.parse(destroy, REQ12);
	}
	catch (mvError* err)
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
	catch (mvError* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 0);
}

TEST lookup_all_items() {
	mvCommand lookup;
	mvSession state;
	mvIntset result(8);

	__prepare_for_REQ10_11(state, 1);

	try
	{
		singletonParser.parse(lookup, REQ13);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
	FAILFAST(state.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.contains(0), 1);
}

TEST numlookup() {
	mvSession session;
	mvStrArray script(3);
	script.append(REQ14);
	script.append(REQ15);
	script.append(REQ16);
	try
	{
		session.perform(script);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}

	mvIntset result(8);

	mvCommand lookup;
	try
	{
		singletonParser.parse(lookup, REQ17);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
	FAILFAST(session.lookup(result, lookup));

	ASSERT_INT(result.cardinality(), 1);
	ASSERT_INT(result.contains(0), 1);
}

TEST subquery() {
	mvSession session;
	mvStrArray script(2);
	script.append(REQ19);
	script.append(REQ18);
	try
	{
		session.perform(script);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
	ASSERT_INT(session.classes.size(), 2);
	ASSERT_INT(session.classes[1].data.size(), 1);

	mvAttrSpec& spc = session.classes[1].data[0];
	ASSERT_INT(spc.get_type(), SUBQUERY);
	const mvQuery& qr = spc.subquery();
	ASSERT_STRREF(qr.classname, "book");
}

TEST update() {
	mvSession session;
	mvStrArray script(2);
	script.append(REQ14);
	script.append(REQ20);
	try
	{
		session.perform(script);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}

	ASSERT_INT(session.entities.size(), 1);
	mvAttrlist& data = session.entities[0].data;
	ASSERT_INT(data.size(), 2);
}

TEST badupdate() {
	mvSession session;
	mvStrArray script(2);
	script.append(REQ23);
	script.append(BADREQ4);
	try
	{
		session.perform(script);
		ASSERT_ERROR(((mvError*)NULL), MVERROR_SYNTAX);
	}
	catch (mvError* err)
	{
		ASSERT_ERROR(err, MVERROR_SYNTAX);
		mvError_release(err);
	}
}


