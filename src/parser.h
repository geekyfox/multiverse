
#ifndef __MULTIVERSE_PARSER_HEADER__
#define __MULTIVERSE_PARSER_HEADER__

#include <assert.h>

#include "mvAst.h"
#include "error.h"
#include "model.h"
#include "mvArray.h"
#include "mvCommand.h"

/* Parses a string into an AST.
 *
 * target - AST to populate
 * request - request to parse
 *
 * In case of parsing error, releases all allocated
 * memory and returns an mv_error.
 */

void mv_attr_parse(mv_attr* target, char* key, char* value);

/* Populates an mv_attrlist from AST. */
void mv_attrlist_parse(mv_attrlist* target, mv_ast& source);

/* Builds a subquery from corresponding AST. */
void mv_attrquery_parse(mv_attrspec* ptr, char* key, mv_ast& value);

/* Builds attribute's specification. */
void mv_spec_parse(mv_attrspec* ptr, char* key, char* value, int rel);

/* Parses a string into a command. */
void mv_command_parse(mvCommand& target, const char* request) throw (mv_error*);

/* Tokenizes a string. */
mv_error* mv_tokenize(mv_strarr* target, const char* request);

#endif

