
#ifndef __MULTIVERSE_ATTR_HEADER__
#define __MULTIVERSE_ATTR_HEADER__

#include "mvArray.h"
#include "mvStrBuffer.h"
#include "mvStrref.h"

enum mvTypeCode {
	STRING, RAWREF, REF, INTEGER
};

// Multiverse is an object-oriented database and this
// is why there's such a basic building element as a
// key-value pair.
class mvAttr {
private:
public:
	mvAttr();
	mvAttr(const mvAttr&);
	~mvAttr();
	void operator=(const mvAttr&);
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
};

mvStrBuffer& operator<<(mvStrBuffer& buff, const mvAttr& attr);

class mvAttrlist : public mvStaticArray<mvAttr> {
private:
	mvAttrlist(const mvAttrlist&);
public:
	mvAttrlist() : mvStaticArray<mvAttr>() {}
	mvAttrlist(int sz) : mvStaticArray<mvAttr>(sz) {}
	void copy_from(const mvAttrlist& src);
};

mvStrBuffer& operator<<(mvStrBuffer& buff, const mvAttrlist& attr);

#endif
 
