
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#include "mvMemPool.h"

inline static void __mvAttr_set__(mvAttr* dst, const mvAttr* src) {
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

void mvAttr::operator=(const mvAttr& src)
{
	this->name = strdup(src.name);
	__mvAttr_set__(this, &src);
}

inline static void __mvAttr_release_value__(mvAttr* attr) {
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

mvAttr::~mvAttr()
{
	__mvAttr_release_value__(this);
	free(this->name);
}

void mvAttr_update(mvAttr* dst, mvAttr* src) {
	__mvAttr_release_value__(dst);
	__mvAttr_set__(dst, src);
}

void mvAttrlist::copy_from(const mvAttrlist& src)
{
	alloc(src.size());
	for (int i=0; i<src.size(); i++) {
		(*this)[i] = src[i];
	}
}

mvAttr::mvAttr()
{
}

mvAttr::mvAttr(const mvAttr& src)
{
	(*this) = src;
}

mvStrBuffer& operator<<(mvStrBuffer& buf, const mvAttr& attr)
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

mvStrBuffer& operator<<(mvStrBuffer& buf, const mvAttrlist& ptr)
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


