
#ifndef __MULTIVERSE_COMMAND_HEADER__
#define __MULTIVERSE_COMMAND_HEADER__

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
	mvCommand() : inited(false)
	{
	}
	~mvCommand();
	// Command's code, should be one of MVCMD_ codes.
	mvCommandType code;
	// Attributes of the command.
	mv_attrlist attrs;
	// Attribute specifications of the command.
	mv_speclist spec;
	// Variables of the command.
	mv_strarr vars;
	void init_done() { inited = true; }
	void destroy();
};

typedef mvCommand mv_command;

#endif
