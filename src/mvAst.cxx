
#include <assert.h>
#include <string.h>

#include "mvAst.h"
#include "multiverse.h"
#include "parser.h"

void mv_ast_entry::set_subtree(int code)
{
	mv_ast* tree = new mv_ast(0);
	set(code, tree);
}

void mv_ast_entry::set_subtree(int code, mv_ast_entry& first)
{
	mv_ast* tree = new mv_ast(1);
	(*tree)[0] = first;
	set(code, tree);
}

void mv_ast_entry::set_subtree(int code, mv_ast_entry& first,
                               mv_ast_entry& second)
{
	mv_ast* tree = new mv_ast(2);
	(*tree)[0] = first;
	(*tree)[1] = second;
	set(code, tree);
}

void mv_ast_entry::clear()
{
	switch(type) {
	case MVAST_LEAF:
		mv_strref_free(value.leaf);
		free(value.leaf);
		break;
	case MVAST_TEMPOPENBRACE:
	case MVAST_TEMPCLOSEBRACE:
	case MVAST_TEMPCOMMA:
	case MVAST_TEMPCOLON:
	case MVAST_TEMPEQUALS:
		break;
	case MVAST_ATTRLIST:
	case MVAST_ATTRPAIR:
	case MVAST_TYPESPEC:
	case MVAST_ATTRSPECLIST:
	case MVAST_SUBQUERY:
	case MVAST_ATTRQUERY:
		delete _subtree;
		break;
	default:
		printf("code = %d\n", type);
	}
}

void mvAst::populate(mv_speclist& target) {
	target.alloc(size());
	int i;
	for (i=0; i<size(); i++) {
		mv_ast_entry& src = (*this)[i];
		EXPECT(
		    src.subtree().size() == 2,
			"Two elements expected"
		);
		mv_ast& sub = src.subtree();
		EXPECT(sub[0].is_leaf(), "Leaf expected as a first item");
		char* key = sub[0].value.leaf->ptr;
		switch (src.type) {
		case MVAST_ATTRQUERY:
			EXPECT(
			    sub[1].is_subquery(),
			    "First item of AttrQuery should be a Subquery"
			);
			mv_attrquery_parse(
			    &(target[i]), key, sub[1].subtree()
			);
			break;
		case MVAST_ATTRPAIR:
		case MVAST_TYPESPEC:
			EXPECT(sub[1].is_leaf(), "Leaf expected as a second item");
			mv_spec_parse(
				&(target[i]), key, sub[1].value.leaf->ptr, src.type
			);
			break;
		default:
			DIE("Invalid AST element code: %d", src.type);
		}
	}
}

