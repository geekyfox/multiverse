
#include <assert.h>
#include <string.h>

#include "mvAst.h"
#include "mvMemPool.h"
#include "mvParser.h"
#include "multiverse.h"

void mvAstEntry::operator= (mvAstType code)
{
	(*this) = new mvAst(code, 0);
}

void mvAstEntry::operator= (mvAstEntry& ref)
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

void mvAstEntry::set_subtree(mvAstType code, mvAstEntry& first)
{
	mvAst* tree = new mvAst(code, 1);
	(*tree)[0] = first;
	(*this) = tree;
}

void mvAstEntry::set_subtree(mvAstType code, mvAstEntry& first,
                               mvAstEntry& second)
{
	mvAst* tree = new mvAst(code, 2);
	(*tree)[0] = first;
	(*tree)[1] = second;
	(*this) = tree;
}

mvAstEntry::~mvAstEntry()
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

void mvAstEntry::operator=(mvStrref& token)
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

bool mvAstEntry::operator==(mvAstEntryType code)
{
	return _type == code;
}

bool mvAstEntry::operator!=(mvAstEntryType code)
{
	return _type != code;
}

bool mvAstEntry::operator==(mvAstType type)
{
	if (_subtree == NULL) return false;
	return _subtree->type() == type;
}

bool mvAstEntry::operator!=(mvAstType type)
{
	if (_subtree == NULL) return true;
	return _subtree->type() != type;
}

bool mvAstEntry::operator==(const char* leafval)
{
	return (_leaf != NULL) && ((*_leaf) == leafval);
}

bool mvAstEntry::operator!=(const char* leafval)
{
	return (_leaf == NULL) || ((*_leaf) != leafval);
}

void mvAst::populate(mvSpecList& target) const
{
	target.alloc(size());
	int i;
	for (i=0; i<size(); i++) {
		mvAstEntry& src = (*this)[i];
		EXPECT(
		    src.subtree().size() == 2,
			"Two elements expected"
		);
		const mvAst& sub = src.subtree();
		EXPECT(sub[0] == Leaf, "Leaf expected as a first item");
		mvStrref& key = sub[0].leaf();
		switch (src.subtree().type())
		{
		case AttrQuery:
			EXPECT(
			    sub[1] == SubQuery,
			    "First item of AttrQuery should be a Subquery"
			);
			sub[1].subtree().populate(target[i].subquery_mutable());
			target[i].name = key;
			break;
		case AttrPair:
		case TypeSpec:
			EXPECT(sub[1] == Leaf, "Leaf expected as a second item");
			singletonParser.parse(
				target[i], key, sub[1].leaf(), src.subtree().type()
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

void mvAst::populate(mvAttrlist& target) const
{
	target.alloc(size());
	for (int i=0; i<size(); i++)
	{
		mvAstEntry& srcitem = (*this)[i];
		EXPECT(srcitem == AttrPair, "AttrPair expected");
		const mvAst& astpair = srcitem.subtree();
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

mvAst::mvAst(const char* data)
throw (mvError*) :
	_type(Command)
{
	mvTokenizer tokens(data);
	mvAstStack stack(tokens.size());
	int i, scan = 0;

	while (scan < tokens.size())
	{
		stack.add(tokens[scan++]);
	}
	
	this->set(stack.data, stack.last);
	int die = 0;
	for (int i=0; i<tokens.size(); i++)
	{
		if (stack.data[i] != Unset) die = 1;
	}
	if (die) abort();

	mvError* err = NULL;
	for (i=0; i < stack.last; i++) {
		mvAstEntry& ref = (*this)[i];
		if (ref != Leaf && ref != Subtree && err == NULL)
		{
			err = mvError_unmatched(ref.type(), data);
		}
	}
	if (err != NULL) throw err;
}

