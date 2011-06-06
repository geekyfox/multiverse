
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

char* bad_request_3 = "create entity { name = 'Umberto' , }";
char* request_4 = "create entity {\nname = 'Umberto Eco'\n} umberto_eco";
char* response_1 = "umberto_eco = entity {\n  name = 'Umberto Eco'\n}\n";

TEST mv_ast_test() {
	mv_ast ast;
	mv_error* error;

	error = mv_ast_parse(&ast, BADREQ1);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_ast_parse(&ast, BADREQ2);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);
	
	error = mv_ast_parse(&ast, bad_request_3);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_ast_parse(&ast, request_4);
	assert(error == NULL);
	assert(ast.size == 4);
	assert(ast.items[0].type == MVAST_LEAF);
	assert(strcmp(ast.items[0].value.leaf, "create") == 0);
	assert(ast.items[2].type == MVAST_ATTRLIST);
	assert(ast.items[2].value.subtree.size == 1);
	assert(ast.items[2].value.subtree.items[0].type == MVAST_ATTRPAIR);
	mv_ast_release(&ast);

	error = mv_ast_parse(&ast, REQ6);
	assert(error == NULL);
	assert(ast.size == 4);
	assert(ast.items[3].type == MVAST_ATTRSPECLIST);
	assert(ast.items[3].value.subtree.size == 1);
	assert(ast.items[3].value.subtree.items[0].type == MVAST_TYPESPEC);
	mv_ast_release(&ast);
}

TEST mv_attr_test() {
	mv_attr pair;
	mv_attr_parse(&pair, "name", "'Umberto Eco");
	assert(pair.type == MVTYPE_STRING);
	assert(strcmp(pair.name, "name") == 0);
	assert(strcmp(pair.value.string, "Umberto Eco") == 0);
	mv_attr_release(&pair);
}

void check_parsing_failure(char* request) {
	mv_command action;
	mv_error* error = mv_command_parse(&action, request);
	if (error == NULL) {
		DIE("Parsing not failed '%s'", request);
	}
	if (error->code != MVERROR_SYNTAX) {
		mv_error_display(error, stderr);
		DIE("Error is not syntactic");
	}
	mv_error_release(error);
}

TEST mv_command_test() {
	mv_command action;
	mv_error* error = mv_command_parse(&action, REQ1);
	assert(error == NULL);
	assert(action.code == MVCMD_CREATE_ENTITY);
	assert(action.spec.size == 0);
	assert(action.spec.specs == NULL);
	assert(action.vars.used == 1);
	assert(action.vars.items != NULL);
	assert(strcmp(action.vars.items[0], "umberto_eco") == 0);
	assert(action.attrs.size == 1);
	assert(action.attrs.attrs[0].type == MVTYPE_STRING);
	assert(strcmp(action.attrs.attrs[0].name, "name") == 0);
	assert(strcmp(action.attrs.attrs[0].value.string, "Umberto Eco") == 0);
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ2);
	assert(error == NULL);
	assert(action.code == MVCMD_CREATE_ENTITY);
	assert(action.attrs.size == 2);
	assert(action.attrs.attrs[0].type == MVTYPE_RAWREF);
	assert(strcmp(action.attrs.attrs[0].name, "country") == 0);
	assert(strcmp(action.attrs.attrs[0].value.string, "italy") == 0);
	assert(action.attrs.attrs[1].type == MVTYPE_STRING);
	assert(strcmp(action.attrs.attrs[1].name, "name") == 0);
	assert(strcmp(action.attrs.attrs[1].value.string, "Umberto Eco") == 0);	
	mv_command_release(&action);

	check_parsing_failure(BADREQ1);
	check_parsing_failure(BADREQ2);
	check_parsing_failure(bad_request_3);

	error = mv_command_parse(&action, "quit");
	assert(error == NULL);
	assert(action.code == MVCMD_QUIT);
	assert(action.attrs.size == 0);
	assert(action.vars.size == 0);

	error = mv_command_parse(&action, REQ3);
	assert(error == NULL);
	assert(action.code == MVCMD_SHOW);
	assert(action.attrs.size == 0);
	assert(action.vars.used == 1);
	assert(strcmp(action.vars.items[0], "umberto_eco") == 0);
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ5);
	assert(error == NULL);
	assert(action.code == MVCMD_CREATE_ENTITY);
	assert(action.attrs.size == 2);
	assert(strcmp(action.attrs.attrs[1].name, "author") == 0);
	assert(action.attrs.attrs[1].type == MVTYPE_RAWREF);
	assert(strcmp(action.attrs.attrs[1].value.rawref, "umberto_eco") == 0);
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ7);
	FAIL(error);
	assert(action.code == MVCMD_CREATE_CLASS);
	assert(action.attrs.size == 0);
	assert(action.attrs.attrs == NULL);
	assert(action.spec.size == 1);
	assert(action.spec.specs != NULL);
	mv_command_release(&action);
}

TEST mv_execute_test() {
	mv_command action;
	mv_session state;
	mv_error* error = mv_command_parse(&action, REQ1);
	assert(error == NULL);
	state.clsnames.used = 12345;
	mv_session_init(&state);
	ASSERT_INT(state.clsnames.used, 0);
	error = mv_session_execute(&state, &action);
	if (error != NULL) {
		printf("%s\n", error->message);
	}
	assert(error == NULL);
	assert(state.vars.used == 1);
	assert(state.entities.used == 1);
	assert(state.entities.items[0].data.size == 1);
	assert(state.entities.items[0].data.attrs[0].type == MVTYPE_STRING);
	assert(strcmp(state.entities.items[0].data.attrs[0].name, "name") == 0);
	assert(strcmp(state.entities.items[0].data.attrs[0].value.string, "Umberto Eco") == 0);
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ5);
	FAIL(error);
	error = mv_session_execute(&state, &action);
	FAIL(error);
	assert(state.vars.used == 2);
	assert(state.entities.used == 2);
	assert(state.entities.items[1].data.size == 2);
	assert(state.entities.items[1].data.attrs[1].type == MVTYPE_REF);
	assert(state.entities.items[1].data.attrs[1].value.ref == 0);
	mv_command_release(&action);

	ASSERT_INT(state.classes.used, 0);
	ASSERT_INT(state.clsnames.used, 0);
	error = mv_command_parse(&action, REQ6);
	FAIL(error);
	error = mv_session_execute(&state, &action);
	FAIL(error);
	ASSERT_INT(state.classes.used, 1);
	ASSERT_INT(state.clsnames.used, 1);
	ASSERT_INT(state.classes.items[0].data.size, 1);
	ASSERT_INT(state.classes.items[0].data.specs[0].type, MVSPEC_TYPE);
	mv_command_release(&action);

	error = mv_command_parse(&action, REQ8);
	FAIL(error);
	error = mv_session_execute(&state, &action);
	FAIL(error);

	mv_command_release(&action);
	mv_session_release(&state);
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
	mv_session_init(&state);
	int ref = mv_session_findvar(&state, "foobar");
	assert(ref == -1);
	ref = mv_session_findvar(&state, "##0");
	assert(ref == -1);
	mv_command action;
	mv_error* error = mv_command_parse(&action, REQ1);
	assert(error == NULL);
	error = mv_session_execute(&state, &action);
	assert(error == NULL);
	ref = mv_session_findvar(&state, "##0");
	assert(ref == 0);
	ref = mv_session_findvar(&state, "umberto_eco");
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

	mv_session_release(&state);
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

