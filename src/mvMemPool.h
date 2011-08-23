
#ifndef __MULTIVERSE_MEMPOOL_HEADER__
#define __MULTIVERSE_MEMPOOL_HEADER__

#include <stdlib.h>

template <class T, int SIZE, int ELEM_SIZE>
class mvMemPool {
private:
	T* pool[SIZE];
	int used;
public:
	mvMemPool() : used(0) {}
	~mvMemPool() {
		for (int i=0; i<used; i++) free(pool[i]);
	}
	T* get() {
		if (used == 0) {
			return (T*)malloc(sizeof(T) * ELEM_SIZE);
		} else {
			used--;
			return pool[used];
		}
	}
	void release(T* item) {
		if (used == SIZE) {
			free(item);
		} else {
			pool[used] = item;
			used++;
		}
	}
};

class mvIntPool : public mvMemPool<int, 64, 1> {
public:
	int* acquire(int value);
};

typedef mvMemPool<char, 64, 16> mvStrPool;

extern mvIntPool localIntPool;
extern mvStrPool localStrPool;

#endif

