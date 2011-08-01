
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

mv_attr mv_attr_copy(mv_attr* attr) {
	mv_attr result;

	result.type = attr->type;
	result.name = strdup(attr->name);
	
	switch (result.type) {
	case MVTYPE_STRING:
		result.value.string = strdup(attr->value.string);
		break;
	case MVTYPE_RAWREF:
		result.value.rawref = strdup(attr->value.rawref);
		break;
	case MVTYPE_REF:
		result.value.ref = attr->value.ref;
		break;
	case MVTYPE_INTEGER:
		result.value.integer = attr->value.integer;
		break;
	default:
		DIE("Invalid code (%d)", attr->type);
	}

	return result;
}

void mv_attr_release(mv_attr* attr) {
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
	free(attr->name);
}

void mv_attrlist_alloc(mv_attrlist* ptr, int size) {
	ptr->size = size;
	if (size == 0) {
		ptr->attrs = NULL;
	} else {
		ptr->attrs = malloc(sizeof(mv_attr) * size);
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
	ptr->items = malloc(sizeof(mv_class) * size);
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
			ptr->items = realloc(ptr->items, sizeof(mv_class) * ptr->size);
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
	ptr->items = malloc(sizeof(mv_entity) * size);
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
			ptr->items = realloc(ptr->items, newsz);
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

void mv_intset_alloc(mv_intset* ptr, int size) {
	ptr->items = malloc(sizeof(int) * size);
	ptr->size = size;
	ptr->used = 0;
}

int mv_intset_contains(mv_intset* ptr, int value) {
	int i;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i] == value) return 1;
	}
	return 0;
}

void mv_intset_put(mv_intset* ptr, int value) {
	if (mv_intset_contains(ptr, value)) return;
	if (ptr->used == ptr->size) {
		ptr->size *= 2;
		ptr->items = realloc(ptr->items, ptr->size * sizeof(int));
	}
	ptr->items[ptr->used] = value;
	ptr->used++;
}

void mv_intset_release(mv_intset* ptr) {
	free(ptr->items);
}

void mv_intset_remove(mv_intset* ptr, int value) {
	int ix = -1, i;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i] == value) {
			ix = i;
			break;
		}
	}
	if (ix != -1) {
		ptr->used--;
		if (ix < ptr->used) ptr->items[ix] = ptr->items[ptr->used];
	}
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
	ptr->specs = malloc(sizeof(mv_attrspec) * size);
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
	ptr->items = malloc(sizeof(char*) * size);
	ptr->used = 0;
	ptr->size = size;
}

static void __mv_strarr_expand(mv_strarr* ptr) {
	if (ptr->used < ptr->size) return;
	ptr->size *= 2;
	ptr->items = realloc(ptr->items, sizeof(char*) * ptr->size);
}

void mv_strarr_append(mv_strarr* ptr, char* value) {
	__mv_strarr_expand(ptr);
	ptr->items[ptr->used] = strdup(value);
	ptr->used++;
}

void mv_strarr_appslice(mv_strarr* ptr, char* source, int first, int last) {
	char* text = mv_strslice(source, first, last);
	__mv_strarr_expand(ptr);
	ptr->items[ptr->used] = text;
	ptr->used++;
}

void mv_strarr_release(mv_strarr* ptr) {
	int i;
	for (i=0; i<ptr->used; i++) {
		free(ptr->items[i]);
	}
	free(ptr->items);
}

char* mv_strbuf_align(mv_strbuf* buf) {
	buf->size = buf->used + 1;
	buf->data = realloc(buf->data, sizeof(char) * buf->size);
	buf->data[buf->used] = '\0';
	return buf->data;
}

