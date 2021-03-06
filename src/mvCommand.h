
#ifndef __MULTIVERSE_COMMAND_HEADER__
#define __MULTIVERSE_COMMAND_HEADER__

#include "mvAttrSpec.h"

enum mvCommandType {
	DO_NOTHING,
	QUIT,
	ASSIGN,
	CREATE_CLASS,
	CREATE_ENTITY,
	DESTROY_ENTITY,
	LOOKUP,
	SHOW,
	UPDATE_ENTITY
};

// Multiverse can execute commands. So, there's
// a special datatype for commands as well.
class mvCommand {
public:
	bool inited;
	mvCommand() : inited(false), spec(0), vars(10)
	{
	}
	mvCommand(const char* request);
	~mvCommand();
	// Command's code, should be one of MVCMD_ codes.
	mvCommandType code;
	// Attributes of the command.
	mvAttrlist attrs;
	// Attribute specifications of the command.
	mvSpecList spec;
	// Variables of the command.
	mvStrArray vars;
	void init_done() { inited = true; }
	void destroy();
	void operator=(mvCommandType type);
};

#endif

