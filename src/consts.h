
#ifndef __MULTIVERSE_CONSTS_HEADER__
#define __MULTIVERSE_CONSTS_HEADER__

//
// Constants for syntax trees elements
//

#define MVAST_TEMPCOLON        -2008
#define MVAST_TEMPEQUALS       -2007
#define MVAST_TEMPCOMMA        -2006
#define MVAST_TEMPAPOSTROPHE   -2005
#define MVAST_TEMPATTRSPECLIST -2004
#define MVAST_TEMPCLOSEBRACE   -2003
#define MVAST_TEMPOPENBRACE    -2002
#define MVAST_TEMPATTRLIST     -2001
#define MVAST_LEAF              2001
#define MVAST_ATTRLIST          2002
#define MVAST_ATTRPAIR          2003
#define MVAST_TYPESPEC          2004
#define MVAST_ATTRSPECLIST      2005

//
// Constants for command codes
//

#define MVCMD_DO_NOTHING     -3002
#define MVCMD_QUIT           -3001
#define MVCMD_ASSIGN          3001
#define MVCMD_CREATE_CLASS    3002
#define MVCMD_CREATE_ENTITY   3003
#define MVCMD_DESTROY_ENTITY  3004
#define MVCMD_LOOKUP          3005
#define MVCMD_SHOW            3006

#endif

