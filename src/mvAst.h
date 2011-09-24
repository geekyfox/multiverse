
#ifndef __MULTIVERSE_AST_HEADER__
#define __MULTIVERSE_AST_HEADER__

#include "model.h"
#include "mvArray.h"
#include "error.h"
#include "multiverse.h"

class mv_ast_entry;

enum mvAstType
{
	Command, AttrList, AttrSpecList, AttrPair, AttrQuery,
	TypeSpec, AttrListTMP, AttrSpecListTMP, SubQuery
};

#define MVAST_ATTRLIST     AttrList
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
	void populate(mv_speclist& specs);
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
	mvAstType type()
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
	mv_strref* _leaf;
	mvAstEntryType _type;
public:
	mv_ast_entry() : _subtree(NULL), _leaf(NULL), _type(Unset)
	{
	}
	bool operator==(mvAstEntryType code);
	bool operator!=(mvAstEntryType code);
	bool operator==(mvAstType type);
	bool operator!=(mvAstType type);
	int type()
	{
		return _type;
	}
	/* Entry's type (MVAST_) code. */
	/* Entry's value.
	 *
	 * When type=MVAST_LEAF, leaf contains the value.
	 *
	 * When type is composite (such as MVAST_ATTRLIST), subtree
	 * contains the value.
	 *
	 * When type is atomic (such as MVAST_TEMPCOMMA, see
	 * mv_ast_release() for a full list), there's no value.
	 */
	void clear();
	void operator= (mv_ast* subtree)
	{
		_subtree = subtree;
		this->_type = Subtree;
		this->_leaf = NULL;
	}
	void operator= (mvAstType type);
	void set_subtree(mvAstType type, mv_ast_entry& first);
	void set_subtree(mvAstType type, mv_ast_entry& first, mv_ast_entry& second);
	mv_ast& subtree()
	{
		assert(_subtree != NULL);
		return *_subtree;
	}
	void operator= (mv_strref& leaf);
	void clear_leaf()
	{
		delete _leaf;
	}
	mv_strref& leaf()
	{
		assert(_leaf != NULL);
		return *_leaf;
	}	
};

/*
 * Constants for syntax tree entry's type
 */



#endif

