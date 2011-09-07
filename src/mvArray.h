
#ifndef __MULTIVERSE_ARRAY_HEADER__
#define __MULTIVERSE_ARRAY_HEADER__

#include <stdlib.h>

template <class T>
class mvStaticArray {
private:
	int _size;
public:
	T* items;
	mvStaticArray(int size) :
		items(size == 0 ? NULL : (T*)malloc(sizeof(T) * size)),
		_size(size)
	{
	}
	void push(const T& value)
	{
		_size++;
		items = (T*)realloc(items, sizeof(T)*_size);
		items[_size - 1] = value;
	}
	void set(T* values, int count)
	{
		_size = count;
		items = values;
	}
	int size()
	{
		return _size;
	}
};


#endif

