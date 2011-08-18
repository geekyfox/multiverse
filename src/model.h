
#ifndef __MULTIVERSE_MODEL_HEADER__
#define __MULTIVERSE_MODEL_HEADER__

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

// Multiverse is an object-oriented database and this
// is why there's such a basic building element as a
// key-value pair.
typedef struct {
	// Key of the attribute.
	char* name;
	// Type code, should be one of MVTYPE_ codes.
	int type;
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

#define MVTYPE_STRING  1001
#define MVTYPE_RAWREF  1002
#define MVTYPE_REF     1003
#define MVTYPE_INTEGER 1004

// Key-value pairs should also be grouped.
// Grouping is done using simple lists wrapped
// around pointers.
typedef struct {
	mv_attr* attrs;
	int size;
} mv_attrlist;

typedef struct {
	int exist;
	mv_attrlist data;
	mv_strarr classes;
} mv_entity;

typedef struct {
	char* classname;
	mv_attrlist attrs;
} mv_query;

typedef struct {
	int type;
	char* classname;
} mv_typespec;

typedef struct {
	char* name;
	int type;
	union {
		mv_typespec typespec;
		mv_query subquery;
	} value;	
} mv_attrspec;

typedef struct {
	mv_attrspec* specs;
	int size;
} mv_speclist;

//
// mv_attrspec value selector
//
#define MVSPEC_TYPE     4001
#define MVSPEC_SUBQUERY 4002

// Multiverse can execute commands. So, there's
// a special datatype for commands as well.
typedef struct {
	// Command's code, should be one of MVCMD_ codes.
	int code;
	// Attributes of the command.
	mv_attrlist attrs;
	// Attribute specifications of the command.
	mv_speclist spec;
	// Variables of the command.
	mv_strarr vars;
} mv_command;

#define MVCMD_DO_NOTHING     -3002
#define MVCMD_QUIT           -3001
#define MVCMD_ASSIGN          3001
#define MVCMD_CREATE_CLASS    3002
#define MVCMD_CREATE_ENTITY   3003
#define MVCMD_DESTROY_ENTITY  3004
#define MVCMD_LOOKUP          3005
#define MVCMD_SHOW            3006
#define MVCMD_UPDATE_ENTITY   3007


#endif
 
