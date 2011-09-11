
#ifndef __MULTIVERSE_STRBUFFER_HEADER__
#define __MULTIVERSE_STRBUFFER_HEADER__

class mvStrBuffer {
private:
	char* data;
	int size;
	int used;
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
	void indent(int shift)
	{
		pad += shift;
	}
	void unindent(int shift)
	{
		pad -= shift;
	}
};

typedef mvStrBuffer mv_strbuf;

#endif

