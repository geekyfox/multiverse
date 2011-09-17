
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#include "mvMemPool.h"

mv_strref mv_strref_alloc(int len) {
	char* ptr;
	int alc;
	
	if (len >= 16) {
		ptr = (char*)malloc(sizeof(char) * (len + 1));
		alc = -1;
	} else {
		ptr = localStrPool.get();
		alc = 16;
	}
	mv_strref ref = mv_strref_wrap(ptr);
	ref.alc = alc;
	return ref;
}

mv_strref mv_strref_wrap(char* value) {
	mv_strref result;
	result.ptr = value;
	result.ctr = localIntPool.acquire(1);
	result.alc = -1;
	return result;
}

void mv_strref_free(mv_strref* ref) {
	int x = *(ref->ctr);
	x--;
	if (x == 0) {
		if (ref->alc != 16) {
			free(ref->ptr);
		} else {
			localStrPool.release(ref->ptr);
		}

		localIntPool.release(ref->ctr);
	} else {
		*(ref->ctr) = x;
	}
}

mv_strref mv_strref_copy(mv_strref* ref) {
	mv_strref result = *ref;
	*(result.ctr) += 1;
	return result;
}

inline static void __mv_attr_set__(mv_attr* dst, mv_attr* src) {
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

mv_attr mv_attr_copy(mv_attr* attr) {
	mv_attr result;
	result.name = strdup(attr->name);
	__mv_attr_set__(&result, attr);
	return result;
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

void mv_attr_release(mv_attr* attr) {
	__mv_attr_release_value__(attr);	
	free(attr->name);
}

void mv_attr_update(mv_attr* dst, mv_attr* src) {
	__mv_attr_release_value__(dst);
	__mv_attr_set__(dst, src);
}

void mv_attrlist_alloc(mv_attrlist* ptr, int size) {
	ptr->size = size;
	if (size == 0) {
		ptr->attrs = NULL;
	} else {
		ptr->attrs = (mv_attr*)malloc(sizeof(mv_attr) * size);
	}
}

mv_attrlist mv_attrlist_copy(mv_attrlist* ptr) {
	int i;
	mv_attrlist result;
	mv_attrlist_alloc(&result, ptr->size);
	for (i=0; i<ptr->size; i++) {
		result.attrs[i] = mv_attr_copy(&(ptr->attrs[i]));
	}
	return result;
}

void mv_attrlist_realloc(mv_attrlist* ptr, int size) {
	if (ptr->size == 0) {
		mv_attrlist_alloc(ptr, size);
		return;
	}
	ptr->size = size;
	if (size == 0) {
		free(ptr->attrs);
		ptr->attrs = NULL;
		return;
	}
	ptr->attrs = (mv_attr*)realloc(ptr->attrs, sizeof(mv_attr) * size);
}

void mv_attrlist_release(mv_attrlist* ptr) {
	int i;
	for (i=0; i<ptr->size; i++) {
		mv_attr_release(&ptr->attrs[i]);
	}
	if (ptr->attrs != NULL) {
		free(ptr->attrs);
	}
}

void mv_attrspec::clear()
{
	switch (type) {
	case UNSET:
		return;
	case TYPE:
		mv_typespec_release(&(value.typespec));
		break;
	case SUBQUERY:
		mv_query_release(&(value.subquery));
		break;
	default:
		DIE("Unknown type (%d)", type);
	}
	free(name);
}

mv_error* mv_entity_update(mv_entity* enty, mv_attrlist attrs) {
	int sz = attrs.size, add = 0, i, j;
	int tmp[sz];
	int fill = enty->data.size;
	mv_attr* data = enty->data.attrs;
	for (i=0; i<sz; i++) {
		tmp[i] = -1;
		char* name = attrs.attrs[i].name;
		for (j=0; j<fill; j++) {
			if (STREQ(data[j].name, name)) {
				tmp[i] = j;
				break;
			}
		}
		if (tmp[i] == -1) add++;
	}
	if (add > 0) {
		mv_attrlist_realloc(&(enty->data), fill + add);
		data = enty->data.attrs;
	}
	for (i=0; i<sz; i++) {
		mv_attr* src = &attrs.attrs[i];
		switch (tmp[i]) {
		case -1:
			data[fill++] = mv_attr_copy(src);
			break;
		default:
			mv_attr_update(&data[tmp[i]], src);
		}
	}
	
	return NULL;
}

void mv_query_release(mv_query* query) {
	free(query->classname);
	mv_attrlist_release(&(query->attrs));
}

void mv_strarr_alloc(mv_strarr* ptr, int size) {
	if (size < 8) size = 8;
	ptr->items = (mv_strref*)malloc(sizeof(mv_strref) * size);
	ptr->used = 0;
	ptr->size = size;
}

static void __mv_strarr_expand(mv_strarr* ptr) {
	if (ptr->used < ptr->size) return;
	ptr->size *= 2;
	ptr->items = (mv_strref*)realloc(ptr->items, sizeof(mv_strref) * ptr->size);
}

void mv_strarr_append(mv_strarr* ptr, char* value) {
	__mv_strarr_expand(ptr);
	ptr->items[ptr->used] = mv_strref_wrap(value);
	ptr->used++;
}

void mv_strarr_appref(mv_strarr* ptr, mv_strref* ref) {
	__mv_strarr_expand(ptr);
	ptr->items[ptr->used] = mv_strref_copy(ref);
	ptr->used++;
}

void mv_strarr_appslice(mv_strarr* ptr, const char* source, int first, int last) {
	__mv_strarr_expand(ptr);
	ptr->items[ptr->used] = mv_strslice(source, first, last);
	ptr->used++;
}

void mv_strarr_release(mv_strarr* ptr) {
	int i;
	for (i=0; i<ptr->used; i++) {
		mv_strref_free(&ptr->items[i]);
	}
	free(ptr->items);
}

void mv_typespec_release(mv_typespec* spec) {
	switch (spec->type) {
	case STRING:
	case INTEGER:
		break;
	case RAWREF:
		free(spec->classname);
		break;
	default:
		DIE("Unknown type (%d)", spec->type);
	}
}

