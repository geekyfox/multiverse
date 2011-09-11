
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

template <class T>
class mvDynamicArray {
private:
	int _size;
	int _used;
	T* _data;
	void resize(int newsize)
	{
		if (_size == newsize) return;
		T* newdata = new T[newsize];
		for (int i=0; i<_used; i++) newdata[i] = _data[i];
		delete[] _data;
		_size = newsize;
		_data = newdata;
	}
public:
	mvDynamicArray(int size) :
		_size(size),
		_used(0),
		_data(new T[size])
	{
	}
	~mvDynamicArray()
	{
		resize(_used);
		delete[] _data;
	}
	const int size()
	{
		return _used;
	}
	T& operator[](int index)
	{
		return _data[index];
	}
	int push(T& value)
	{
		if (_size == _used) resize(_used * 2);
		int index = _used;
		_data[index] = value;
		_used++;
		return index;
	}
};

template <class T>
class mvObjectCache {
private:
	mvDynamicArray<T*> _data;
public:
	mvObjectCache(int size) :
		_data(size)
	{
	}
	~mvObjectCache()
	{
		for (int i=0; i<_data.size(); i++)
		{
			if (_data[i] != NULL) delete _data[i];
		}
	}
	int size()
	{
		return _data.size();
	}
	T& operator[](int index)
	{
		return *_data[index];
	}
	bool exists(int index)
	{
		return _data[index] != NULL;
	}
	void drop(int index)
	{
		delete _data[index];
		_data[index] = NULL;
	}
	int push(T* value)
	{
		for (int i=0; i<_data.size(); i++)
		{
			if (_data[i] == NULL)
			{
				_data[i] = value;
				return i;
			}
		}
		return _data.push(value);
	}
};

#endif

