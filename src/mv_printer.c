
#include <string.h>
#include "multiverse.h"

#define __appendimpl(x, y) x->append(y)
#define __appendimpli(x, y) x->append(y)

void mv_attr_show(mv_strbuf* buf, mv_attr* attr) {
	buf->append(attr->name);
	buf->append(" = ");
	switch (attr->type) {
	case MVTYPE_STRING:
		__appendimpl(buf, "'");
		__appendimpl(buf, attr->value.string);
		__appendimpl(buf, "'");
		break;
	case MVTYPE_RAWREF:
		__appendimpl(buf, attr->value.rawref);
		__appendimpl(buf, " (UNRESOLVED)");
		break;
	case MVTYPE_REF:
		__appendimpl(buf, "##");
		__appendimpli(buf, attr->value.ref);
		break;
	case MVTYPE_INTEGER:
		__appendimpli(buf, attr->value.integer);
		break;
	default:
		DIE("Invalid code (%d)", attr->type);
	}
}

void mv_attrlist_show(mv_strbuf* buf, const mv_attrlist& ptr) {
	int i;
	__appendimpl(buf, "{\n");
	for (i=0; i<ptr.size; i++) {
		__appendimpl(buf, "  ");
		mv_attr_show(buf, &(ptr.attrs[i]));
		if (i != ptr.size - 1) __appendimpl(buf, ",");
		__appendimpl(buf, "\n");
	}
	__appendimpl(buf, "}");
}

void mv_attrspec_show(mv_strbuf* buf, mv_attrspec* spec) {
	switch(spec->type) {
	case TYPE:
		__appendimpl(buf, spec->name);
		__appendimpl(buf, " : ");
		mv_typespec_show(buf, &(spec->value.typespec));
		break;
	case SUBQUERY:
		__appendimpl(buf, spec->name);
		__appendimpl(buf, " = ");
		mv_query_show(buf, &(spec->value.subquery));
		break;
	default:
		DIE("Invalid code (%d)", spec->type);
	}
}

void mv_class_show(mv_strbuf* buf, mv_class* cls) {
	__appendimpl(buf, "class ");
	mv_speclist_show(buf, cls->data);
}

void mv_query_show(mv_strbuf* buf, mv_query* query) {
	__appendimpl(buf, query->classname);
	__appendimpl(buf, " with ");
	buf->indent(2);
	mv_attrlist_show(buf, query->attrs);
	buf->unindent(2);
}

void mv_speclist_show(mv_strbuf* buf, mv_speclist& ptr) {
	int i;
	__appendimpl(buf, "{\n");
	for (i=0; i<ptr.size(); i++) {
		__appendimpl(buf, "  ");
		mv_attrspec_show(buf, &(ptr[i]));
		if (i != ptr.size() - 1) __appendimpl(buf, ",");
		__appendimpl(buf, "\n");
	}
	__appendimpl(buf, "}\n");
}

void mv_typespec_show(mv_strbuf* buf, mv_typespec* spec) {
	switch (spec->type) {
	case MVTYPE_STRING:
		__appendimpl(buf, "string");
		break;
	case MVTYPE_RAWREF:
		__appendimpl(buf, spec->classname);
		__appendimpl(buf, " (UNRESOLVED)");
		break;
	default:
		DIE("Invalid code (%d)\n", spec->type);
	}
}

