
#include <assert.h>
#include <string.h>

#include "mvAst.h"
#include "mvMemPool.h"
#include "multiverse.h"
#include "parser.h"

void mv_ast_entry::operator= (mvAstType code)
{
	(*this) = new mv_ast(code, 0);
}

void mv_ast_entry::set_subtree(mvAstType code, mv_ast_entry& first)
{
	mv_ast* tree = new mv_ast(code, 1);
	(*tree)[0] = first;
	(*this) = tree;
}

void mv_ast_entry::set_subtree(mvAstType code, mv_ast_entry& first,
                               mv_ast_entry& second)
{
	mv_ast* tree = new mv_ast(code, 2);
	(*tree)[0] = first;
	(*tree)[1] = second;
	(*this) = tree;
}

void mv_ast_entry::clear()
{
	if (_leaf != NULL)
	{
		delete _leaf;
	}
	if (_subtree != NULL)
	{
		delete _subtree;
	}
}

void mv_ast_entry::operator=(mv_strref& token)
{
	if (_leaf != NULL)
	{
		_leaf = NULL;
	}
	if (_subtree != NULL)
	{
		_subtree = NULL;
	}
	if (strlen(token.ptr) == 1)
	{
		switch (token.ptr[0])
		{
		case '{': _type = OpenBrace;              return;
		case '}': _type = MVAST_TEMPCLOSEBRACE;   return;
		case ',': _type = MVAST_TEMPCOMMA;        return;
		case ':': _type = MVAST_TEMPCOLON;        return;
		case '=': _type = MVAST_TEMPEQUALS;       return;
		case '[': _type = MVAST_TEMPOPENBRACKET;  return;
		case ']': _type = MVAST_TEMPCLOSEBRACKET; return;
		}
	}
	_leaf = new mv_strref(token);
	_type = Leaf;
}

bool mv_ast_entry::operator==(mvAstEntryType code)
{
	return _type == code;
}

bool mv_ast_entry::operator!=(mvAstEntryType code)
{
	return _type != code;
}

bool mv_ast_entry::operator==(mvAstType type)
{
	if (_subtree == NULL) return false;
	return _subtree->type() == type;
}

bool mv_ast_entry::operator!=(mvAstType type)
{
	if (_subtree == NULL) return true;
	return _subtree->type() != type;
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
		EXPECT(sub[0] == Leaf, "Leaf expected as a first item");
		char* key = sub[0].leaf().ptr;
		switch (src.subtree().type())
		{
		case MVAST_ATTRQUERY:
			EXPECT(
			    sub[1] == SubQuery,
			    "First item of AttrQuery should be a Subquery"
			);
			mv_attrquery_parse(
			    &(target[i]), key, sub[1].subtree()
			);
			break;
		case MVAST_ATTRPAIR:
		case MVAST_TYPESPEC:
			EXPECT(sub[1] == Leaf, "Leaf expected as a second item");
			mv_spec_parse(
				&(target[i]), key, sub[1].leaf().ptr, src.subtree().type()
			);
			break;
		default:
			DIE("Invalid AST element code: %d", src.type());
		}
	}
}

mvMemPool<mvAst, 512, 1> mempool;

void* mvAst::operator new(size_t size)
{
	return mempool.get();
}

void mvAst::operator delete(void* ptr)
{
	mempool.release((mvAst*)ptr);
}


