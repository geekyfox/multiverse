
#ifndef __MULTIVERSE_ARRAY_HEADER__
#define __MULTIVERSE_ARRAY_HEADER__

#include <stdlib.h>

template <class T>
class mvStaticArray {
private:
	mvStaticArray<T>* operator&();
	int _size;
	T* items;
	mvStaticArray<T>& operator=(const mvStaticArray<T>&);
public:
	mvStaticArray() :
		_size(0), items(NULL)
	{
	}
	mvStaticArray(int size) :
		_size(size),
		items(size == 0 ? NULL : new T[size])
	{
	}
	~mvStaticArray()
	{
		clear();
	}
	void push(T& value)
	{
		T* update = new T[_size + 1];
		for (int i=0; i<_size; i++) update[i] = items[i];
		update[_size] = value;
		_size++;
		delete[] items;
		items = update;
	}
	void set(T* values, int count)
	{
		alloc(count);
		for (int i=0; i<count; i++) items[i] = values[i];
	}
	int size() const
	{
		return _size;
	}
	T& operator[](const int index) const
	{
		return items[index];
	}
	void clear()
	{
		if (items != NULL)
		{
			delete[] items;
			items = NULL;
			_size = 0;
		}
	}
	void alloc(int size)
	{
		items = (size == 0) ? NULL : new T[size];
		_size = size;
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
		delete[] _data;
	}
	int size() const
	{
		return _used;
	}
	T& operator[](int index) const
	{
		return _data[index];
	}
	int push(const T& value)
	{
		if (_size == _used) resize(_used * 2 + 2);
		int index = _used;
		_data[index] = value;
		_used++;
		return index;
	}
	void pack()
	{
	}
	void clear()
	{
		_used = 0;
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

