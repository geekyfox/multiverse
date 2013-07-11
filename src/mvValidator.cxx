
#include <string.h>
#include "multiverse.h"
#include "mvValidator.h"

mvError* __check_type__(const char* name, const mvTypeSpec& spec, const mvAttrlist& attrs) {
	int i;
	for (i=attrs.size() - 1; i >= 0; i--) {
		if (attrs[i].name != name) continue;
		switch (spec.type) {
		case STRING:
			if (attrs[i].type != STRING) {
				THROW(INVALID, "Attribute %s is not string\n", name);	
			}
			break;
		case INTEGER:
			if (attrs[i].type != INTEGER) {
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

mvError* __check_absent__(const char* name, const mvAttrlist& attrs) {
	int i;
	for (i=attrs.size() - 1; i >= 0; i--) {
		if (attrs[i].name == name) {
			THROW(INVALID, "Custom attribute for %s\n", name);
		}
	}
	return NULL;
}

void mvValidator::validate(const mvEntity& enty,
                           const mvClass& cls)
throw (mvError*)
{
	for (int i=cls.data.size() - 1; i >= 0; i--) {
		mvAttrSpec* spec = &(cls.data[i]);
		const char* name = spec->name.ptr;
		switch (spec->get_type()) {
		case TYPE:
			FAILTHROW(
				__check_type__(name, spec->typespec(), enty.data)
			);
			break;
		case SUBQUERY:
			FAILTHROW(__check_absent__(name, enty.data));
			break;
		default:
			DIE("Unknown attrspec code = %d\n", spec->get_type());
		}
	}
}

mvValidator singletonValidator;


