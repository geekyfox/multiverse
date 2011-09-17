
#include <string.h>
#include "multiverse.h"

mv_error* __check_type__(char* name, mv_typespec spec, mv_attrlist attrs) {
	int i;
	for (i=attrs.size - 1; i >= 0; i--) {
		if (!STREQ(attrs.attrs[i].name, name)) continue;
		switch (spec.type) {
		case MVTYPE_STRING:
			if (attrs.attrs[i].type != MVTYPE_STRING) {
				THROW(INVALID, "Attribute %s is not string\n", name);	
			}
			break;
		case MVTYPE_INTEGER:
			if (attrs.attrs[i].type != MVTYPE_INTEGER) {
				THROW(INVALID, "Attribute %s is not integer\n", name);	
			}
			break;
		default:
			DIE("Unknown typespec code = %d\n", spec.type);
		}
		return NULL;
	}
	THROW(INVALID, "Attribute %s not found in object", name);
}

mv_error* __check_absent__(char* name, mv_attrlist attrs) {
	int i;
	for (i=attrs.size - 1; i >= 0; i--) {
		if (STREQ(attrs.attrs[i].name, name)) {
			THROW(INVALID, "Custom attribute for %s\n", name);
		}
	}
	return NULL;
}

mv_error* mv_validate_assign(mv_entity* entity, mv_class* cls) {
	for (int i=cls->data.size() - 1; i >= 0; i--) {
		mv_attrspec* spec = &(cls->data[i]);
		char* name = spec->name;
		switch (spec->type) {
		case TYPE:
			FAILRET(
				__check_type__(name, spec->value.typespec, entity->data)
			);
			break;
		case SUBQUERY:
			FAILRET(__check_absent__(name, entity->data));
			break;
		default:
			DIE("Unknown attrspec code = %d\n", spec->type);
		}
	}
	return NULL;
}


