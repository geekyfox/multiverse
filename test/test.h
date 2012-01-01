
#ifndef __MULTIVERSE_TEST_HEADER__
#define __MULTIVERSE_TEST_HEADER__

#include <string.h>
#include <multiverse.h>

#include <mvAst.h>

#define FAIL(__err) do { if (__err != NULL) {  \
mvError_display(__err, stderr);               \
DIE("Unexpected error"); }} while (0)

#define ASSERT_INT(__value, __pattern)              \
do { if ((__value) != (__pattern)) {                \
DIE("Expected '%d', got '%d'", __pattern, __value); \
}} while (0)

#define ASSERT_NOLESS(__value, __pattern) do {           \
if ((__value) < (__pattern)) {                           \
DIE("Expected at least %d, got %d", __pattern, __value); \
}} while (0)

#define ASSERT_STRING(__value, __pattern)           \
do { if (strcmp(__value, __pattern) != 0) {         \
DIE("Expected '%s', got '%s'", __pattern, __value); \
}} while (0)

#define ASSERT_STRREF(__ref, __pattern) ASSERT_STRING(__ref.ptr, __pattern)

#define ASSERT_NOTNULL(__value) do {         \
if (__value == NULL) DIE("Unexpected NULL"); \
} while (0)

#define ASSERT_NULL(__value) do {         \
if (__value != NULL) DIE("Unexpected non-NULL value"); \
} while (0)

#define ASSERT_ERROR(__err, __code) do { if (__err == NULL) { \
DIE("Expected errorcode = %d, got null\n", __code); } else    \
if (__err->code != __code) { mvError_display(__err, stderr); \
DIE("Expected errorcode = %d\n", __code); }}  while (0)

#define ENTER() printf("%s ENTERED\n", __FUNCTION__)

#define SUCCESS() printf("%s PASSED\n", __FUNCTION__)

#define FAILFAST(expr) do { mvError* __err = expr; \
FAIL(__err); } while (0)

#define TEST static void
#define TESTCASE void

#define REQ1 "create entity { name = 'Umberto Eco' } umberto_eco"
#define REQ2 "create entity { country = italy, name = 'Umberto Eco' \
} umberto_eco"
#define REQ3 "show umberto_eco"
#define REQ4 "create entity {\nname = 'Umberto Eco'\n} umberto_eco"
#define REQ5 "create entity { title = 'Name of the Rose', author = \
umberto_eco } name_of_the_rose"
#define REQ6 "create class person { name : string }"
#define REQ7 "create class person { name:string }"
#define REQ8 "create class fruit { name : string, color : color }"
#define REQ9 "create entity { name = 'Umberto Eco', \
country = italy, hometown = bologna }"
#define REQ10 "assign person to umberto_eco"
#define REQ11 "lookup person with { name = 'Umberto Eco' }"
#define REQ12 "destroy entity umberto_eco"
#define REQ13 "lookup person"
#define REQ14 "create entity { height = 324 } eiffel_tower"
#define REQ15 "create class tower { height : integer }"
#define REQ16 "assign tower to eiffel_tower"
#define REQ17 "lookup tower with { height = 324 }"
#define REQ18 "create class writer { \
books = [book with { author = $$ }] }"
#define REQ19 "create class book { title : string }"
#define REQ20 "update entity eiffel_tower with { location = london }"
#define REQ21 "update entity eiffel_tower with { location = paris }"
#define REQ22 "update entity eiffel_tower remove location"
#define REQ23 "create entity {} dummy"
#define REQ24 "assign person to dummy"
#define REQ25 "lookup entity with { name = 'Umberto Eco' }"

#define RESP2 "name_of_the_rose = entity {\n  title = 'Name of the \
Rose',\n  author = ##0\n}\n"
#define RESP3 "person = class {\n  name : string\n}\n"
#define RESP4 "writer = class {\n  books = book with {\n    \
author = $$ (UNRESOLVED)\n  }\n}\n"

#define BADREQ1 "create entity { name = 'Umberto "
#define ERRRESP1 "Unmatched \"'\" in \"create entity { name = 'Umberto \"" 
#define BADREQ2 "create entity { name = 'Umberto' "
#define BADREQ3 "create entity { name = 'Umberto' , }"
#define BADREQ4 "update dummy with { name = 'Dummy' }"

#endif

