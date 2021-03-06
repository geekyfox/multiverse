
#include "test.h"

#define BEFORE(RQ) try { mvAst ast(RQ);

#define AFTER } catch (mvError* err) { FAIL(err); }

#define BEFOREBAD(RQ) mvError* error; \
try { mvAst ast(RQ); DIE("error expected"); } \
catch (mvError* err) { error = err; }

#define AFTERBAD mvError_release(error);

TESTREQ 1 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[0] == Leaf, true);
	ASSERT_STRING(ast[0].leaf().ptr, "create");
	ASSERT_INT(ast[2] == AttrList, true);
	ASSERT_INT(ast[2].subtree().size(), 1);
	ASSERT_INT(ast[2].subtree()[0] == (AttrPair), true);
}

TESTREQ 2 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[0] == Leaf, true);
	ASSERT_STRING(ast[0].leaf().ptr, "create");
	ASSERT_INT(ast[2] == AttrList, true);
	ASSERT_INT(ast[2].subtree().size(), 2);
	ASSERT_INT(ast[2].subtree()[0] == (AttrPair), true);
}

TESTREQ 4 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[0] == Leaf, true);
	ASSERT_STRING(ast[0].leaf().ptr, "create");
	ASSERT_INT(ast[2] == AttrList, true);
	ASSERT_INT(ast[2].subtree().size(), 1);
	ASSERT_INT(ast[2].subtree()[0] == (AttrPair), true);
}

TESTREQ 6 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[3] == (AttrSpecList), true);
	ASSERT_INT(ast[3].subtree().size(), 1);
	ASSERT_INT(ast[3].subtree()[0] == (TypeSpec), true);
}

TESTREQ 9 {
	ASSERT_INT(ast.size(), 3);
	ASSERT_INT(ast[2] == AttrList, true);
	ASSERT_INT(ast[2].subtree().size(), 3);
}

TESTREQ 10 {
	ASSERT_INT(ast.size(), 4);
}

TESTREQ 11 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[3] == AttrList, true);
	ASSERT_INT(ast[3].subtree().size(), 1);
}

TESTREQ 14 {
	ASSERT_INT(ast.size(), 4);
	ASSERT_INT(ast[2] == AttrList, true);
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
	ASSERT_INT(ast[0] == (SubQuery), true);
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
	ASSERT_INT(ast[3] == AttrSpecList, true);
	const mvAst& subtree = ast[3].subtree();
	ASSERT_INT(subtree.size(), 1);
	ASSERT_INT(subtree[0] == AttrQuery, true);
}

TESTREQ 20 {
	ASSERT_INT(ast.size(), 5);
	ASSERT_INT(ast[4] == AttrList, true);
	const mvAst& subtree = ast[4].subtree();
	ASSERT_INT(subtree.size(), 1);
	ASSERT_INT(subtree[0] == AttrPair, true);
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

