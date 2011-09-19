
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
	ptr(value), ctr(localIntPool.acquire(1)), alc(WILD)
{
}

mv_strref& mv_strref::operator= (char* value)
{
	clear();
	this->ptr = value;
	this->ctr = localIntPool.acquire(1);
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

void mv_strref::set(const char* source, int first, int last) {
	clear();
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

void mv_strarr::append(char* value)
{
	mv_strref ref;
	ref = value;
	push(ref);
}

