
#include "test.h"

static void __tokenizeimpl(char* request, char** expect, int count) {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, request);
	FAIL(error);
	ASSERT_INT(tokens.size, count);
	ASSERT_INT(tokens.used, count);
	ASSERT_NOTNULL(tokens.items);
	int i;
	for (i=0; i<count; i++) ASSERT_STRING(tokens.items[i], expect[i]);
	mv_strarr_release(&tokens);
}

TEST attr_test1() {
	mv_attr pair;
	mv_attr_parse(&pair, "name", "'Umberto Eco");
	ASSERT_INT(pair.type, MVTYPE_STRING);
	ASSERT_STRING(pair.name, "name");
	ASSERT_STRING(pair.value.string, "Umberto Eco");
	mv_attr_release(&pair);
}

TEST tokenize_REQ1() {
	char* expect[] = {
		"create", "entity", "{", "name", "=", "'Umberto Eco", "}",
        "umberto_eco"
	};
	__tokenizeimpl(REQ1, expect, 8);
}

TEST tokenize_REQ2() {
	char* expect[] = {
		"create", "entity", "{", "country", "=", "italy", ",", "name",
        "=", "'Umberto Eco", "}", "umberto_eco"
	};

	__tokenizeimpl(REQ2, expect, 12);
}

TEST tokenize_REQ7() {
	char* expect[] = {
		"create", "class", "person", "{", "name", ":", "string", "}"
	};

	__tokenizeimpl(REQ7, expect, 8);
}

TEST tokenize_fails() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, BADREQ1);
	ASSERT_NOTNULL(error);
	ASSERT_INT(error->code, MVERROR_SYNTAX);
	mv_error_release(error);
}

TEST tokenize_test() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, "\nquit");
	FAIL(error);
	ASSERT_INT(tokens.used, 1);
	mv_strarr_release(&tokens);
}

TEST astparse_REQ2() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ2);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree.size, 2);
	ASSERT_INT(ast.items[2].value.subtree.items[0].type, MVAST_ATTRPAIR);
	mv_ast_release(&ast);
}

TEST astparse_REQ4() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ4);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree.size, 1);
	ASSERT_INT(ast.items[2].value.subtree.items[0].type, MVAST_ATTRPAIR);
	mv_ast_release(&ast);
}

TEST astparse_REQ6() {
	mv_ast ast;
	mv_error* error;
	
	error = mv_ast_parse(&ast, REQ6);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[3].type, MVAST_ATTRSPECLIST);
	ASSERT_INT(ast.items[3].value.subtree.size, 1);
	ASSERT_INT(ast.items[3].value.subtree.items[0].type, MVAST_TYPESPEC);
	mv_ast_release(&ast);
}

TEST astparse_REQ9() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ9);
	FAIL(error);
	ASSERT_INT(ast.size, 3);
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree.size, 3);
	mv_ast_release(&ast);
}

TEST astparse_REQ10() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ10);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	mv_ast_release(&ast);
}

TEST astparse_REQ11() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ11);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[3].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[3].value.subtree.size, 1);
	mv_ast_release(&ast);
}

TEST astparse_REQ14() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ14);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree.size, 1);
	mv_ast_release(&ast);
}

static void __astparse_fail(char* request) {
	mv_ast ast;
	mv_error* error;
	
	error = mv_ast_parse(&ast, request);
	ASSERT_NOTNULL(error);
	ASSERT_INT(error->code, MVERROR_SYNTAX);
	mv_error_release(error);
}

TEST astparse_failures() {
	__astparse_fail(BADREQ1);
	__astparse_fail(BADREQ2);
	__astparse_fail(BADREQ3);
}

static void __cmdparse_fail(char* request) {
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

TEST cmdparse_failures() {
	__cmdparse_fail(BADREQ1);
	__cmdparse_fail(BADREQ2);
	__cmdparse_fail(BADREQ3);
}

