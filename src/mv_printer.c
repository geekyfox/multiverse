
#include <string.h>
#include "multiverse.h"

inline static void __appendimpl(mv_strbuf* buf, char* text) {
	int available = buf->size - buf->used;
	char* fill = buf->data + buf->used;
	while (*text != '\0') {
		if (available == 0) {
			available = buf->size;
			buf->size *= 2;
			buf->data = realloc(buf->data, sizeof(char) * buf->size);
			fill = buf->data + available;
		}
		*(fill++) = *(text++);
		available--;
	}
	buf->used = fill - buf->data;
}

inline static void __appendimpli(mv_strbuf* buf, int index) {
	char text[100];
	sprintf(text, "%d", index);
	__appendimpl(buf, text);
}

void mv_strbuf_append(mv_strbuf* buf, char* text) {
	__appendimpl(buf, text);
}

void mv_strbuf_appendi(mv_strbuf* buf, int index) {
	__appendimpli(buf, index);
}

void mv_attr_show(mv_strbuf* buf, mv_attr* attr) {
	__appendimpl(buf, attr->name);
	__appendimpl(buf, " = ");
	switch (attr->type) {
	case MVTYPE_STRING:
		__appendimpl(buf, "'");
		__appendimpl(buf, attr->value.string);
		__appendimpl(buf, "'");
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

void mv_attrlist_show(mv_strbuf* buf, mv_attrlist* ptr) {
	int i;
	__appendimpl(buf, "{\n");
	for (i=0; i<ptr->size; i++) {
		__appendimpl(buf, "  ");
		mv_attr_show(buf, &(ptr->attrs[i]));
		if (i != ptr->size - 1) __appendimpl(buf, ",");
		__appendimpl(buf, "\n");
	}
	__appendimpl(buf, "}\n");
}

void mv_attrspec_show(mv_strbuf* buf, mv_attrspec* spec) {
	switch(spec->type) {
	case MVSPEC_TYPE:
		__appendimpl(buf, spec->name);
		__appendimpl(buf, " : ");
		mv_typespec_show(buf, &(spec->value.typespec));
		break;
	default:
		DIE("Invalid code (%d)", spec->type);
	}
}

void mv_class_show(mv_strbuf* buf, mv_class* cls) {
	__appendimpl(buf, "class ");
	mv_speclist_show(buf, &(cls->data));
}

void mv_entity_show(mv_strbuf* buf, mv_entity* obj) {
	__appendimpl(buf, "entity ");
	mv_attrlist_show(buf, &(obj->data));
}

void mv_speclist_show(mv_strbuf* buf, mv_speclist* ptr) {
	int i;
	__appendimpl(buf, "{\n");
	for (i=0; i<ptr->size; i++) {
		__appendimpl(buf, "  ");
		mv_attrspec_show(buf, &(ptr->specs[i]));
		if (i != ptr->size - 1) __appendimpl(buf, ",");
		__appendimpl(buf, "\n");
	}
	__appendimpl(buf, "}\n");
}

void mv_typespec_show(mv_strbuf* buf, mv_typespec* spec) {
	switch (spec->type) {
	case MVTYPE_STRING:
		__appendimpl(buf, "string");
		break;
	default:
		DIE("Invalid code (%d)\n", spec->type);
	}
}

