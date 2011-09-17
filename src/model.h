
#ifndef __MULTIVERSE_MODEL_HEADER__
#define __MULTIVERSE_MODEL_HEADER__

#include "mvArray.h"
#include "mvStrBuffer.h"

typedef struct {
	char* ptr;
	int* ctr;
	int alc;
} mv_strref;

typedef struct {
	mv_strref* items;
	int size;
	int used;
} mv_strarr;

enum mvTypeCode {
	STRING, RAWREF, REF, INTEGER
};

// Multiverse is an object-oriented database and this
// is why there's such a basic building element as a
// key-value pair.
typedef struct {
	// Key of the attribute.
	char* name;
	// Type code, should be one of MVTYPE_ codes.
	mvTypeCode type;
	// Value of the attribute.
	// Valid field of the union is set by type code
	//   MVTYPE_STRING  -> string
	//   MVTYPE_RAWREF  -> rawref
	//   MVTYPE_REF     -> ref
	//   MVTYPE_INTEGER -> integer
	union {
		// A string.
		char* string;
		// A "raw" reference to an object that is
		// not resolved by database engine yet.
		char* rawref;
		// A real reference to an object.
		int ref;
		// An integer number.
		int integer;
	} value;
} mv_attr;

// Key-value pairs should also be grouped.
// Grouping is done using simple lists wrapped
// around pointers.
typedef struct {
	mv_attr* attrs;
	int size;
} mv_attrlist;

typedef struct {
	char* classname;
	mv_attrlist attrs;
} mv_query;

typedef struct {
	mvTypeCode type;
	char* classname;
} mv_typespec;

enum mvAttrSpecType {
	UNSET, TYPE, SUBQUERY
};

class mv_attrspec {
public:
	mv_attrspec() : type(UNSET)
	{
	}
	char* name;
	mvAttrSpecType type;
	union {
		mv_typespec typespec;
		mv_query subquery;
	} value;	
	void clear();
};

typedef mvStaticArray<mv_attrspec> mv_speclist;

#endif
 
