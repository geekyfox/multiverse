
#ifndef __MULTIVERSE_CLASS_HEADER__
#define __MULTIVERSE_CLASS_HEADER__

#include "mvAttr.h"
#include "mvArray.h"
#include "mvAttrSpec.h"

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

mvStrBuffer& operator<<(mvStrBuffer&, const mvClass&);

typedef mvObjectCache<mvClass> mvClassCache;

mv_error* mv_validate_assign(mv_entity* entity, mv_class* cls);

#endif

