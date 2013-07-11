
#include "mvEntity.h"
#include "mvQuery.h"
#include "multiverse.h"

#include <assert.h>
#include <string.h>

mvQuery::mvQuery()
{
}

mvQuery::mvQuery(mvCommand& cmd)
{
	assert(cmd.code == LOOKUP);
	if (cmd.vars.size() == 0)
	{
		classname = NULL;
	}
	else
	{
		assert(cmd.vars.size() == 1);
		classname = strdup(cmd.vars[0].ptr);
	}
	attrs.copy_from(cmd.attrs);
}

mvQuery::~mvQuery()
{
	free(classname);
	attrs.clear();
}

bool __attrmatch(mvAttr& x, mvAttr& y)
{
	if (x.name != y.name) return 0;
	if (x.type != y.type) return 0;
	switch (x.type) {
	case STRING:
		return STREQ(x.value.string, y.value.string);
	case INTEGER:
		return x.value.integer == y.value.integer;
	default:
		DIE("Invalid code (%d)", x.type);
	}
}

bool mvQuery::match(mvEntity& entity)
{
	int i, j;
	
	if (classname != NULL)
	{
		bool match = false;
		for (i=0; i<entity.classes.size(); i++)
		{
			if (STREQ(classname, entity.classes[i].ptr))
			{
				match = true;
				break;
			}
		}
		if (!match) return false;
	}

	for (i=0; i<attrs.size(); i++)
	{
		mvAttr& x = attrs[i];
		bool match = false;
		for (j=0; j<entity.data.size(); j++) {
			mvAttr& y = entity.data[j];
			if (__attrmatch(x, y)) {
				match = true;
				break;
			}
		}
		if (!match) return false;
	}
	return true;
}

mvStrBuffer& operator<<(mvStrBuffer& buff, const mvQuery& q)
{
	buff << q.classname << " with ";
	buff.indent(2);
	buff << q.attrs;
	buff.unindent(2);
	return buff;
}

