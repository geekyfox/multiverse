
#include "test.h"

#define BEFORE(RQ) \
mv_command action; FAILFAST(mv_command_parse(&action, RQ));

#define AFTER mv_command_release(&action);

TESTREQ 1 {
	ASSERT_INT(action.code, MVCMD_CREATE_ENTITY);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.vars.used, 1);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "umberto_eco");
	ASSERT_INT(action.attrs.size, 1);
	ASSERT_INT(action.attrs.attrs[0].type, MVTYPE_STRING);
	ASSERT_STRING(action.attrs.attrs[0].name, "name");
	ASSERT_STRING(action.attrs.attrs[0].value.string, "Umberto Eco");
}

TESTREQ 7 {
	ASSERT_INT(action.code, MVCMD_CREATE_CLASS);
	ASSERT_INT(action.attrs.size, 0);
	ASSERT_NULL(action.attrs.attrs);
	ASSERT_INT(action.spec.size, 1);
	ASSERT_NOTNULL(action.spec.specs);
}

TESTREQ 10 {
	ASSERT_INT(action.code, MVCMD_ASSIGN);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.vars.used, 2);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "person");
	ASSERT_STRING(action.vars.items[1], "umberto_eco");
	ASSERT_INT(action.attrs.size, 0);
	ASSERT_NULL(action.attrs.attrs);
}

TESTREQ 11 {
	ASSERT_INT(action.code, MVCMD_LOOKUP);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.vars.used, 1);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "person");
	ASSERT_INT(action.attrs.size, 1);
	ASSERT_NOTNULL(action.attrs.attrs);
	ASSERT_STRING(action.attrs.attrs[0].name, "name");
	ASSERT_INT(action.attrs.attrs[0].type, MVTYPE_STRING);
	ASSERT_STRING(action.attrs.attrs[0].value.string, "Umberto Eco");
}

TESTREQ 12 {
	ASSERT_INT(action.code, MVCMD_DESTROY_ENTITY);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.vars.used, 1);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "umberto_eco");
	ASSERT_INT(action.attrs.size, 0);
	ASSERT_NULL(action.attrs.attrs);
}

TESTREQ 13 {
	ASSERT_INT(action.code, MVCMD_LOOKUP);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.vars.used, 1);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "person");
	ASSERT_INT(action.attrs.size, 0);
	ASSERT_NULL(action.attrs.attrs);
}

TESTREQ 14 {
	ASSERT_INT(action.code, MVCMD_CREATE_ENTITY);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.vars.used, 1);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "eiffel_tower");
	ASSERT_INT(action.attrs.size, 1);
	ASSERT_INT(action.attrs.attrs[0].type, MVTYPE_INTEGER);
	ASSERT_STRING(action.attrs.attrs[0].name, "height");
	ASSERT_INT(action.attrs.attrs[0].value.integer, 324);
}

TESTREQ 15 {
	ASSERT_INT(action.code, MVCMD_CREATE_CLASS);
	ASSERT_INT(action.attrs.size, 0);
	ASSERT_NULL(action.attrs.attrs);
	ASSERT_INT(action.spec.size, 1);
	ASSERT_NOTNULL(action.spec.specs);
	mv_attrspec asp = action.spec.specs[0];
	ASSERT_INT(asp.type, MVSPEC_TYPE);
	ASSERT_STRING(asp.name, "height");
	ASSERT_INT(asp.value.typespec.type, MVTYPE_INTEGER);
	ASSERT_NULL(asp.value.typespec.classname);
}

TESTREQ 18 {
	ASSERT_INT(action.code, MVCMD_CREATE_CLASS);
	ASSERT_INT(action.spec.size, 1);
	ASSERT_NOTNULL(action.spec.specs);
	mv_attrspec asp = action.spec.specs[0];
	ASSERT_INT(asp.type, MVSPEC_SUBQUERY);
	ASSERT_STRING(asp.name, "books");
	ASSERT_STRING(asp.value.subquery.classname, "book");
}

TESTREQ 20 {
	ASSERT_INT(action.code, MVCMD_UPDATE_ENTITY);
	ASSERT_INT(action.spec.size, 0);
	ASSERT_NULL(action.spec.specs);
	ASSERT_INT(action.attrs.size, 1);
	ASSERT_NOTNULL(action.attrs.attrs);
	ASSERT_INT(action.vars.used, 1);
	ASSERT_NOTNULL(action.vars.items);
	ASSERT_STRING(action.vars.items[0], "eiffel_tower");
}