void mv_strbuf_alloc(mv_strbuf* buf, int size) {
	buf->data = (char*)malloc(sizeof(char) * size);
	buf->used = 0;
	buf->size = size;
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

void mv_varbind_alloc(mv_varbind* ptr, int size) {
	ptr->keys = malloc(sizeof(char*) * size);
	int i;
	for (i=0; i<size; i++) ptr->keys[i] = NULL;
	ptr->values = malloc(sizeof(int) * size);
	ptr->hashes = malloc(sizeof(int) * size);
	ptr->size = size;
	ptr->used = 0;
}

inline static void __varbind_expand(mv_varbind* ptr) {
	if (ptr->size <= ptr->used * 2) {
		mv_varbind tmp;
		mv_varbind_alloc(&tmp, ptr->size * 2);
		int i;
		for (i=0; i<ptr->size; i++) {
			if (ptr->keys[i] != NULL) {
				mv_varbind_insert(&tmp, ptr->keys[i], ptr->values[i]);
			}
		}
		mv_varbind_release(ptr);
		*ptr = tmp;
	}
}

void mv_varbind_insert(mv_varbind* ptr, char* key, int value) {
	__varbind_expand(ptr);
	int hash = mv_strhash(key) % ptr->size, i, j;
	char* keyCopy = strdup(key);
	for (i=0, j=hash; i<ptr->size; i++, j++) {
		if (j == ptr->size) j = 0;
		if (ptr->keys[j] == NULL) {
			ptr->keys[j] = keyCopy;
			ptr->values[j] = value;
			ptr->hashes[j] = hash;
			ptr->used++;
			return;
		}
		if (ptr->hashes[j] < hash) continue;
		if (ptr->hashes[j] == hash) {
			if (STREQ(ptr->keys[j], key)) {
				free(keyCopy);
				ptr->values[j] = value;
				return;
			}
			continue;
		}
		char* ctmp = ptr->keys[j];
		ptr->keys[j] = keyCopy;
		keyCopy = ctmp;
		int itmp = ptr->values[j];
		ptr->values[j] = value;
		value = itmp;
		itmp = ptr->hashes[j];
		ptr->hashes[j] = hash;
		hash = itmp;
	}
}

int mv_varbind_lookup(mv_varbind* ptr, char* key) {
	int hash = mv_strhash(key) % ptr->size, i, j;
	for (i=0, j=hash; i<ptr->size; i++, j++) {
		if (j == ptr->size) j = 0;
		if ( (ptr->keys[j] == NULL) || (ptr->hashes[j] > hash) ) return -1;		
		if ( (ptr->hashes[j] == hash) && STREQ(key, ptr->keys[j]) ) {
			return ptr->values[j];
		}
	}
	assert(0);
}

void mv_varbind_remove(mv_varbind* ptr, char* key) {
	int hash = mv_strhash(key) % ptr->size, i, j;
	for (i=0, j=hash; i<ptr->size; i++, j++) {
		if (j == ptr->size) j = 0;
		if (ptr->keys[j] == NULL) return;
		if (ptr->hashes[j] < hash) continue;
		if (ptr->hashes[j] > hash) return;
		if (STREQ(key, ptr->keys[j])) break;
	}

	free(ptr->keys[j]);
	ptr->keys[j] = NULL;
	ptr->used--;
	while (1) {
		i = j;
		j++;
		if (j == ptr->size) j = 0;
		if ( (ptr->keys[j] == NULL) || (ptr->hashes[j] == j) ) return;
		ptr->keys[i] = ptr->keys[j];
		ptr->values[i] = ptr->values[j];
		ptr->hashes[i] = ptr->hashes[j];
		ptr->keys[j] = NULL;
	}
}

void mv_varbind_release(mv_varbind* ptr) {
	int i;
	for (i=0; i<ptr->size; i++) {
		if (ptr->keys[i] != NULL) free(ptr->keys[i]);
	}
	free(ptr->keys);
	free(ptr->values);
	free(ptr->hashes);
}

void mv_varbind_show(mv_varbind* ptr) {
	int i, f = 1;
	printf("{");
	for (i=0; i<ptr->size; i++) {
		if (ptr->keys[i] != NULL) {
			if (f) f=0; else printf(", ");
			printf("%s=%d", ptr->keys[i], ptr->values[i]);
		}
	}
	printf("}\n");
}

