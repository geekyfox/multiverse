
#ifndef __MULTIVERSE_QUERY_HEADER__
#define __MULTIVERSE_QUERY_HEADER__

#include "mvEntity.h"
#include "mvAttr.h"

class mvCommand;

class mvQuery
{
public:
	mvQuery();
	mvQuery(mvCommand& cmd);
	~mvQuery();
	bool match(mvEntity& entity);
	mvStrref classname;
	mvAttrlist attrs;
};

mvStrBuffer& operator<<(mvStrBuffer&, const mvQuery&);

#include "mvCommand.h"

#endif

