
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

void mvEntity::update(const mv_attrlist& attrs)
throw (mv_error*)
{
	int sz = attrs.size(), i, j;
	int tmp[sz];
	int fill = data.size();
	for (i=0; i<sz; i++) {
		tmp[i] = -1;
		for (j=0; j<fill; j++) {
			if (STREQ(data[j].name, attrs[i].name)) {
				tmp[i] = j;
				break;
			}
		}
	}
	for (i=0; i<sz; i++) {
		mv_attr& src = attrs[i];
		switch (tmp[i]) {
		case -1:
			{
				mv_attr copy = src;
				data.push(copy);
			}
			break;
		default:
			mv_attr_update(&data[tmp[i]], &src);
		}
	}
}
