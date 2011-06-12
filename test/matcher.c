
#include <test.h>

TEST compile_REQ11() {
	mv_command cmd;
	mv_pattern patt;

	FAILFAST(mv_command_parse(&cmd, REQ11));
	FAILFAST(mv_pattern_compile(&patt, &cmd));

	ASSERT_STRING(patt.clsname, "person");
	ASSERT_INT(patt.attrs.size, 1);
	ASSERT_STRING(patt.attrs.attrs[0].name, "name");
	ASSERT_INT(patt.attrs.attrs[0].type, MVTYPE_STRING);
	ASSERT_STRING(patt.attrs.attrs[0].value.string, "Umberto Eco");

	mv_command_release(&cmd);
	mv_pattern_release(&patt);
}

TEST match_REQ11() {
	mv_entity entity;
	mv_pattern patt;

	mv_entity_alloc(&entity, 1, 1);
	entity.exist = 1;
	mv_attr_parse(&(entity.data.attrs[0]), "name", "'Umberto Eco");
	mv_strarr_append(&(entity.classes), "person");

	patt.clsname = strdup("person");
	mv_attrlist_alloc(&(patt.attrs), 1);
	mv_attr_parse(&(patt.attrs.attrs[0]), "name", "'Umberto Eco");

	int match = mv_pattern_match(&patt, &entity);
	ASSERT_INT(match, 1);

	mv_entity_release(&entity);
	mv_pattern_release(&patt);
}	


