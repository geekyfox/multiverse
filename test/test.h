
#ifndef __MULTIVERSE_TEST_HEADER__
#define __MULTIVERSE_TEST_HEADER__

#include <string.h>
#include "../src/multiverse.h"

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

void perform_data_test();
void perform_parser_test();

#define REQ1 "create entity { name = 'Umberto Eco' } umberto_eco"
#define REQ2 "create entity { country = italy, name = 'Umberto Eco' \
} umberto_eco"
#define REQ6 "create class person { name : string }"
#define REQ7 "create class person { name:string }"

#define BADREQ1 "create entity { name = 'Umberto "

#endif

