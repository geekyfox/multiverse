
#include "mvMemPool.h"

mvIntPool localIntPool;
mvStrPool localStrPool;

int* mvIntPool::acquire(int value) {
	int* ptr = this->get();
	*ptr = value;
	return ptr;
}

