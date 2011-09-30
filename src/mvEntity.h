
#ifndef __MULTIVERSE_ENTITY_CACHE__
#define __MULTIVERSE_ENTITY_CACHE__

#include "error.h"
#include "mvAttr.h"
#include "mvArray.h"

class mvEntity {
public:
	mvEntity(int attrCount, int classCount);
	~mvEntity();
	mv_attrlist data;
	mv_strarr classes;
	void show(mvStrBuffer& buff) const;
	void update(const mv_attrlist& attrs) throw (mv_error*);
};

mvStrBuffer& operator << (mvStrBuffer& buf, const mvEntity& enty);

typedef mvEntity mv_entity;

typedef mvObjectCache<mv_entity> mvEntityCache;

#endif

