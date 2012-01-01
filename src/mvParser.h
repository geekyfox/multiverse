
#ifndef __MULTIVERSE_PARSER_HEADER__
#define __MULTIVERSE_PARSER_HEADER__

#include "mvAttr.h"
#include "mvAst.h"
#include "mvCommand.h"
#include "mvStrref.h"

class mvTokenizer : public mvStrArray
{
public:
	mvTokenizer(const char* request) throw (mvError*);
};	

class mvParser {
private:
	void _parse_assign(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_create(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_create_entity(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_destroy(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_lookup(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_show(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_update(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_update_entity(mvCommand&, const mvAst&)
	throw (mvError*);
	void _parse_quit(mvCommand&, const mvAst&)
	throw (mvError*);
public:
	void parse(mvAttr& target, const char* name, const char* value);
	void parse(mvCommand& target, const char* data)
	throw (mvError*);
	void parse(mvAttrSpec& target,
	           const mvStrref& key,
	           const mvStrref& value,
	           mvAstType rel);
};

class mvAstStack : public mvStack<mvAstEntry>
{
public:
	mvAstStack(int size) : mvStack<mvAstEntry>(size)
	{
	}

	void add(mvStrref& token);

private:
	void _comma_rule();
	void _emptylist_rule();
	void _list_rule();
	void _pair_rule();
	void _subquery_rule();
	void _compress();
};

extern mvParser singletonParser;

#endif

