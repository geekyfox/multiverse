
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
	mvSpecList data;
};

mvStrBuffer& operator<<(mvStrBuffer&, const mvClass&);

typedef mvObjectCache<mvClass> mvClassCache;

mvError* mv_validate_assign(mvEntity* entity, mvClass* cls);

#endif

