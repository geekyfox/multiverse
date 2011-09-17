
#ifndef __MULTIVERSE_CLASS_HEADER__
#define __MULTIVERSE_CLASS_HEADER__

#include "model.h"
#include "multiverse.h"
#include "mvArray.h"

class mvClass {
public:
	mvClass(int size) :
		data(size)
	{
	}
	~mvClass()
	{
	}
	mv_speclist data;
};

typedef mvClass mv_class;

void mv_class_show(mv_strbuf* buf, mv_class* cls);

typedef mvObjectCache<mv_class> mvClassCache;

#endif

