
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

char* response_1 = "umberto_eco = entity {\n  name = 'Umberto Eco'\n}\n";

TEST mv_command_test() {
	mv_command action;

	mv_error* error = mv_command_parse(&action, "quit");
	assert(error == NULL);
	assert(action.code == MVCMD_QUIT);
	assert(action.attrs.size == 0);
	assert(action.vars.size == 0);

	error = mv_command_parse(&action, REQ3);
	assert(error == NULL);
	assert(action.code == MVCMD_SHOW);
	assert(action.attrs.size == 0);
	assert(action.vars.used == 1);
	ASSERT_STRREF(action.vars.items[0], "umberto_eco");
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ5);
	assert(error == NULL);
	assert(action.code == MVCMD_CREATE_ENTITY);
	assert(action.attrs.size == 2);
	assert(strcmp(action.attrs.attrs[1].name, "author") == 0);
	assert(action.attrs.attrs[1].type == MVTYPE_RAWREF);
	assert(strcmp(action.attrs.attrs[1].value.rawref, "umberto_eco") == 0);
	mv_command_release(&action);
}

TEST mv_execute_test() {
	mv_command action;
	mv_session state;

	FAILFAST(mv_command_parse(&action, REQ1));
	FAILFAST(state.execute(&action));
	mv_command_release(&action);

	mv_error* error = mv_command_parse(&action, REQ5);
	FAIL(error);
	FAILFAST(state.execute(&action));
	ASSERT_INT(state.varcount(), 2);
	assert(state.entities.used == 2);
	assert(state.entities.items[1].data.size == 2);
	assert(state.entities.items[1].data.attrs[1].type == MVTYPE_REF);
	assert(state.entities.items[1].data.attrs[1].value.ref == 0);
	mv_command_release(&action);

	ASSERT_INT(state.classes.used, 0);
	ASSERT_INT(state.clsnames.used, 0);
	error = mv_command_parse(&action, REQ6);
	FAIL(error);
	FAILFAST(state.execute(&action));
	ASSERT_INT(state.classes.used, 1);
	ASSERT_INT(state.clsnames.used, 1);
	ASSERT_INT(state.classes.items[0].data.size, 1);
	ASSERT_INT(state.classes.items[0].data.specs[0].type, MVSPEC_TYPE);
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ8);
	FAIL(error);
	FAILFAST(state.execute(&action));

	mv_command_release(&action);
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
	mv_error* error = mv_command_parse(&action, REQ1);
	assert(error == NULL);
	FAILFAST(state.execute(&action));
	ref = state.findvar("##0");
	assert(ref == 0);
	ref = state.findvar("umberto_eco");
	assert(ref == 0);
	char* text1;
	error = mv_session_show(&text1, &state, "umberto_eco");
	assert(error == NULL);
	assert(strcmp(text1, response_1) == 0);

	free(text1);
	mv_command_release(&action);

	error = mv_session_show(&text1, &state, "##1");
	assert(error != NULL);
	mv_error_release(error);
}

TEST mv_attrspec_release_test() {
	mv_command cmd;
	mv_error* error;

	error = mv_command_parse(&cmd, REQ7);
	FAIL(error);
	mv_command_release(&cmd);

	error = mv_command_parse(&cmd, "show person");

	ASSERT_NULL(cmd.attrs.attrs);
	ASSERT_INT(cmd.attrs.size, 0);
	ASSERT_NULL(cmd.spec.specs);
	ASSERT_INT(cmd.spec.size, 0);
	FAIL(error);
	mv_command_release(&cmd);
}

