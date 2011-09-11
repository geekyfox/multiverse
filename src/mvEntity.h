
#ifndef __MULTIVERSE_ENTITY_CACHE__
#define __MULTIVERSE_ENTITY_CACHE__

#include "model.h"
#include "mvArray.h"

class mvEntity {
public:
	mvEntity(int attrCount, int classCount);
	~mvEntity();
	mv_attrlist data;
	mv_strarr classes;
};

typedef mvEntity mv_entity;

void mv_entity_show(mv_strbuf* buf, mv_entity* entity);
mv_error* mv_entity_update(mv_entity* entity, mv_attrlist attrs);

typedef mvObjectCache<mv_entity> mvEntityCache;

#endif

