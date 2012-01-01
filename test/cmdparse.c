
#include "test.h"
#include "mvParser.h"

#define BEFORE(RQ) \
mvCommand action; try { singletonParser.parse(action, RQ); } \
catch (mvError* err) { FAIL(err); }

#define AFTER

TESTREQ 1 {
	ASSERT_INT(action.code, CREATE_ENTITY);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.vars.size(), 1);
	ASSERT_STRREF(action.vars[0], "umberto_eco");
	ASSERT_INT(action.attrs.size(), 1);
	ASSERT_INT(action.attrs[0].type, STRING);
	ASSERT_STRING(action.attrs[0].name, "name");
	ASSERT_STRING(action.attrs[0].value.string, "Umberto Eco");
}

TESTREQ 7 {
	ASSERT_INT(action.code, CREATE_CLASS);
	ASSERT_INT(action.attrs.size(), 0);
	ASSERT_INT(action.spec.size(), 1);
}

TESTREQ 10 {
	ASSERT_INT(action.code, ASSIGN);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.vars.size(), 2);
	ASSERT_STRREF(action.vars[0], "person");
	ASSERT_STRREF(action.vars[1], "umberto_eco");
	ASSERT_INT(action.attrs.size(), 0);
}

TESTREQ 11 {
	ASSERT_INT(action.code, LOOKUP);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.vars.size(), 1);
	ASSERT_STRREF(action.vars[0], "person");
	ASSERT_INT(action.attrs.size(), 1);
	ASSERT_STRING(action.attrs[0].name, "name");
	ASSERT_INT(action.attrs[0].type, STRING);
	ASSERT_STRING(action.attrs[0].value.string, "Umberto Eco");
}

TESTREQ 25 {
	ASSERT_INT(action.code, LOOKUP);
	ASSERT_INT(action.spec.size(), 0);
    ASSERT_INT(action.vars.size(), 0);
	ASSERT_INT(action.attrs.size(), 1);
	ASSERT_STRING(action.attrs[0].name, "name");
	ASSERT_INT(action.attrs[0].type, STRING);
	ASSERT_STRING(action.attrs[0].value.string, "Umberto Eco");
}

TESTREQ 12 {
	ASSERT_INT(action.code, DESTROY_ENTITY);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.vars.size(), 1);
	ASSERT_STRREF(action.vars[0], "umberto_eco");
	ASSERT_INT(action.attrs.size(), 0);
}

TESTREQ 13 {
	ASSERT_INT(action.code, LOOKUP);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.vars.size(), 1);
	ASSERT_STRREF(action.vars[0], "person");
	ASSERT_INT(action.attrs.size(), 0);
}

TESTREQ 14 {
	ASSERT_INT(action.code, CREATE_ENTITY);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.vars.size(), 1);
	ASSERT_STRREF(action.vars[0], "eiffel_tower");
	ASSERT_INT(action.attrs.size(), 1);
	ASSERT_INT(action.attrs[0].type, INTEGER);
	ASSERT_STRING(action.attrs[0].name, "height");
	ASSERT_INT(action.attrs[0].value.integer, 324);
}

TESTREQ 15 {
	ASSERT_INT(action.code, CREATE_CLASS);
	ASSERT_INT(action.attrs.size(), 0);
	ASSERT_INT(action.spec.size(), 1);
	mvAttrSpec& asp = action.spec[0];
	ASSERT_INT(asp.get_type(), TYPE);
	ASSERT_STRREF(asp.name, "height");
	ASSERT_INT(asp.typespec().type, INTEGER);
}

TESTREQ 18 {
	ASSERT_INT(action.code, CREATE_CLASS);
	ASSERT_INT(action.spec.size(), 1);
	mvAttrSpec& asp = action.spec[0];
	ASSERT_INT(asp.get_type(), SUBQUERY);
	ASSERT_STRREF(asp.name, "books");
	ASSERT_STRING(asp.subquery().classname, "book");
}

TESTREQ 20 {
	ASSERT_INT(action.code, UPDATE_ENTITY);
	ASSERT_INT(action.spec.size(), 0);
	ASSERT_INT(action.attrs.size(), 1);
	ASSERT_INT(action.vars.size(), 1);
	ASSERT_STRREF(action.vars[0], "eiffel_tower");
}



