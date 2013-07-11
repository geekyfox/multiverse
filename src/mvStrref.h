
#ifndef __MULTIVERSE_STRREF_HEADER__
#define __MULTIVERSE_STRREF_HEADER__

#include "mvArray.h"

enum mvStrrefAlloc { WILD, POOLED, OWN };

class mvStrref {
public:
	const char* ptr;
private:
	mvStrref(char* value);
	void clear();
	int* ctr;
	mvStrrefAlloc alc;
public:
	mvStrref();
	mvStrref(const mvStrref& ref);
	mvStrref(const char* source, int first, int last);
	~mvStrref();
	void operator= (const char* value);
	void operator= (const mvStrref& ref);
	bool operator== (const char* value) const;
	bool operator!= (const char* value) const
	{
		return !((*this) == value);
	}
	bool operator== (const mvStrref& value) const;
	bool operator!= (const mvStrref& value) const
	{
		return !((*this) == value);
	}
	void* operator new(size_t size);
	void operator delete(void* ptr);
	void adopt(const char* text);
};

class mvStrArray : public mvDynamicArray<mvStrref>
{
public:
	mvStrArray(int size) : mvDynamicArray<mvStrref>(size)
	{
	}
	void append(const char* value);
	void append(const char* source, int first, int last);
};

#endif
