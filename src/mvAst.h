
#ifndef __MULTIVERSE_AST_HEADER__
#define __MULTIVERSE_AST_HEADER__

#include <assert.h>
#include "mvAttr.h"
#include "mvArray.h"
#include "error.h"
#include "multiverse.h"

class mv_ast_entry;

enum mvAstType
{
	Command, AttrList, AttrSpecList, AttrPair, AttrQuery,
	TypeSpec, AttrListTMP, AttrSpecListTMP, SubQuery
};

#define MVAST_ATTRSPECLIST AttrSpecList
#define MVAST_ATTRPAIR     AttrPair
#define MVAST_ATTRQUERY    AttrQuery
#define MVAST_TYPESPEC     TypeSpec
#define MVAST_TEMPATTRLIST AttrListTMP
#define MVAST_TEMPATTRSPECLIST AttrSpecListTMP
#define MVAST_SUBQUERY     SubQuery

class mvAst : public mvStaticArray<mv_ast_entry> {
private:
	mvAstType _type;
public:
	mvAst(mvAstType type) :
		mvStaticArray<mv_ast_entry>(),
		_type(type)
	{
	}
	mvAst(mvAstType type, int size) :
		mvStaticArray<mv_ast_entry>(size),
		_type(type)
	{
	}
	mvAst(const char* request) throw (mv_error*);
	void populate(mv_speclist& specs) const;
	void populate(mv_attrlist& target) const;
	void populate(mvQuery& target) const;
	void* operator new(size_t size);
	void operator delete(void* ptr);
	void fix()
	{
		switch(_type)
		{
		case AttrListTMP: _type = AttrList; return;
		case AttrSpecListTMP: _type = AttrSpecList; return;
		default: assert(0);
		}
	}
	mvAstType type() const
	{
		return _type;
	}
};

typedef mvAst mv_ast;

enum mvAstEntryType
{
	Unset, Leaf, Subtree, OpenBrace, CloseBrace,
	Comma, Colon, Equals, Apostrophe, CloseBracket,
	OpenBracket
};

#define MVAST_TEMPCLOSEBRACKET CloseBracket
#define MVAST_TEMPOPENBRACKET  OpenBracket
#define MVAST_TEMPCOLON        Colon
#define MVAST_TEMPEQUALS       Equals
#define MVAST_TEMPCOMMA        Comma
#define MVAST_TEMPAPOSTROPHE   Apostrophe
#define MVAST_TEMPCLOSEBRACE   CloseBrace

/*
 *  This structure represents an abstract syntax
 *  tree's entry.
 */
class mv_ast_entry {
private:
	mv_ast* _subtree;
	mvStrref* _leaf;
	mvAstEntryType _type;
public:
	mv_ast_entry() : _subtree(NULL), _leaf(NULL), _type(Unset)
	{
	}
	mv_ast_entry(mv_ast_entry& ref);
	~mv_ast_entry();
	bool operator==(mvAstEntryType code);
	bool operator!=(mvAstEntryType code);
	bool operator==(mvAstType type);
	bool operator!=(mvAstType type);
	bool operator==(const char* leafval);
	bool operator!=(const char* leafval);
	int type()
	{
		return _type;
	}
	void reset()
	{
		_type = Unset;
	}
private:
	void operator= (mv_ast* subtree)
	{
		_subtree = subtree;
		this->_type = Subtree;
		this->_leaf = NULL;
	}
public:
	void operator= (mvAstType type);
	void set_subtree(mvAstType type, mv_ast_entry& first);
	void set_subtree(mvAstType type, mv_ast_entry& first, mv_ast_entry& second);
	const mv_ast& subtree()
	{
		assert(_subtree != NULL);
		return *_subtree;
	}
	void subtree_push(mv_ast_entry& entry)
	{
		assert(_subtree != NULL);
		_subtree->push(entry);
	}
	void subtree_fix()
	{
		_subtree->fix();
	}
	void operator= (mvStrref& leaf);
	void operator= (mv_ast_entry& entry);
	void clear_leaf()
	{
		delete _leaf;
		_leaf = NULL;
		_type = Unset;
	}
	mvStrref& leaf()
	{
		assert(_leaf != NULL);
		return *_leaf;
	}	
};

/*
 * Constants for syntax tree entry's type
 */



#endif

