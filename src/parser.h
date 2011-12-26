
#ifndef __MULTIVERSE_OLDPARSER_HEADER__
#define __MULTIVERSE_OLDPARSER_HEADER__

#include <assert.h>

#include "mvAst.h"
#include "error.h"
#include "mvAttr.h"
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

/* Builds a subquery from corresponding AST. */
void mv_attrquery_parse(mv_attrspec* ptr, const mvStrref& key, const mv_ast& value);

/* Builds attribute's specification. */
void mv_spec_parse(mv_attrspec* ptr, const mvStrref& key, const mvStrref& value, mvAstType rel);

/* Parses a string into a command. */
void mv_command_parse(mvCommand& target, const mvStrref& cmdname,
const mvAst& ast) throw (mv_error*);

class mvTokenizer : public mv_strarr
{
public:
	mvTokenizer(const char* request) throw (mv_error*);
};	

#endif

