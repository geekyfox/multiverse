
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
	mv_strarr_release(&vars);
	spec.clear();
	inited = false;
}

