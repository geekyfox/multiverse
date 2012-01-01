
#ifndef __MULTIVERSE_AST_HEADER__
#define __MULTIVERSE_AST_HEADER__

#include <assert.h>
#include "mvAttr.h"
#include "mvArray.h"
#include "error.h"
#include "multiverse.h"

class mvAstEntry;

enum mvAstType
{
	Command, AttrList, AttrSpecList, AttrPair, AttrQuery,
	TypeSpec, AttrListTMP, AttrSpecListTMP, SubQuery
};

class mvAst : public mvStaticArray<mvAstEntry> {
private:
	mvAstType _type;
public:
	mvAst(mvAstType type) :
		mvStaticArray<mvAstEntry>(),
		_type(type)
	{
	}
	mvAst(mvAstType type, int size) :
		mvStaticArray<mvAstEntry>(size),
		_type(type)
	{
	}
	mvAst(const char* request) throw (mvError*);
	void populate(mvSpecList& specs) const;
	void populate(mvAttrlist& target) const;
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
class mvAstEntry {
private:
	mvAst* _subtree;
	mvStrref* _leaf;
	mvAstEntryType _type;
public:
	mvAstEntry() : _subtree(NULL), _leaf(NULL), _type(Unset)
	{
	}
	mvAstEntry(mvAstEntry& ref);
	~mvAstEntry();
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
	void operator= (mvAst* subtree)
	{
		_subtree = subtree;
		this->_type = Subtree;
		this->_leaf = NULL;
	}
public:
	void operator= (mvAstType type);
	void set_subtree(mvAstType type, mvAstEntry& first);
	void set_subtree(mvAstType type, mvAstEntry& first, mvAstEntry& second);
	const mvAst& subtree()
	{
		assert(_subtree != NULL);
		return *_subtree;
	}
	void subtree_push(mvAstEntry& entry)
	{
		assert(_subtree != NULL);
		_subtree->push(entry);
	}
	void subtree_fix()
	{
		_subtree->fix();
	}
	void operator= (mvStrref& leaf);
	void operator= (mvAstEntry& entry);
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

