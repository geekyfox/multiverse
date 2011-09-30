
#include "multiverse.h"
#include "mvCommand.h"

mvCommand::~mvCommand()
{
	destroy();
}

void mvCommand::destroy()
{
	if (!inited) return;
	attrs.clear();
	vars.clear();
	spec.clear();
	inited = false;
}

