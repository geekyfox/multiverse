
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
	mv_speclist_release(&spec);
	inited = false;
}

