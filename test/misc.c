
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mvParser.h"
#include "test.h"

char* response_1 = "umberto_eco = entity {\n  name = 'Umberto Eco'\n}\n";

TEST mv_command_test() {
	mv_command action;

	try
	{
		singletonParser.parse(action, "quit");
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	assert(action.code == QUIT);
	assert(action.attrs.size() == 0);
	assert(action.vars.size() == 0);

	try
	{
		singletonParser.parse(action, REQ3);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	assert(action.code == SHOW);
	assert(action.attrs.size() == 0);
	assert(action.vars.size() == 1);
	ASSERT_STRREF(action.vars[0], "umberto_eco");

	try
	{
		singletonParser.parse(action, REQ5);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	assert(action.code == CREATE_ENTITY);
	assert(action.attrs.size() == 2);
	assert(strcmp(action.attrs[1].name, "author") == 0);
	assert(action.attrs[1].type == RAWREF);
	assert(strcmp(action.attrs[1].value.rawref, "umberto_eco") == 0);
}

TEST mv_execute_test() {
	mv_command action;
	mvSession state;

	try
	{
		singletonParser.parse(action, REQ1);
		assert(action.inited);
		state.execute(action);

		singletonParser.parse(action, REQ5);
		assert(action.inited);
		state.execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	ASSERT_INT(state.varcount(), 2);
	assert(state.entities.size() == 2);
	assert(state.entities[1].data.size() == 2);
	assert(state.entities[1].data[1].type == REF);
	assert(state.entities[1].data[1].value.ref == 0);

	ASSERT_INT(state.classes.size(), 0);
	ASSERT_INT(state.clscount(), 0);
	try
	{
		singletonParser.parse(action, REQ6);
		assert(action.inited);
		state.execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	ASSERT_INT(state.classes.size(), 1);
	ASSERT_INT(state.clscount(), 1);
	ASSERT_INT(state.classes[0].data.size(), 1);
	ASSERT_INT(state.classes[0].data[0].get_type(), TYPE);

	try
	{
		singletonParser.parse(action, REQ8);
		assert(action.inited);
		state.execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
}

TEST mv_attrlist_show_test() {
	mv_attrlist attrs;
	attrs.alloc(1);
	singletonParser.parse(attrs[0], "name", "'Umberto Eco");
	assert(attrs[0].type == STRING);
	assert(strcmp(attrs[0].name, "name") == 0);
	assert(strcmp(attrs[0].value.string, "Umberto Eco") == 0);
}

TEST mv_session_findvar_test() {
	mvSession state;
	int ref = state.findvar("foobar");
	assert(ref == -1);
	ref = state.findvar("##0");
	assert(ref == -1);
	mv_command action;
	try
	{
		singletonParser.parse(action, REQ1);
		state.execute(action);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	ref = state.findvar("##0");
	assert(ref == 0);
	ref = state.findvar("umberto_eco");
	assert(ref == 0);
	char* text1;
	try
	{
		text1 = state.show("umberto_eco");
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	ASSERT_STRING(text1, response_1);
	free(text1);

	try
	{
		text1 = state.show("##1");
		DIE("Error expected");
	}
	catch (mv_error* err)
	{
		mv_error_release(err);
	}
}

TEST mv_attrspec_release_test() {
	mv_command cmd;
	mv_error* error;

	try
	{
		singletonParser.parse(cmd, REQ7);
		singletonParser.parse(cmd, "show person");
		ASSERT_INT(cmd.attrs.size(), 0);
		ASSERT_INT(cmd.spec.size(), 0);
	}
	catch (mv_error* err)
	{
		FAIL(error);
	}
}

