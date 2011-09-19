
#include "multiverse.h"
#include "mvCommand.h"

mvCommand::~mvCommand()
{
	destroy();
}

void mvCommand::destroy()
{
	if (!inited) return;
	mv_attrlist_release(&attrs);
	vars.clear();
	spec.clear();
	inited = false;
}

