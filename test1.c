
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

void mv_strbuf_test() {
	mv_strbuf buf;
	mv_strbuf_alloc(&buf, 1000);
	mv_strbuf_append(&buf, "foo");
	mv_strbuf_append(&buf, " + bar");
	mv_strbuf_append(&buf, " = baz");
	char* text = mv_strbuf_align(&buf);
	assert(strcmp(text, "foo + bar = baz") == 0);
	free(text);
	printf("mv_strbuf_test PASSED\n");
}

void mv_varbind_test() {
	mv_varbind tmp;
	mv_varbind_alloc(&tmp, 2);
	assert(tmp.size == 2);
	assert(tmp.used == 0);
	int t = mv_varbind_lookup(&tmp, "foo");
	assert(t == -1);
	mv_varbind_insert(&tmp, "foo", 42);
	assert(tmp.used == 1);
	t = mv_varbind_lookup(&tmp, "foo");
	assert(t == 42);
	mv_varbind_insert(&tmp, "bar", 4242);
	assert(tmp.used == 2);
	assert(tmp.size == 4);
	t = mv_varbind_lookup(&tmp, "foo");
	assert(t == 42);
	t = mv_varbind_lookup(&tmp, "bar");
	assert(t == 4242);
	mv_varbind_insert(&tmp, "baz", 4242);
	mv_varbind_insert(&tmp, "quux", 424242);
	assert(tmp.size == 8);
	mv_varbind_insert(&tmp, "foo", 12345);
	assert(tmp.used == 4);
	t = mv_varbind_lookup(&tmp, "foo");
	assert(t == 12345);
	mv_varbind_remove(&tmp, "baz");
	assert(tmp.used == 3);
	t = mv_varbind_lookup(&tmp, "baz");
	assert(t == -1);
	t = mv_varbind_lookup(&tmp, "quux");
	assert(t == 424242);
	mv_varbind_release(&tmp);
	printf("mv_varbind_test PASSED\n");
}

char* request_1 = "create entity { name = 'Umberto Eco' } umberto_eco";
char* request_2 = "create entity { country = italy, name = 'Umberto Eco' } umberto_eco";
char* bad_request_1 = "create entity { name = 'Umberto ";
char* bad_request_2 = "create entity { name = 'Umberto' ";
char* bad_request_3 = "create entity { name = 'Umberto' , }";
char* request_3 = "show umberto_eco";
char* request_4 = "create entity {\nname = 'Umberto Eco'\n} umberto_eco";
char* response_1 = "umberto_eco = entity {\n  name = 'Umberto Eco'\n}\n";

void test_mvp_tokenize1() {
	mv_strarr tokens;
	mv_error* result = mv_tokenize(&tokens, request_1);
	assert(result == NULL);
	assert(tokens.used == 8);
	assert(tokens.items != NULL);
	char* expected[] = {
		"create", "entity", "{", "name", "=", "'Umberto Eco", "}", "umberto_eco"
	};
	int i;
	for (i=0; i<8; i++) {
		assert(strcmp(tokens.items[i], expected[i]) == 0);
	}
	mv_strarr_release(&tokens);
	printf("test_mvp_tokenize1 PASSED\n");
}

void mv_tokenize_test() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, request_2);
	assert(error == NULL);
	assert(tokens.size == 12);
	assert(tokens.items != NULL);
	char* expected[] = {
		"create", "entity", "{", "country", "=", "italy", ",", "name", "=", "'Umberto Eco", "}",
		"umberto_eco"
	};
	int i;
	for (i=0; i<12; i++) {
		assert(strcmp(tokens.items[i], expected[i]) == 0);
	}
	mv_strarr_release(&tokens);

	error = mv_tokenize(&tokens, bad_request_1);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_tokenize(&tokens, "\nquit");
	assert(error == NULL);
	assert(tokens.used == 1);
	mv_strarr_release(&tokens);
	printf("mv_tokenize_test PASSED\n");
}

