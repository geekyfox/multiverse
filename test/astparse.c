
#include "test.h"

#define BEFORE(RQ) mv_ast ast; FAILFAST(mv_ast_parse(&ast, RQ));

#define AFTER mv_ast_release(&ast);

TESTREQ 1 {
	ASSERT_INT(ast.size, 4);
	ASSERT_INT(ast.items[0].type, MVAST_LEAF);
	ASSERT_STRING(ast.items[0].value.leaf, "create");
	ASSERT_INT(ast.items[2].type, MVAST_ATTRLIST);
	ASSERT_INT(ast.items[2].value.subtree.size, 1);
	ASSERT_INT(ast.items[2].value.subtree.items[0].type, MVAST_ATTRPAIR);
}
