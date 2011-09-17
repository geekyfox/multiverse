
#ifndef __MULTIVERSE_AST_HEADER__
#define __MULTIVERSE_AST_HEADER__

#include "error.h"
#include "model.h"
#include "mvArray.h"
#include "mvCommand.h"

/*
 *  This structure represents an abstract syntax
 *  tree as a bounded pointer.
 */
class mv_ast : public mvStaticArray<struct mv_ast_entry> {
public:
	mv_ast();
	mv_ast(struct mv_ast_entry& item);
	mv_ast(struct mv_ast_entry& first, struct mv_ast_entry& second);
};

/*
 *  This structure represents an abstract syntax
 *  tree's entry.
 */
class mv_ast_entry {
public:
	/* Entry's type (MVAST_) code. */
	int type;
	/* Entry's value.
	 *
	 * When type=MVAST_LEAF, leaf contains the value.
	 *
	 * When type is composite (such as MVAST_ATTRLIST), subtree
	 * contains the value.
	 *
	 * When type is atomic (such as MVAST_TEMPCOMMA, see
	 * mv_ast_release() for a full list), there's no value.
	 */
	union {
		mv_strref* leaf;
		mv_ast* subtree;
	} value;
	void clear();
};

/*
 * Constants for syntax tree entry's type
 */
#define MVAST_TEMPCLOSEBRACKET -2010
#define MVAST_TEMPOPENBRACKET  -2009
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
#define MVAST_SUBQUERY          2006
#define MVAST_ATTRQUERY         2007

/* Parses a string into an AST.
 *
 * target - AST to populate
 * request - request to parse
 *
 * In case of parsing error, releases all allocated
 * memory and returns an mv_error.
 */
mv_error* mv_ast_parse(mv_ast& target, const char* request);

/* Builds attribute's value from it's name and value. */
void mv_attr_parse(mv_attr* target, char* name, char* value);

/* Populates an mv_attrlist from AST. */
void mv_attrlist_parse(mv_attrlist* target, mv_ast* source);

/* Populates an mv_speclist from it's representation. */
void mv_speclist_parse(mv_speclist& target, mv_ast* source);

/* Builds a subquery from corresponding AST. */
void mv_attrquery_parse(mv_attrspec* ptr, char* key, mv_ast& value);

/* Builds attribute's specification. */
void mv_spec_parse(mv_attrspec* ptr, char* key, char* value, int rel);

/* Parses a string into a command. */
void mv_command_parse(mvCommand& target, const char* request) throw (mv_error*);

/* Tokenizes a string. */
mv_error* mv_tokenize(mv_strarr* target, const char* request);

#endif

