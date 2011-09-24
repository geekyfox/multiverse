
#include <stdio.h>
#include <string.h>

#include "mvMemPool.h"
#include "mvStrref.h"

mv_strref::mv_strref() :
	ptr(NULL), ctr(NULL), alc(WILD)
{
}

mv_strref::mv_strref(const mv_strref& ref) :
	ptr(ref.ptr), ctr(ref.ctr), alc(ref.alc)
{
	if (ctr != NULL) (*ctr)++;
}

mv_strref::mv_strref(char* value) :
	ptr(value), ctr(NULL), alc(WILD)
{
}

mv_strref::~mv_strref()
{
	if (ptr != NULL && ctr != NULL) clear();
}

mv_strref& mv_strref::operator= (char* value)
{
	clear();
	this->ptr = value;
	this->ctr = NULL;
	this->alc = WILD;
}

mv_strref& mv_strref::operator= (const mv_strref& ref)
{
	clear();
	this->ptr = ref.ptr;
	this->ctr = ref.ctr;
	this->alc = ref.alc;

	if (ctr != NULL) (*ctr)++;
}

void mv_strref::clear()
{
	if (ptr == NULL) return;
	if (ctr == NULL) return;
	int x = *ctr;
	if (x == 1) {
		if (alc == OWN) {
			free(ptr);
		} else if (alc == POOLED) {
			localStrPool.release(ptr);
		}

		localIntPool.release(ctr);
	} else {
		*ctr = x - 1;
	}
}

mv_strref::mv_strref(const char* source, int first, int last) {
	int len = last - first;
	ctr = localIntPool.acquire(1);
	if (len >= 16) {
		ptr = (char*)malloc(sizeof(char) * (len + 1));
		alc = OWN;
	} else {
		ptr = localStrPool.get();
		alc = POOLED;
	}
	strncpy(ptr, source + first, len);
	ptr[len] = '\0';
}

static mvMemPool<mv_strref, 512, 1> refPool;

void* mv_strref::operator new(size_t size)
{
	return refPool.get();
}

void mv_strref::operator delete(void* ptr)
{
	refPool.release((mv_strref*)ptr);
}

void mv_strarr::append(char* value)
{
	mv_strref ref;
	ref = value;
	push(ref);
}

void mv_strarr::append(const char* source, int first, int last)
{
	mv_strref ref(source, first, last);
	push(ref);
}

