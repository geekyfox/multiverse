
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

char* response_1 = "umberto_eco = entity {\n  name = 'Umberto Eco'\n}\n";

TEST mv_command_test() {
	mv_command action;

	try
	{
		mv_command_parse(action, "quit");
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	assert(action.code == QUIT);
	assert(action.attrs.size == 0);
	assert(action.vars.size == 0);

	try
	{
		mv_command_parse(action, REQ3);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	assert(action.code == SHOW);
	assert(action.attrs.size == 0);
	assert(action.vars.used == 1);
	ASSERT_STRREF(action.vars.items[0], "umberto_eco");

	try
	{
		mv_command_parse(action, REQ5);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	assert(action.code == CREATE_ENTITY);
	assert(action.attrs.size == 2);
	assert(strcmp(action.attrs.attrs[1].name, "author") == 0);
	assert(action.attrs.attrs[1].type == MVTYPE_RAWREF);
	assert(strcmp(action.attrs.attrs[1].value.rawref, "umberto_eco") == 0);
}

TEST mv_execute_test() {
	mv_command action;
	mv_session state;

	try
	{
		mv_command_parse(action, REQ1);
		assert(action.inited);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}

	FAILFAST(state.execute(&action));

	try
	{
		mv_command_parse(action, REQ5);
		assert(action.inited);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.execute(&action));
	ASSERT_INT(state.varcount(), 2);
	assert(state.entities.size() == 2);
	assert(state.entities[1].data.size == 2);
	assert(state.entities[1].data.attrs[1].type == MVTYPE_REF);
	assert(state.entities[1].data.attrs[1].value.ref == 0);

	ASSERT_INT(state.classes.used, 0);
	ASSERT_INT(state.clscount(), 0);
	try
	{
		mv_command_parse(action, REQ6);
		assert(action.inited);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.execute(&action));
	ASSERT_INT(state.classes.used, 1);
	ASSERT_INT(state.clscount(), 1);
	ASSERT_INT(state.classes.items[0].data.size, 1);
	ASSERT_INT(state.classes.items[0].data.specs[0].type, MVSPEC_TYPE);

	try
	{
		mv_command_parse(action, REQ8);
		assert(action.inited);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.execute(&action));
}

TEST mv_attrlist_show_test() {
	mv_attrlist attrs;
	mv_attrlist_alloc(&attrs, 1);
	mv_attr_parse(attrs.attrs, "name", "'Umberto Eco");
	assert(attrs.attrs[0].type == MVTYPE_STRING);
	assert(strcmp(attrs.attrs[0].name, "name") == 0);
	assert(strcmp(attrs.attrs[0].value.string, "Umberto Eco") == 0);

	mv_attrlist_release(&attrs);
}

TEST mv_session_findvar_test() {
	mv_session state;
	int ref = state.findvar("foobar");
	assert(ref == -1);
	ref = state.findvar("##0");
	assert(ref == -1);
	mv_command action;
	try
	{
		mv_command_parse(action, REQ1);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(state.execute(&action));
	ref = state.findvar("##0");
	assert(ref == 0);
	ref = state.findvar("umberto_eco");
	assert(ref == 0);
	char* text1;
	try
	{
		state.show(&text1, "umberto_eco");
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	ASSERT_STRING(text1, response_1);
	free(text1);

	try
	{
		state.show(&text1, "##1");
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
		mv_command_parse(cmd, REQ7);
		mv_command_parse(cmd, "show person");
		ASSERT_NULL(cmd.attrs.attrs);
		ASSERT_INT(cmd.attrs.size, 0);
		ASSERT_NULL(cmd.spec.specs);
		ASSERT_INT(cmd.spec.size, 0);
	}
	catch (mv_error* err)
	{
		FAIL(error);
	}
}

