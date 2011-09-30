
#ifndef __MULTIVERSE_STRREF_HEADER__
#define __MULTIVERSE_STRREF_HEADER__

#include "mvArray.h"

enum mvStrrefAlloc { WILD, POOLED, OWN };

class mvStrref {
private:
	mvStrrefAlloc alc;
	mvStrref(char* value);
	void clear();
	int* ctr;
public:
	const char* ptr;
	mvStrref();
	mvStrref(const mvStrref& ref);
	mvStrref(const char* source, int first, int last);
	~mvStrref();
	mvStrref& operator= (const char* value);
	mvStrref& operator= (const mvStrref& ref);
	bool operator== (const char* value) const;
	void* operator new(size_t size);
	void operator delete(void* ptr);
};

class mv_strarr : public mvDynamicArray<mvStrref>
{
public:
	mv_strarr(int size) : mvDynamicArray<mvStrref>(size)
	{
	}
	void append(const char* value);
	void append(const char* source, int first, int last);
};

#endif
