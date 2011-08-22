
#ifndef __MULTIVERSE_INTSET_HEADER__
#define __MULTIVERSE_INTSET_HEADER__

class mvIntset {
private:
	int used;
	int size;
	int* items;
public:
	mvIntset();
	mvIntset(int size);
	~mvIntset();
	void clear();
	bool contains(int value);
	int get(int index) {
		return items[index];
	}
	void put(int value);
	void remove(int value);
	int cardinality() {
		return used;
	}
};

#endif

