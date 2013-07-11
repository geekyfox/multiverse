
#include <test.h>
#include "mvParser.h"

TEST compile_REQ11() {
	mvCommand cmd;

	try
	{
		singletonParser.parse(cmd, REQ11);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
	mvQuery patt(cmd);

	ASSERT_STRREF(patt.classname, "person");
	ASSERT_INT(patt.attrs.size(), 1);
	ASSERT_STRREF(patt.attrs[0].name, "name");
	ASSERT_INT(patt.attrs[0].type, STRING);
	ASSERT_STRING(patt.attrs[0].value.string, "Umberto Eco");
}

TEST match_REQ11() {
	mvEntity entity(1, 1);
	mvQuery patt;

	singletonParser.parse(entity.data[0], "name", "'Umberto Eco");
	entity.classes.append("person");

	patt.classname = "person";
	patt.attrs.alloc(1);
	singletonParser.parse(patt.attrs[0], "name", "'Umberto Eco");

	int match = patt.match(entity);
	ASSERT_INT(match, 1);
}	

TEST compile_REQ25() {
	mvCommand cmd;

	try
	{
		singletonParser.parse(cmd, REQ25);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}
	mvQuery patt(cmd);

	ASSERT_NULL(patt.classname);
	ASSERT_INT(patt.attrs.size(), 1);
	ASSERT_STRREF(patt.attrs[0].name, "name");
	ASSERT_INT(patt.attrs[0].type, STRING);
	ASSERT_STRING(patt.attrs[0].value.string, "Umberto Eco");
}

TEST match_REQ25() {
	mvEntity entity(1, 1);
	mvQuery patt;

	singletonParser.parse(entity.data[0], "name", "'Umberto Eco");
	entity.classes.append("person");

	patt.classname = NULL;
	patt.attrs.alloc(1);
	singletonParser.parse(patt.attrs[0], "name", "'Umberto Eco");

	int match = patt.match(entity);
	ASSERT_INT(match, 1);
}	

