
#ifndef __MULTIVERSE_ATTRSPEC_HEADER__
#define __MULTIVERSE_ATTRSPEC_HEADER__

#include "mvStrref.h"
#include "mvStrBuffer.h"

class mv_attrspec;
typedef mvStaticArray<mv_attrspec> mv_speclist;

#include "mvQuery.h"

class mv_typespec {
private:
	mvStrref classname;
public:
	mv_typespec(mvTypeCode type);
	mv_typespec(const mvStrref& classname);
	~mv_typespec();
	const mvTypeCode type;
	const mvStrref& name() const
	{
		return classname;
	}
};

mvStrBuffer& operator<<(mvStrBuffer&, const mv_typespec&);

enum mvAttrSpecType
{
	UNSET, TYPE, SUBQUERY
};

class mv_attrspec
{
private:
	mv_attrspec(mv_attrspec&);
	void operator=(mv_attrspec&);
	union {
		mv_typespec* typespec;
		mvQuery* subquery;
	} value;	
	mvAttrSpecType type;
public:
	mv_attrspec() : type(UNSET)
	{
	}
	~mv_attrspec();
	mvStrref name;
	mvAttrSpecType get_type() const { return type; }
	const mvQuery& subquery() const
	{
		if (type != SUBQUERY) abort();
		return (*value.subquery);
	}
	mvQuery& subquery_mutable()
	{
		if (type != SUBQUERY)
		{
			if (type == TYPE) delete value.typespec;
			type = SUBQUERY;
			value.subquery = new mvQuery;
		}
		return (*value.subquery);
	}
	void set_typespec(mvTypeCode code)
	{
		if (type == TYPE) delete value.typespec;
		if (type == SUBQUERY) delete value.subquery;
		type = TYPE;
		value.typespec = new mv_typespec(code);
	}
	void set_typespec(const mvStrref& classname)
	{
		if (type == TYPE) delete value.typespec;
		if (type == SUBQUERY) delete value.subquery;
		type = TYPE;
		value.typespec = new mv_typespec(classname);
	}
	const mv_typespec& typespec() const
	{
		return (*value.typespec);
	}
};

mvStrBuffer& operator << (mvStrBuffer& buf, const mv_attrspec& enty);
mvStrBuffer& operator << (mvStrBuffer& buf, const mv_speclist& enty);


#endif

