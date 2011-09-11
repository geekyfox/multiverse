
#include <stdlib.h>
#include "mvStrBuffer.h"
#include <stdio.h>

mvStrBuffer::mvStrBuffer(int size) :
	data((char*)malloc(sizeof(char) * size)),
	used(0),
	size(size),
	pad(0)
{
}

mvStrBuffer::~mvStrBuffer()
{
	if (data != NULL) free(data);
}

char* mvStrBuffer::release()
{
	char* result = (char*)realloc(data, sizeof(char) * (used + 1));
	result[used] = '\0';
	size = 0;
	used = 0;
	data = NULL;
	return result;
}

void mvStrBuffer::append(const char* text)
{
	int available = size - used;
	char* fill = data + used;
	while (*text != '\0') {
		int spc = *text == '\n' ? pad : 0;
		int req = spc + 1;
		if (available <= req) {
			int sizenow = size;
			size += size + req;
			data = (char*)realloc(data, sizeof(char) * size);
			fill = data + sizenow;
			available = size - sizenow;
		}
		*(fill++) = *(text++);
		available--;
		for (; spc > 0; spc--) {
			*(fill++) = ' ';
			available--;
		} 
	}
	used = fill - data;
}

void mvStrBuffer::append(int number)
{
	char text[100];
	sprintf(text, "%d", number);
	append(text);
}

