
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

void mvCommand::operator=(mvCommandType cmdtype)
{
	this->code = cmdtype;
	this->attrs.clear();
	this->spec.clear();
	this->vars.clear();
}

