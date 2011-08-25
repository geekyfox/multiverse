
#include "parser.h"
#include "test.h"

#define BEFORE(RQ) mv_ast ast; FAILFAST(mv_ast_parse(ast, RQ));

#define AFTER mv_ast_release(&ast);

#define BEFOREBAD(RQ) \
mv_ast ast; \
mv_error* error = mv_ast_parse(ast, RQ); \
ASSERT_NOTNULL(error)

#define AFTERBAD mv_error_release(error);

TESTREQ 1 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf.ptr, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree->size(), 1);
	ASSERT_INT(ast.items[2].value.subtree->items[0].type, MVAST_ATTRPAIR);
}

TESTREQ 2 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf.ptr, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree->size(), 2);
	ASSERT_INT(ast.items[2].value.subtree->items[0].type, MVAST_ATTRPAIR);
}

TESTREQ 4 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf.ptr, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree->size(), 1);
	ASSERT_INT(ast.items[2].value.subtree->items[0].type, MVAST_ATTRPAIR);
}

TESTREQ 6 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[3].type, MVAST_ATTRSPECLIST);
	ASSERT_INT(ast.items[3].value.subtree->size(), 1);
	ASSERT_INT(ast.items[3].value.subtree->items[0].type, MVAST_TYPESPEC);
}

TESTREQ 9 {
	ASSERT_INT(ast.size(), 3);
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree->size(), 3);
}

TESTREQ 10 {
	ASSERT_INT(ast.size(), 4);
}

TESTREQ 11 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[3].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[3].value.subtree->size(), 1);
}

TESTREQ 14 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree->size(), 1);
}

TEST selfquery() {
	BEFORE("author = $$");
	ASSERT_INT(ast.size(), 1);
	AFTER;
}

TEST subquery() {
	BEFORE("[book with { author = $$ }]");
	ASSERT_INT(ast.size(), 1);
	ASSERT_INT(ast.items[0].type, MVAST_SUBQUERY);
	ASSERT_INT(ast.items[0].value.subtree->size(), 2);
	AFTER;
}

TEST subquery2() {
	BEFORE("[book with {}]");
	ASSERT_INT(ast.size(), 1);
	AFTER;
} 

TEST subquery3() {
	BEFORE("{ foo = [book with {}] }");
	ASSERT_INT(ast.size(), 1);
	AFTER;
}

TESTREQ 18 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast.items[3].type, MVAST_ATTRSPECLIST);
	mv_ast* subtree = ast.items[3].value.subtree;
	ASSERT_INT(subtree->size(), 1);
	ASSERT_INT(subtree->items[0].type, MVAST_ATTRQUERY);
}

TESTREQ 20 {
	ASSERT_INT(ast.size(), 5);
	ASSERT_INT(ast.items[4].type, MVAST_ATTRLIST);
	mv_ast* subtree = ast.items[4].value.subtree;
	ASSERT_INT(subtree->size(), 1);
	ASSERT_INT(subtree->items[0].type, MVAST_ATTRPAIR);
}

TESTBADREQ 1 {
	ASSERT_INT(error->code, MVERROR_SYNTAX);
}

TESTBADREQ 2 {
	ASSERT_INT(error->code, MVERROR_SYNTAX);
}

TESTBADREQ 3 {
	ASSERT_INT(error->code, MVERROR_SYNTAX);
}

