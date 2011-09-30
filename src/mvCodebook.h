
#ifndef __MULTIVERSE_CODEBOOK_HEADER__
#define __MULTIVERSE_CODEBOOK_HEADER__

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
	int lookup(const char* key);
	void remove(const char* key);
	int cardinality() {
		return used;
	}
};

#endif

