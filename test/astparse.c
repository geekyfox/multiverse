
#include "parser.h"
#include "test.h"

#define BEFORE(RQ) try { mvAst ast(RQ);

#define AFTER } catch (mv_error* err) { FAIL(err); }

#define BEFOREBAD(RQ) mv_error* error; \
try { mv_ast ast(RQ); DIE("error expected"); } \
catch (mv_error* err) { error = err; }

#define AFTERBAD mv_error_release(error);

TESTREQ 1 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[0].type, MVAST_LEAF);
	ASSERT_STRING(ast[0].value.leaf->ptr, "create");
	ASSERT_INT(ast[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast[2].subtree().size(), 1);
	ASSERT_INT(ast[2].subtree()[0].type, MVAST_ATTRPAIR);
}

TESTREQ 2 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[0].type, MVAST_LEAF);
	ASSERT_STRING(ast[0].value.leaf->ptr, "create");
	ASSERT_INT(ast[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast[2].subtree().size(), 2);
	ASSERT_INT(ast[2].subtree()[0].type, MVAST_ATTRPAIR);
}

TESTREQ 4 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[0].type, MVAST_LEAF);
	ASSERT_STRING(ast[0].value.leaf->ptr, "create");
	ASSERT_INT(ast[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast[2].subtree().size(), 1);
	ASSERT_INT(ast[2].subtree()[0].type, MVAST_ATTRPAIR);
}

TESTREQ 6 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[3].type, MVAST_ATTRSPECLIST);
	ASSERT_INT(ast[3].subtree().size(), 1);
	ASSERT_INT(ast[3].subtree()[0].type, MVAST_TYPESPEC);
}

TESTREQ 9 {
	ASSERT_INT(ast.size(), 3);
	ASSERT_INT(ast[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast[2].subtree().size(), 3);
}

TESTREQ 10 {
	ASSERT_INT(ast.size(), 4);
}

TESTREQ 11 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[3].type, MVAST_ATTRLIST);
	ASSERT_INT(ast[3].subtree().size(), 1);
}

TESTREQ 14 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast[2].subtree().size(), 1);
}

TEST selfquery() {
	BEFORE("author = $$");
	ASSERT_INT(ast.size(), 1);
	AFTER;
}

TEST subquery() {
	BEFORE("[book with { author = $$ }]");
	ASSERT_INT(ast.size(), 1);
	ASSERT_INT(ast[0].type, MVAST_SUBQUERY);
	ASSERT_INT(ast[0].subtree().size(), 2);
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
	ASSERT_INT(ast[3].type, MVAST_ATTRSPECLIST);
	mv_ast& subtree = ast[3].subtree();
	ASSERT_INT(subtree.size(), 1);
	ASSERT_INT(subtree[0].type, MVAST_ATTRQUERY);
}

TESTREQ 20 {
	ASSERT_INT(ast.size(), 5);
	ASSERT_INT(ast[4].type, MVAST_ATTRLIST);
	mv_ast& subtree = ast[4].subtree();
	ASSERT_INT(subtree.size(), 1);
	ASSERT_INT(subtree[0].type, MVAST_ATTRPAIR);
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

