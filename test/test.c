
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"


char* bad_request_2 = "create entity { name = 'Umberto' ";
char* bad_request_3 = "create entity { name = 'Umberto' , }";
char* request_3 = "show umberto_eco";
char* request_4 = "create entity {\nname = 'Umberto Eco'\n} umberto_eco";
char* response_1 = "umberto_eco = entity {\n  name = 'Umberto Eco'\n}\n";
char* request_5 = "create entity { title = 'Name of the Rose', author = umberto_eco } name_of_the_rose";
char* response_2 = "name_of_the_rose = entity {\n  title = 'Name of the Rose',\n  author = ##0\n}\n";

void mv_ast_test() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ1);
	assert(error == NULL);
	assert(ast.size == 4);
	assert(ast.items[0].type == MVAST_LEAF);
	assert(strcmp(ast.items[0].value.leaf, "create") == 0);
	assert(ast.items[2].type == MVAST_ATTRLIST);
	assert(ast.items[2].value.subtree.size == 1);
	assert(ast.items[2].value.subtree.items[0].type == MVAST_ATTRPAIR);
	mv_ast_release(&ast);

	error = mv_ast_parse(&ast, REQ2);
	assert(error == NULL);
	assert(ast.size == 4);
	assert(ast.items[0].type == MVAST_LEAF);
	assert(strcmp(ast.items[0].value.leaf, "create") == 0);
	assert(ast.items[2].type == MVAST_ATTRLIST);
	assert(ast.items[2].value.subtree.size == 2);
	assert(ast.items[2].value.subtree.items[0].type == MVAST_ATTRPAIR);
	mv_ast_release(&ast);

	error = mv_ast_parse(&ast, BADREQ1);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_ast_parse(&ast, bad_request_2);
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

	printf("mv_ast_test PASSED\n");	
}

void mv_attr_test() {
	mv_attr pair;
	mv_attr_parse(&pair, "name", "'Umberto Eco");
	assert(pair.type == MVTYPE_STRING);
	assert(strcmp(pair.name, "name") == 0);
	assert(strcmp(pair.value.string, "Umberto Eco") == 0);
	mv_attr_release(&pair);

	printf("mv_attr_test PASSED\n");
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

void mv_command_test() {
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

	printf("mv_command_test STEP 1 PASSED\n");

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

	printf("mv_command_test STEP 2 PASSED\n");

	check_parsing_failure(BADREQ1);
	check_parsing_failure(bad_request_2);
	check_parsing_failure(bad_request_3);

	error = mv_command_parse(&action, "quit");
	assert(error == NULL);
	assert(action.code == MVCMD_QUIT);
	assert(action.attrs.size == 0);
	assert(action.vars.size == 0);

	printf("mv_command_test STEP 6 PASSED\n");

	error = mv_command_parse(&action, request_3);
	assert(error == NULL);
	assert(action.code == MVCMD_SHOW);
	assert(action.attrs.size == 0);
	assert(action.vars.used == 1);
	assert(strcmp(action.vars.items[0], "umberto_eco") == 0);
	mv_command_release(&action);

	printf("mv_command_test STEP 7 PASSED\n");

	error = mv_command_parse(&action, request_5);
	assert(error == NULL);
	assert(action.code == MVCMD_CREATE_ENTITY);
	assert(action.attrs.size == 2);
	assert(strcmp(action.attrs.attrs[1].name, "author") == 0);
	assert(action.attrs.attrs[1].type == MVTYPE_RAWREF);
	assert(strcmp(action.attrs.attrs[1].value.rawref, "umberto_eco") == 0);
	mv_command_release(&action);

	printf("mv_command_test STEP 8 PASSED\n");

	error = mv_command_parse(&action, REQ7);
	FAIL(error);
	assert(action.code == MVCMD_CREATE_CLASS);
	assert(action.attrs.size == 0);
	assert(action.attrs.attrs == NULL);
	assert(action.spec.size == 1);
	assert(action.spec.specs != NULL);
	mv_command_release(&action);

	printf("mv_command_test PASSED\n");
}

void mv_execute_test() {
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

	error = mv_command_parse(&action, request_5);
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

	mv_session_release(&state);

	printf("mv_execute_test PASSED\n");	
}

void mv_attrlist_show_test() {
	mv_attrlist attrs;
	mv_attrlist_alloc(&attrs, 1);
	mv_attr_parse(attrs.attrs, "name", "'Umberto Eco");
	assert(attrs.attrs[0].type == MVTYPE_STRING);
	assert(strcmp(attrs.attrs[0].name, "name") == 0);
	assert(strcmp(attrs.attrs[0].value.string, "Umberto Eco") == 0);

	mv_attrlist_release(&attrs);
	printf("mv_attrlist_show_test PASSED\n");
}

void mv_session_findvar_test() {
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

	printf("mv_session_findvar_test PASSED\n");
}

void mv_showcmd_test() {
	mv_session state;
	mv_strarr script;

	mv_session_init(&state);
	mv_strarr_alloc(&script, 2);
	mv_strarr_append(&script, REQ1);
	mv_strarr_append(&script, request_5);

	mv_error* error = mv_session_perform(&state, &script);

	char* target;
	error = mv_session_show(&target, &state, "name_of_the_rose");
	assert(error == NULL);
	assert(strcmp(target, response_2) == 0);

	mv_session_release(&state);
	mv_strarr_release(&script);
	free(target);

	printf("mv_showcmd_test PASSED\n");	
}

void perform_fast_test() {
	mv_command_test();
	mv_attr_test();
	mv_ast_test();
	mv_execute_test();
	mv_attrlist_show_test();
	mv_session_findvar_test();
	mv_showcmd_test();
}

void perform_full_test() {
	perform_data_test();
	perform_parser_test();
	perform_fast_test();
}

int main(int argc, char** argv) {
	perform_full_test();
	return 0;
}

