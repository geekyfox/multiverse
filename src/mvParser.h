
#ifndef __MULTIVERSE_PARSER_HEADER__
#define __MULTIVERSE_PARSER_HEADER__

#include "mvAttr.h"
#include "mvAst.h"
#include "mvCommand.h"
#include "mvStrref.h"

class mvParser {
private:
	void _parse_lookup(mvCommand&, const mvAst&)
	throw (mvError*);
public:
	void parse(mvAttr& target, const char* name, const char* value);

	void parse(mvCommand& target, const char* data)
	throw (mvError*);
};

extern mvParser singletonParser;

#endif

