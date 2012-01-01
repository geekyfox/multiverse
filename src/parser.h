
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
 * memory and returns an mvError.
 */

/* Builds a subquery from corresponding AST. */
void mvAttrquery_parse(mvAttrSpec* ptr, const mvStrref& key, const mvAst& value);

/* Builds attribute's specification. */
void mv_spec_parse(mvAttrSpec* ptr, const mvStrref& key, const mvStrref& value, mvAstType rel);

/* Parses a string into a command. */
void mvCommand_parse(mvCommand& target, const mvStrref& cmdname,
const mvAst& ast) throw (mvError*);

class mvTokenizer : public mvStrArray
{
public:
	mvTokenizer(const char* request) throw (mvError*);
};	

#endif

