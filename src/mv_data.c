
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
	case MVTYPE_STRING:
		dst->value.string = strdup(src->value.string);
		break;
	case MVTYPE_RAWREF:
		dst->value.rawref = strdup(src->value.rawref);
		break;
	case MVTYPE_REF:
		dst->value.ref = src->value.ref;
		break;
	case MVTYPE_INTEGER:
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
	case MVTYPE_STRING:
		free(attr->value.string);
		break;
	case MVTYPE_RAWREF:
		free(attr->value.rawref);
		break;
	case MVTYPE_REF:
	case MVTYPE_INTEGER:
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

void mv_attrspec_release(mv_attrspec* ptr) {
	switch (ptr->type) {
	case MVSPEC_TYPE:
		mv_typespec_release(&(ptr->value.typespec));
		break;
	case MVSPEC_SUBQUERY:
		mv_query_release(&(ptr->value.subquery));
		break;
	default:
		DIE("Unknown type (%d)", ptr->type);
	}
	free(ptr->name);
}

void mv_clscache_alloc(mv_clscache* ptr, int size) {
	ptr->items = (mv_class*)malloc(sizeof(mv_class) * size);
	ptr->size = size;
	ptr->used = 0;
}

void mv_clscache_put(mv_clscache* ptr, int* ref, mv_speclist* obj) {
	int i, index = -1;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i].exist == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		if (ptr->used == ptr->size) {
			ptr->size *= 2;
			ptr->items = (mv_class*)realloc(ptr->items, sizeof(mv_class) * ptr->size);
		}
		index = ptr->used;
		ptr->used++;
	}
	ptr->items[index].exist = 1;
	ptr->items[index].data = *obj;

	if (ref != NULL) *ref = index;
}

void mv_clscache_release(mv_clscache* ptr) {
	int i;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i].exist) {
			mv_speclist_release(&ptr->items[i].data);
		}
	}
	free(ptr->items);
}
	
void mv_entcache_alloc(mv_entcache* ptr, int size) {
	ptr->items = (mv_entity*)malloc(sizeof(mv_entity) * size);
	ptr->size = size;
	ptr->used = 0;
}

void mv_entcache_put(mv_entcache* ptr, int* ref, mv_entity* obj) {
	int i, index = -1;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i].exist == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		if (ptr->used == ptr->size) {
			ptr->size *= 2;
			size_t newsz = sizeof(mv_entity) * ptr->size;
			ptr->items = (mv_entity*)realloc(ptr->items, newsz);
		}
		index = ptr->used;
		ptr->used++;
	}
	ptr->items[index].exist = 1;
	ptr->items[index].data = obj->data;
	ptr->items[index].classes = obj->classes;

	if (ref != NULL) *ref = index;
}

void mv_entcache_release(mv_entcache* ptr) {
	int i;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i].exist) {
			mv_entity_release(&ptr->items[i]);
		}
	}
	free(ptr->items);
}

void mv_entity_alloc(mv_entity* entity, int attrs, int classes) {
	mv_attrlist_alloc(&(entity->data), attrs);
	mv_strarr_alloc(&(entity->classes), classes);
}

void mv_entity_release(mv_entity* entity) {
	mv_attrlist_release(&(entity->data));
	mv_strarr_release(&(entity->classes));
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

void mv_command_release(mv_command* sess) {
	mv_attrlist_release(&sess->attrs);
	mv_strarr_release(&sess->vars);
	mv_speclist_release(&sess->spec);
}

void mv_query_release(mv_query* query) {
	free(query->classname);
	mv_attrlist_release(&(query->attrs));
}

void mv_speclist_alloc(mv_speclist* ptr, int size) {
	ptr->specs = (mv_attrspec*)malloc(sizeof(mv_attrspec) * size);
	ptr->size = size;
}

void mv_speclist_release(mv_speclist* ptr) {
	if (ptr->specs != NULL) {
		int i;
		for (i=0; i<ptr->size; i++) {
			mv_attrspec_release(&ptr->specs[i]);
		}
		free(ptr->specs);
	}
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

void mv_strarr_appslice(mv_strarr* ptr, char* source, int first, int last) {
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

char* mv_strbuf_align(mv_strbuf* buf) {
	buf->size = buf->used + 1;
	buf->data = (char*)realloc(buf->data, sizeof(char) * buf->size);
	buf->data[buf->used] = '\0';
	return buf->data;
}

void mv_strbuf_alloc(mv_strbuf* buf, int size) {
	buf->data = (char*)malloc(sizeof(char) * size);
	buf->used = 0;
	buf->size = size;
	buf->pad  = 0;
}

void mv_typespec_release(mv_typespec* spec) {
	switch (spec->type) {
	case MVTYPE_STRING:
	case MVTYPE_INTEGER:
		break;
	case MVTYPE_RAWREF:
		free(spec->classname);
		break;
	default:
		DIE("Unknown type (%d)", spec->type);
	}
}

