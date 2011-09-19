
#include "multiverse.h"
#include "mvEntity.h"

mvEntity::mvEntity(int attrCount, int classCount) :
	classes(classCount)
{
	mv_attrlist_alloc(&data, attrCount);
}

mvEntity::~mvEntity()
{
	mv_attrlist_release(&data);
}

void mvEntity::show(mvStrBuffer& buff) const
{
	buff.append("entity ");
	mv_attrlist_show(&buff, data);
	buff.append("\n");
}

mvStrBuffer& operator << (mvStrBuffer& buf, const mvEntity& enty)
{
	enty.show(buf);
	return buf;
}

