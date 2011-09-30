
#include "mvClass.h"
#include "mvAttrSpec.h"

mvStrBuffer& operator<<(mvStrBuffer& buf, const mvClass& cls)
{
	buf << "class " << cls.data;
	return buf;
}

