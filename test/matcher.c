
#include <test.h>

TEST compile_REQ11() {
	mv_command cmd;
	mv_query patt;

	try
	{
		cmd = mv_command_parse(REQ11);
	}
	catch (mv_error* err)
	{
		FAIL(err);
	}
	FAILFAST(mv_query_compile(&patt, &cmd));

	ASSERT_STRING(patt.classname, "person");
	ASSERT_INT(patt.attrs.size, 1);
	ASSERT_STRING(patt.attrs.attrs[0].name, "name");
	ASSERT_INT(patt.attrs.attrs[0].type, MVTYPE_STRING);
	ASSERT_STRING(patt.attrs.attrs[0].value.string, "Umberto Eco");

	mv_command_release(&cmd);
	mv_query_release(&patt);
}

TEST match_REQ11() {
	mv_entity entity;
	mv_query patt;

	mv_entity_alloc(&entity, 1, 1);
	entity.exist = 1;
	mv_attr_parse(&(entity.data.attrs[0]), "name", "'Umberto Eco");
	mv_strarr_append(&(entity.classes), strdup("person"));

	patt.classname = strdup("person");
	mv_attrlist_alloc(&(patt.attrs), 1);
	mv_attr_parse(&(patt.attrs.attrs[0]), "name", "'Umberto Eco");

	int match = mv_query_match(&patt, &entity);
	ASSERT_INT(match, 1);

	mv_entity_release(&entity);
	mv_query_release(&patt);
}	


