
#ifndef __MULTIVERSE_PARSER_HEADER__
#define __MULTIVERSE_PARSER_HEADER__

#include "mvAttr.h"
#include "mvStrref.h"

class mvParser {
public:
	void parse(mv_attr& target, const char* name, const char* value);
};

extern mvParser singletonParser;

#endif