void mv_ast_test() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, request_1);
	assert(error == NULL);
	assert(ast.size == 4);
	assert(ast.items[0].type == MVAST_LEAF);
	assert(strcmp(ast.items[0].value.leaf, "create") == 0);
	assert(ast.items[2].type == MVAST_ATTRLIST);
	assert(ast.items[2].value.subtree.size == 1);
	assert(ast.items[2].value.subtree.items[0].type == MVAST_ATTRPAIR);
	mv_ast_release(&ast);

	error = mv_ast_parse(&ast, request_2);
	assert(error == NULL);
	assert(ast.size == 4);
	assert(ast.items[0].type == MVAST_LEAF);
	assert(strcmp(ast.items[0].value.leaf, "create") == 0);
	assert(ast.items[2].type == MVAST_ATTRLIST);
	assert(ast.items[2].value.subtree.size == 2);
	assert(ast.items[2].value.subtree.items[0].type == MVAST_ATTRPAIR);
	mv_ast_release(&ast);

	error = mv_ast_parse(&ast, bad_request_1);
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

	printf("mv_ast_test PASSED\n");	
}


void test_mvp_build_attrpair1() {
	mv_attr pair;
	mv_attr_parse(&pair, "name", "'Umberto Eco");
	assert(pair.type == MVTYPE_STRING);
	assert(strcmp(pair.name, "name") == 0);
	assert(strcmp(pair.value.string, "Umberto Eco") == 0);
	mv_attr_release(&pair);
}

void mv_command_test() {
	mv_command action;
	mv_error* error = mv_command_parse(&action, request_1);
	assert(error == NULL);
	assert(action.code == MVCMD_CREATE_ENTITY);
	assert(action.attrs.size == 1);
	assert(action.attrs.attrs[0].type == MVTYPE_STRING);
	assert(strcmp(action.attrs.attrs[0].name, "name") == 0);
	assert(strcmp(action.attrs.attrs[0].value.string, "Umberto Eco") == 0);
	mv_command_release(&action);

	error = mv_command_parse(&action, request_2);
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

	error = mv_command_parse(&action, bad_request_1);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_command_parse(&action, bad_request_2);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_command_parse(&action, bad_request_3);
	assert(error != NULL);
	assert(error->code == MVERROR_SYNTAX);
	mv_error_release(error);

	error = mv_command_parse(&action, "quit");
	assert(error == NULL);
	assert(action.code == MVCMD_QUIT);
	assert(action.attrs.size == 0);
	assert(action.vars.size == 0);

	printf("mv_command_test PASSED\n");
}

void test_mvp_parse4() {
	mv_command action;
	mv_error* error = mv_command_parse(&action, request_3);
	assert(error == NULL);
	assert(action.code == MVCMD_SHOW);
	assert(action.attrs.size == 0);
	assert(action.vars.used == 1);
	assert(strcmp(action.vars.items[0], "umberto_eco") == 0);
	mv_command_release(&action);

	printf("test_mvp_parse4 PASSED\n");
}

void mv_execute_test() {
	mv_command action;
	mv_session state;
	mv_error* error = mv_command_parse(&action, request_1);
	assert(error == NULL);
	mv_session_init(&state);
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
	mv_error* error = mv_command_parse(&action, request_1);
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

void mv_error_raiseform_test() {
	mv_error* error = mv_error_raiseform(MVERROR_BADVAR, "foo: %s", "bar");
	assert(error != NULL);
	assert(error->code == MVERROR_BADVAR);
	assert(strcmp(error->message, "foo: bar") == 0);

	printf("mv_error_raiseform_test PASSED\n");

	mv_error_release(error);
}

int main() {
	mv_strbuf_test();
	mv_varbind_test();
	test_mvp_tokenize1();
	mv_tokenize_test();
	mv_ast_test();
	test_mvp_build_attrpair1();
	mv_command_test();
	mv_execute_test();
	mv_attrlist_show_test();
	mv_session_findvar_test();
	mv_error_raiseform_test();
	return 0;
}

