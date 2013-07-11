
#include <stdio.h>
#include <string.h>

#include "mvMemPool.h"
#include "mvStrref.h"

mvStrref::mvStrref() :
	ptr(NULL), ctr(NULL), alc(WILD)
{
}

mvStrref::mvStrref(const mvStrref& ref) :
	ptr(ref.ptr), ctr(ref.ctr), alc(ref.alc)
{
	if (ctr != NULL) (*ctr)++;
}

mvStrref::mvStrref(char* value) :
	ptr(value), ctr(NULL), alc(WILD)
{
}

mvStrref::~mvStrref()
{
	if (ptr != NULL && ctr != NULL) clear();
}

void mvStrref::operator= (const char* value)
{
	clear();
	this->ptr = value;
	this->ctr = NULL;
	this->alc = WILD;
}

void mvStrref::operator= (const mvStrref& ref)
{
	clear();
	this->ptr = ref.ptr;
	this->ctr = ref.ctr;
	this->alc = ref.alc;

	if (ctr != NULL) (*ctr)++;
}

bool mvStrref::operator== (const char* value) const
{
	return strcmp(ptr, value) == 0;
}

bool mvStrref::operator== (const mvStrref& value) const
{
	return strcmp(ptr, value.ptr) == 0;
}

void mvStrref::clear()
{
	if (ptr == NULL) return;
	if (ctr == NULL) return;
	int x = *ctr;
	if (x == 1) {
		if (alc == OWN) {
			free((void*)ptr);
		} else if (alc == POOLED) {
			localStrPool.release((char*)ptr);
		}

		localIntPool.release(ctr);
	} else {
		*ctr = x - 1;
	}
}

mvStrref::mvStrref(const char* source, int first, int last) {
	int len = last - first;
	ctr = localIntPool.acquire(1);
	char* target;
	if (len >= 16) {
		target = (char*)malloc(sizeof(char) * (len + 1));
		alc = OWN;
	} else {
		target = localStrPool.get();
		alc = POOLED;
	}
	strncpy(target, source + first, len);
	target[len] = '\0';
	ptr = target;	
}

void mvStrref::adopt(const char* text)
{
	clear();
	ctr = localIntPool.acquire(1);
	char* target;
	int len = strlen(text);
	if (len >= 16)
	{
		target = (char*)malloc(sizeof(char) * (len + 1));
		alc = OWN;
	}
	else
	{
		target = localStrPool.get();
		alc = POOLED;
	}
	strcpy(target, text);
	ptr = target;
}

static mvMemPool<mvStrref, 512, 1> refPool;

void* mvStrref::operator new(size_t size)
{
	return refPool.get();
}

void mvStrref::operator delete(void* ptr)
{
	refPool.release((mvStrref*)ptr);
}

void mvStrArray::append(const char* value)
{
	mvStrref ref;
	ref = value;
	push(ref);
}

void mvStrArray::append(const char* source, int first, int last)
{
	mvStrref ref(source, first, last);
	push(ref);
}

