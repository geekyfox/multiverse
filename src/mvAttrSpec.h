
#ifndef __MULTIVERSE_ATTRSPEC_HEADER__
#define __MULTIVERSE_ATTRSPEC_HEADER__

#include "mvStrref.h"
#include "mvStrBuffer.h"

class mvAttrSpec;
typedef mvStaticArray<mvAttrSpec> mvSpecList;

#include "mvQuery.h"

class mvTypeSpec {
private:
	mvStrref classname;
public:
	mvTypeSpec(mvTypeCode type);
	mvTypeSpec(const mvStrref& classname);
	~mvTypeSpec();
	const mvTypeCode type;
	const mvStrref& name() const
	{
		return classname;
	}
};

mvStrBuffer& operator<<(mvStrBuffer&, const mvTypeSpec&);

enum mvAttrSpecType
{
	UNSET, TYPE, SUBQUERY
};

class mvAttrSpec
{
private:
	mvAttrSpec(mvAttrSpec&);
	union {
		mvTypeSpec* typespec;
		mvQuery* subquery;
	} value;	
	mvAttrSpecType type;
public:
	mvAttrSpec() : type(UNSET)
	{
	}
	~mvAttrSpec();
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
		value.typespec = new mvTypeSpec(code);
	}
	void set_typespec(const mvStrref& classname)
	{
		if (type == TYPE) delete value.typespec;
		if (type == SUBQUERY) delete value.subquery;
		type = TYPE;
		value.typespec = new mvTypeSpec(classname);
	}
	const mvTypeSpec& typespec() const
	{
		return (*value.typespec);
	}
	void operator=(mvAttrSpec&)
	throw (mvError*);
};

mvStrBuffer& operator << (mvStrBuffer& buf, const mvAttrSpec& enty);
mvStrBuffer& operator << (mvStrBuffer& buf, const mvSpecList& enty);


#endif

