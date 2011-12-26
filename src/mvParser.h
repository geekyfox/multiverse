
#ifndef __MULTIVERSE_PARSER_HEADER__
#define __MULTIVERSE_PARSER_HEADER__

#include "mvAttr.h"
#include "mvAst.h"
#include "mvCommand.h"
#include "mvStrref.h"

class mvParser {
private:
	void _parse_lookup(mvCommand&, const mvAst&)
	throw (mv_error*);
public:
	void parse(mv_attr& target, const char* name, const char* value);

	void parse(mvCommand& target, const char* data)
	throw (mv_error*);
};

extern mvParser singletonParser;

#endif

