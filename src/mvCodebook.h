
#ifndef __MULTIVERSE_CODEBOOK_HEADER__
#define __MULTIVERSE_CODEBOOK_HEADER__

#include "mvStrref.h"

class mvCodebook {
private:
	char** keys;
	int* values;
	int* hashes;
	int used;
	int size;
	void expand();
	void init(int);
	void push(char*, int);
public:
	mvCodebook(int);
	~mvCodebook();
	void insert(const char* key, int value);
	void insert(const mvStrref& key, int value)
	{
		insert(key.ptr, value);
	}
	int lookup(const char* key);
	int lookup(const mvStrref& key)
	{
		return lookup(key.ptr);
	}
	void remove(const char* key);
	int cardinality() {
		return used;
	}
};

#endif

