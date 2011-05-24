
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include "utils.h"

void mv_attr_release(mv_attr* attr) {
	switch (attr->type) {
		case MVTYPE_STRING:
			free(attr->value.string);
			break;
		case MVTYPE_RAWREF:
			free(attr->value.rawref);
			break;
		default:
			fprintf(stderr, "code = %d\n", attr->type);
			assert(0);
	}
	free(attr->name);
}

void mv_attr_show(mv_strbuf* buf, mv_attr* attr) {
	mv_strbuf_append(buf, attr->name);
	mv_strbuf_append(buf, " = ");
	switch (attr->type) {
		case MVTYPE_STRING:
			mv_strbuf_append(buf, "'");
			mv_strbuf_append(buf, attr->value.string);
			mv_strbuf_append(buf, "'");
			break;
		default:
			printf("Unknown type %d\n", attr->type);
			assert(0);
	}
}

void mv_attrlist_alloc(mv_attrlist* ptr, int size) {
	ptr->size = size;
	ptr->attrs = (mv_attr*)malloc(sizeof(mv_attr) * size);
}

void mv_attrlist_release(mv_attrlist* ptr) {
	if (ptr->size != 0) {
		int i;
		for (i=0; i<ptr->size; i++) {
			mv_attr_release(ptr->attrs + i);
		}
		free(ptr->attrs);
	}
}

void mv_attrlist_show(mv_strbuf* buf, mv_attrlist* ptr) {
	int i;
	mv_strbuf_append(buf, "{\n");
	for (i=0; i<ptr->size; i++) {
		mv_strbuf_append(buf, "  ");
		mv_attr_show(buf, &(ptr->attrs[i]));
		if (i != ptr->size - 1) mv_strbuf_append(buf, ",");
		mv_strbuf_append(buf, "\n");
	}
	mv_strbuf_append(buf, "}\n");
}

void mv_entcache_alloc(mv_entcache* ptr, int size) {
	ptr->items = (mv_entity*)malloc(sizeof(mv_entity) * size);
	ptr->size = size;
	ptr->used = 0;
}

int mv_entcache_put(mv_entcache* ptr, mv_attrlist* obj) {
	int i, index = -1;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i].exist == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		if (ptr->used == ptr->size) {
			ptr->items = (mv_entity*)realloc(ptr->items, sizeof(mv_entity) * ptr->used * 2);
			ptr->size *= 2;
		}
		index = ptr->used;
		ptr->used++;
	}
	ptr->items[index].exist = 1;
	ptr->items[index].data = *obj;
	return index;
}

void mv_entcache_release(mv_entcache* ptr) {
	int i;
	for (i=0; i<ptr->used; i++) {
		if (ptr->items[i].exist) {
			mv_attrlist_release(&ptr->items[i].data);
		}
	}
	free(ptr->items);
}

void mv_entity_show(mv_strbuf* buf, mv_entity* obj) {
	mv_strbuf_append(buf, "entity ");
	mv_attrlist_show(buf, &(obj->data));
}

void mv_command_release(mv_command* sess) {
	mv_attrlist_release(&sess->attrs);
	mv_strarr_release(&sess->vars);
}

void mv_strarr_alloc(mv_strarr* ptr, int size) {
	if (size < 8) size = 8;
	ptr->items = (char**)malloc(sizeof(char*) * size);
	ptr->used = 0;
	ptr->size = size;
}

void mv_strarr_append(mv_strarr* ptr, char* value) {
	if (ptr->used == ptr->size) {
		ptr->items = (char**)realloc(ptr->items, sizeof(char*) * ptr->size * 2);
		ptr->size *= 2;
	}
	ptr->items[ptr->used] = strdup(value);
	ptr->used++;
}

void mv_strarr_appslice(mv_strarr* ptr, char* source, int first, int last) {
	char* text = mv_strslice(source, first, last);
	if (ptr->used == ptr->size) {
		ptr->items = (char**)realloc(ptr->items, sizeof(char*) * ptr->size * 2);
		ptr->size *= 2;
	}
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
	buf->data = (char*)realloc(buf->data, sizeof(char) * buf->size);
	buf->data[buf->used] = '\0';
	return buf->data;
}

void mv_strbuf_alloc(mv_strbuf* buf, int size) {
	buf->data = (char*)malloc(sizeof(char) * size);
	buf->used = 0;
	buf->size = size;
}

void mv_strbuf_append(mv_strbuf* buf, char* text) {
	int available = buf->size - buf->used;
	int len = strlen(text);
	if (len > available) {
		buf->size = buf->size * 2 + len;
		buf->data = (char*)realloc(buf->data, sizeof(char) * buf->size);
	}
	strcpy(buf->data + buf->used, text);
	buf->used += len;
}

void mv_varbind_alloc(mv_varbind* ptr, int size) {
	ptr->keys = (char**)malloc(sizeof(char*) * size);
	int i;
	for (i=0; i<size; i++) ptr->keys[i] = NULL;
	ptr->values = (int*)malloc(sizeof(int) * size);
	ptr->hashes = (int*)malloc(sizeof(int) * size);
	ptr->size = size;
	ptr->used = 0;
}

void mv_varbind_insert(mv_varbind* ptr, char* key, int value) {
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
			if (strcmp(ptr->keys[j], key) == 0) {
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
		if ( (ptr->hashes[j] == hash) && (strcmp(key, ptr->keys[j]) == 0) ) return ptr->values[j];
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
		if (strcmp(key, ptr->keys[j]) == 0) break;
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

