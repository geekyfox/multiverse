
#include <test.h>
#include "mvParser.h"

TEST compile_REQ11() {
	mv_command cmd;

	try
	{
		mv_command_parse(cmd, REQ11);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	mvQuery patt(cmd);

	ASSERT_STRING(patt.classname, "person");
	ASSERT_INT(patt.attrs.size(), 1);
	ASSERT_STRING(patt.attrs[0].name, "name");
	ASSERT_INT(patt.attrs[0].type, STRING);
	ASSERT_STRING(patt.attrs[0].value.string, "Umberto Eco");
}

TEST match_REQ11() {
	mv_entity entity(1, 1);
	mvQuery patt;

	singletonParser.parse(entity.data[0], "name", "'Umberto Eco");
	entity.classes.append("person");

	patt.classname = strdup("person");
	patt.attrs.alloc(1);
	singletonParser.parse(patt.attrs[0], "name", "'Umberto Eco");

	int match = patt.match(entity);
	ASSERT_INT(match, 1);
}	


