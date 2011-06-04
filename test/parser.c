
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

TEST mv_astparse_REQ1() {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, REQ1);
	FAIL(error);
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree.size, 1);
	ASSERT_INT(ast.items[2].value.subtree.items[0].type, MVAST_ATTRPAIR);
	mv_ast_release(&ast);
}

TEST mv_astparse_REQ2() {
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

