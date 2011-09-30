
#include "multiverse.h"
#include "mvAttrSpec.h"

mv_typespec::mv_typespec(mvTypeCode type) :
	type(type), classname()
{
}

mv_typespec::mv_typespec(const mvStrref& ref) :
	type(RAWREF), classname(ref)
{
}

mv_typespec::~mv_typespec()
{
}

mvStrBuffer& operator<<(mvStrBuffer& buff, const mv_typespec& spec)
{
	switch (spec.type) {
	case STRING:
		buff << "string";
		break;
	case RAWREF:
		buff << spec.name() << " (UNRESOLVED)";
		break;
	default:
		DIE("Invalid code (%d)\n", spec.type);
	}
}

mv_attrspec::~mv_attrspec()
{
	switch (type) {
	case UNSET:
		return;
	case TYPE:
		delete value.typespec;
		break;
	case SUBQUERY:
		delete value.subquery;
		break;
	default:
		DIE("Unknown type (%d)", type);
	}
}

mvStrBuffer& operator<< (mvStrBuffer& buf, const mv_attrspec& spec)
{
	switch(spec.get_type()) {
	case TYPE:
		buf << spec.name << " : " << spec.typespec();
		break;
	case SUBQUERY:
		buf << spec.name << " = " << spec.subquery();
		break;
	default:
		DIE("Invalid code (%d)", spec.get_type());
	}
	return buf;
}

mvStrBuffer& operator<< (mvStrBuffer& buf, const mv_speclist& spec)
{
	buf << "{\n";
	for (int i=0; i<spec.size(); i++) {
		buf << "  " << spec[i];
		if (i != spec.size() - 1) buf << ",";
		buf << "\n";
	}
	buf << "}\n";
	return buf;
}

