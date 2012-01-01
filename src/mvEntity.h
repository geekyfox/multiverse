
#ifndef __MULTIVERSE_ENTITY_CACHE__
#define __MULTIVERSE_ENTITY_CACHE__

#include "error.h"
#include "mvAttr.h"
#include "mvArray.h"

class mvEntity {
public:
	mvEntity(int attrCount, int classCount);
	~mvEntity();
	mvAttrlist data;
	mvStrArray classes;
	void show(mvStrBuffer& buff) const;
	void update(const mvAttrlist& attrs) throw (mvError*);
};

mvStrBuffer& operator << (mvStrBuffer& buf, const mvEntity& enty);

typedef mvObjectCache<mvEntity> mvEntityCache;

#endif

