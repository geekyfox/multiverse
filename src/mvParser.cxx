
#include <string.h>
#include <ctype.h>
#include "mvParser.h"
#include "parser.h"

inline static int __parse_number__(mv_attr& target, const char* value) {
	int ival = 0;
	if (*value == '\0') return 0;
	while (1) {
		if (isdigit(*value)) {
			ival = ival * 10 + (*value - '0');
			value++;
			continue;
		} else if (*value == '\0') {
			target.type = INTEGER;
			target.value.integer = ival;
			return 1;
		} else {
			return 0;
		}
	}
}

void mvParser::parse(mv_attr& target, const char* name, const char* value) {
	target.name = strdup(name);
	if (value[0] == '\'') {
		target.type = STRING;
		target.value.string = strdup(value + 1);
		return;
	}
	if (__parse_number__(target, value)) {
		return;
	}
	target.type = RAWREF;
	target.value.rawref = strdup(value);
}

void mvParser::parse(mvCommand& target, const char* data)
throw (mv_error*)
{
	mvAst ast(data);
	target.destroy();
	if (ast.size() == 0 || ast[0] != Leaf)
	{
		NEWTHROW(SYNTAX, "Syntax error");
	}
	mvStrref& cmdname = ast[0].leaf();

	if (cmdname == "lookup")
	{
		_parse_lookup(target, ast);
		return;
	}
	mv_command_parse(target, cmdname, ast);
}

void mvParser::_parse_lookup(mvCommand& cmd, const mvAst& ast)
throw (mv_error*)
{
	if (ast[1] != Leaf)
	{
		NEWTHROW(SYNTAX, "Syntax error");
	}
	bool set_class = (ast[1] != "entity");
	bool set_attrs = (ast.size() != 2);
	if (set_attrs)
	{
		if (ast[2] != "with" || ast[3] != AttrList)
		{
			NEWTHROW(SYNTAX, "Syntax error");
		}
	}
	cmd = LOOKUP;
	if (set_class)
	{
		cmd.vars.push(ast[1].leaf());
	}
	if (set_attrs)
	{
		ast[3].subtree().populate(cmd.attrs);
	}
}

mvParser singletonParser;

