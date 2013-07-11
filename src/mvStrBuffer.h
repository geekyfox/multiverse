
#ifndef __MULTIVERSE_STRBUFFER_HEADER__
#define __MULTIVERSE_STRBUFFER_HEADER__

#include "mvStrref.h"

class mvStrBuffer {
private:
	char* data;
	int used;
	int size;
	int pad;
public:
	mvStrBuffer(int size);
	~mvStrBuffer();
	char* release();
	void append(const char* text);
	void append(const int num);
	mvStrBuffer& operator <<(const char* text)
	{
		append(text);
		return *this;
	}
	mvStrBuffer& operator <<(const mvStrref& text)
	{
		append(text.ptr);
		return *this;
	}
	mvStrBuffer& operator <<(int value)
	{
		append(value);
		return *this;
	}
	void indent(int shift)
	{
		pad += shift;
	}
	void unindent(int shift)
	{
		pad -= shift;
	}
};

#endif

