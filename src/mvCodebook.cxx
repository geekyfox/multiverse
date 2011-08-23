
#include <stdlib.h>
#include <string.h>
#include "mvCodebook.h"
#include "multiverse.h"

mvCodebook::mvCodebook(int size) {
	init(size);
}

mvCodebook::~mvCodebook() {
	if (size == 0) return;
	for (int i=0; i<size; i++) {
		if (keys[i] != NULL) free(keys[i]);
	}
	free(keys);
	free(values);
	free(hashes);
}

void mvCodebook::init(int size) {
	keys = (char**)malloc(sizeof(char*) * size);
	int i;
	for (i=0; i<size; i++) keys[i] = NULL;
	values = (int*)malloc(sizeof(int) * size);
	hashes = (int*)malloc(sizeof(int) * size);
	this->size = size;
	used = 0;
}

void mvCodebook::expand() {
	if (size > used * 2) return;
	char** keys = this->keys;
	int* values = this->values;
	int* hashes = this->hashes;
	int size = this->size;
	init(this->size * 2);
	for (int i=0; i<size; i++) {
		if (keys[i] != NULL) {
			this->push(keys[i], values[i]);
		}
	}
	free(keys);
	free(values);
	free(hashes);
}

void mvCodebook::insert(char* key, int value) {
	expand();
	push(strdup(key), value);
}

int mvCodebook::lookup(char* key) {
	int hash = mv_strhash(key) % size, i, j;
	for (i=0, j=hash; i<size; i++, j++) {
		if (j == size) j = 0;
		if ( (keys[j] == NULL) || (hashes[j] > hash) ) return -1;		
		if ( (hashes[j] == hash) && STREQ(key, keys[j]) ) {
			return values[j];
		}
	}
	abort();
}

void mvCodebook::push(char* keyCopy, int value) {
	int i, j, hash = mv_strhash(keyCopy) % size;
	for (i=0, j=hash; i<size; i++, j++) {
		if (j == size) j = 0;
		if (keys[j] == NULL) {
			keys[j] = keyCopy;
			values[j] = value;
			hashes[j] = hash;
			used++;
			return;
		}
		if (hashes[j] < hash) continue;
		if (hashes[j] == hash) {
			if (STREQ(keys[j], keyCopy)) {
				free(keyCopy);
				values[j] = value;
				return;
			}
			continue;
		}
		char* ctmp = keys[j];
		keys[j] = keyCopy;
		keyCopy = ctmp;
		int itmp = values[j];
		values[j] = value;
		value = itmp;
		itmp = hashes[j];
		hashes[j] = hash;
		hash = itmp;
	}
}

void mvCodebook::remove(char* key) {
	int hash = mv_strhash(key) % size, i, j;
	for (i=0, j=hash; i<size; i++, j++) {
		if (j == size) j = 0;
		if (keys[j] == NULL) return;
		if (hashes[j] < hash) continue;
		if (hashes[j] > hash) return;
		if (STREQ(key, keys[j])) break;
	}

	free(keys[j]);
	keys[j] = NULL;
	used--;
	while (1) {
		i = j;
		j++;
		if (j == size) j = 0;
		if ( (keys[j] == NULL) || (hashes[j] == j) ) return;
		keys[i] = keys[j];
		values[i] = values[j];
		hashes[i] = hashes[j];
		keys[j] = NULL;
	}
}
