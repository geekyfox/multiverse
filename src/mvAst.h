
#ifndef __MULTIVERSE_AST_HEADER__
#define __MULTIVERSE_AST_HEADER__

#include "model.h"
#include "mvArray.h"
#include "error.h"
#include "multiverse.h"

class mv_ast_entry;

class mvAst : public mvStaticArray<mv_ast_entry> {
public:
	mvAst() :
		mvStaticArray<mv_ast_entry>()
	{
	}
	mvAst(int size) :
		mvStaticArray<mv_ast_entry>(size)
	{
	}
	mvAst(const char* request) throw (mv_error*);

	void populate(mv_speclist& specs);
};

#define MVAST_SUBQUERY          2006

typedef mvAst mv_ast;

/*
 *  This structure represents an abstract syntax
 *  tree's entry.
 */
class mv_ast_entry {
private:
	mv_ast* _subtree;
	mv_strref* _leaf;
	int _type;
public:
	mv_ast_entry() : _subtree(NULL), _leaf(NULL), _type(0)
	{
	}
	bool type_is(int code)
	{
		if (_leaf != NULL) return false;
		return _type == code;
	}
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
	void set(int type, mv_ast* subtree)
	{
		_subtree = subtree;
		this->_type = type;
		this->_leaf = NULL;
	}
	void set_subtree(int type);
	void set_subtree(int type, mv_ast_entry& first);
	void set_subtree(int type, mv_ast_entry& first, mv_ast_entry& second);
	mv_ast& subtree()
	{
		assert(_subtree != NULL);
		return *_subtree;
	}
	void set_leaf(mv_strref& leaf)
	{
		_leaf = new mv_strref;
		*_leaf = leaf;
	}
	void set_type(int code)
	{
		_type = code;
	}
	void clear_leaf()
	{
		delete _leaf;
	}
	mv_strref& leaf()
	{
		assert(_leaf != NULL);
		return *_leaf;
	}	
	bool is_leaf()
	{
		return _leaf != NULL;
	}
	bool is_subquery()
	{
		return _type == MVAST_SUBQUERY;
	}
};

/*
 * Constants for syntax tree entry's type
 */
#define MVAST_TEMPCLOSEBRACKET -2010
#define MVAST_TEMPOPENBRACKET  -2009
#define MVAST_TEMPCOLON        -2008
#define MVAST_TEMPEQUALS       -2007
#define MVAST_TEMPCOMMA        -2006
#define MVAST_TEMPAPOSTROPHE   -2005
#define MVAST_TEMPATTRSPECLIST -2004
#define MVAST_TEMPCLOSEBRACE   -2003
#define MVAST_TEMPOPENBRACE    -2002
#define MVAST_TEMPATTRLIST     -2001
#define MVAST_ATTRLIST          2002
#define MVAST_ATTRPAIR          2003
#define MVAST_TYPESPEC          2004
#define MVAST_ATTRSPECLIST      2005
#define MVAST_ATTRQUERY         2007



#endif

