
#ifndef __MULTIVERSE_STRREF_HEADER__
#define __MULTIVERSE_STRREF_HEADER__

#include "mvArray.h"

enum mvStrrefAlloc { WILD, POOLED, OWN };

class mv_strref {
private:
	mvStrrefAlloc alc;
	mv_strref(char* value);
	void clear();
	int* ctr;
public:
	mv_strref();
	mv_strref(const mv_strref& ref);
	~mv_strref()
	{
		clear();
	}
	char* ptr;
	mv_strref& operator= (char* value);
	mv_strref& operator= (const mv_strref& ref);
	void set(const char* source, int first, int last);
};

class mv_strarr : public mvDynamicArray<mv_strref>
{
public:
	mv_strarr(int size) : mvDynamicArray<mv_strref>(size)
	{
	}
	void append(char* value);
};

#endif
