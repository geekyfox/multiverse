
#include <stdio.h>
#include <stdlib.h>
#include "mvIntset.h"

mvIntset::mvIntset() {
	items = NULL;
	size = 0;
	used = 0;
}

mvIntset::mvIntset(int size) {
	items = (int*)malloc(sizeof(int) * size);
	this->size = size;
	used = 0;
}

mvIntset::~mvIntset() {
	free(items);
}

void mvIntset::clear() {
	used = 0;
}

bool mvIntset::contains(int value) {
	int i;
	for (i=0; i<used; i++) {
		if (items[i] == value) return 1;
	}
	return 0;
}

void mvIntset::put(int value) {
	if (contains(value)) return;
	if (used == size) {
		size *= 2;
		items = (int*)realloc(items, size * sizeof(int));
	}
	items[used] = value;
	used++;
}

void mvIntset::remove(int value) {
	int ix = -1, i;
	for (i=0; i<used; i++) {
		if (items[i] == value) {
			ix = i;
			break;
		}
	}
	if (ix != -1) {
		used--;
		if (ix < used) items[ix] = items[used];
	}
}


