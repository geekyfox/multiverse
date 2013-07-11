
#include <string.h>

#include "multiverse.h"
#include "mvEntity.h"

mvEntity::mvEntity(int attrCount, int classCount) :
	classes(classCount),
	data(attrCount)
{
}

mvEntity::~mvEntity()
{
}

void mvEntity::show(mvStrBuffer& buff) const
{
	buff << "entity " << data << "\n";
}

mvStrBuffer& operator << (mvStrBuffer& buf, const mvEntity& enty)
{
	enty.show(buf);
	return buf;
}

void mvEntity::update(const mvAttrlist& attrs)
throw (mvError*)
{
	int sz = attrs.size(), i, j;
	int tmp[sz];
	int fill = data.size();
	for (i=0; i<sz; i++) {
		tmp[i] = -1;
		for (j=0; j<fill; j++) {
			if (data[j].name == attrs[i].name) {
				tmp[i] = j;
				break;
			}
		}
	}
	for (i=0; i<sz; i++) {
		mvAttr& src = attrs[i];
		switch (tmp[i]) {
		case -1:
			{
				mvAttr copy = src;
				data.push(copy);
			}
			break;
		default:
			mvAttr_update(&data[tmp[i]], &src);
		}
	}
}
