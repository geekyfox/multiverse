
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#include "mvMemPool.h"

inline static void __mv_attr_set__(mv_attr* dst, const mv_attr* src) {
	dst->type = src->type;
	switch (src->type) {
	case STRING:
		dst->value.string = strdup(src->value.string);
		break;
	case RAWREF:
		dst->value.rawref = strdup(src->value.rawref);
		break;
	case REF:
		dst->value.ref = src->value.ref;
		break;
	case INTEGER:
		dst->value.integer = src->value.integer;
		break;
	default:
		DIE("Invalid code (%d)", src->type);
	}
}

void mv_attr::operator=(const mv_attr& src)
{
	this->name = strdup(src.name);
	__mv_attr_set__(this, &src);
}

inline static void __mv_attr_release_value__(mv_attr* attr) {
	switch (attr->type) {
	case STRING:
		free(attr->value.string);
		break;
	case RAWREF:
		free(attr->value.rawref);
		break;
	case REF:
	case INTEGER:
		break;
	default:
		DIE("Invalid code (%d)", attr->type);
	}
}

mv_attr::~mv_attr()
{
	__mv_attr_release_value__(this);
	free(this->name);
}

void mv_attr_update(mv_attr* dst, mv_attr* src) {
	__mv_attr_release_value__(dst);
	__mv_attr_set__(dst, src);
}

void mv_attrlist::copy_from(const mv_attrlist& src)
{
	alloc(src.size());
	for (int i=0; i<src.size(); i++) {
		(*this)[i] = src[i];
	}
}

mv_attr::mv_attr()
{
}

mv_attr::mv_attr(const mv_attr& src)
{
	(*this) = src;
}

mvStrBuffer& operator<<(mvStrBuffer& buf, const mv_attr& attr)
{
	buf << attr.name << " = ";
	switch (attr.type) {
	case STRING:
		buf << "'" << attr.value.string << "'";
		break;
	case RAWREF:
		buf << attr.value.rawref << " (UNRESOLVED)";
		break;
	case REF:
		buf << "##" << (attr.value.ref);
		break;
	case INTEGER:
		buf << (attr.value.integer);
		break;
	default:
		DIE("Invalid code (%d)", attr.type);
	}
	return buf;
}

mv_strbuf& operator<<(mv_strbuf& buf, const mv_attrlist& ptr)
{
	buf.append("{\n");
	for (int i=0; i<ptr.size(); i++) {
		buf << "  " << ptr[i];
		if (i != ptr.size() - 1) buf.append(",");
		buf << "\n";
	}
	buf.append("}");
	return buf;
}


