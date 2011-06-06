
#ifndef __MULTIVERSE_TEST_HEADER__
#define __MULTIVERSE_TEST_HEADER__

#include <string.h>
#include <multiverse.h>

#define FAIL(__err) do { if (__err != NULL) {  \
mv_error_display(__err, stderr);               \
DIE("Unexpected error"); }} while (0)

void assert_string(char* pattern, char* value);

#define ASSERT_INT(__value, __pattern)              \
do { if ((__value) != (__pattern)) {                \
DIE("Expected '%d', got '%d'", __pattern, __value); \
}} while (0)

#define ASSERT_STRING(__value, __pattern)           \
do { if (strcmp(__value, __pattern) != 0) {         \
DIE("Expected '%s', got '%s'", __pattern, __value); \
}} while (0)

#define ASSERT_NOTNULL(__value) do {         \
if (__value == NULL) DIE("Unexpected NULL"); \
} while (0)

#define ASSERT_NULL(__value) do {         \
if (__value != NULL) DIE("Unexpected non-NULL value"); \
} while (0)

#define ENTER() printf("%s ENTERED\n", __FUNCTION__)

#define SUCCESS() printf("%s PASSED\n", __FUNCTION__)

#define TEST static void
#define TESTCASE void

void perform_data_test();
void perform_parser_test();
void perform_printer_test();

#define REQ1 "create entity { name = 'Umberto Eco' } umberto_eco"
#define REQ2 "create entity { country = italy, name = 'Umberto Eco' \
} umberto_eco"
#define REQ3 "show umberto_eco"
#define REQ5 "create entity { title = 'Name of the Rose', author = \
umberto_eco } name_of_the_rose"
#define REQ6 "create class person { name : string }"
#define REQ7 "create class person { name:string }"
#define REQ8 "create class fruit { name : string, color : color }"
#define REQ9 "create entity { name = 'Umberto Eco', \
country = italy, hometown = bologna }"

#define RESP2 "name_of_the_rose = entity {\n  title = 'Name of the \
Rose',\n  author = ##0\n}\n"
#define RESP3 "person = class {\n  name : string\n}\n"

#define BADREQ1 "create entity { name = 'Umberto "
#define ERRRESP1 "Unmatched \"'\" in \"create entity { name = 'Umberto \"" 
#define BADREQ2 "create entity { name = 'Umberto' "

#endif

