
#include <assert.h>
#include <string.h>

#include "mvAst.h"
#include "mvMemPool.h"
#include "mvParser.h"
#include "multiverse.h"
#include "parser.h"

void mv_ast_entry::operator= (mvAstType code)
{
	(*this) = new mv_ast(code, 0);
}

void mv_ast_entry::operator= (mv_ast_entry& ref)
{
	if (this->_leaf != NULL) delete _leaf;
	if (this->_subtree != NULL) delete _subtree;
	//
	this->_leaf = ref._leaf;
	this->_subtree = ref._subtree;
	this->_type = ref._type;
	//
	ref._leaf = NULL;
	ref._subtree = NULL;
	ref._type = Unset;
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

mv_ast_entry::~mv_ast_entry()
{
	if (_leaf != NULL)
	{
		delete _leaf;
		_leaf = NULL;
	}
	if (_subtree != NULL)
	{
		delete _subtree;
		_subtree = NULL;
	}
}

void mv_ast_entry::operator=(mvStrref& token)
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
	_leaf = new mvStrref(token);
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

void mvAst::populate(mv_speclist& target) const
{
	target.alloc(size());
	int i;
	for (i=0; i<size(); i++) {
		mv_ast_entry& src = (*this)[i];
		EXPECT(
		    src.subtree().size() == 2,
			"Two elements expected"
		);
		const mv_ast& sub = src.subtree();
		EXPECT(sub[0] == Leaf, "Leaf expected as a first item");
		mvStrref& key = sub[0].leaf();
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
				&(target[i]), key, sub[1].leaf(), src.subtree().type()
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

void mvAst::populate(mv_attrlist& target) const
{
	target.alloc(size());
	for (int i=0; i<size(); i++)
	{
		mv_ast_entry& srcitem = (*this)[i];
		EXPECT(srcitem == AttrPair, "AttrPair expected");
		const mv_ast& astpair = srcitem.subtree();
		EXPECT(astpair.size() == 2, "Two elements expected");
		EXPECT(astpair[0] == Leaf, "Leafs expected");
		EXPECT(astpair[1] == Leaf, "Leafs expected");
		singletonParser.parse(target[i],
		                      astpair[0].leaf().ptr,
		                      astpair[1].leaf().ptr);
	}
}

void mvAst::populate(mvQuery& target) const
{
	target.classname = strdup((*this)[0].leaf().ptr);
	(*this)[1].subtree().populate(target.attrs);
}

