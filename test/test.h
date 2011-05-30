
#ifndef __MULTIVERSE_TEST_HEADER__
#define __MULTIVERSE_TEST_HEADER__

#include "../src/multiverse.h"

#define FAIL(__err) do {          \
if (__err != NULL) {              \
mv_error_display(__err, stderr);  \
abort(); }} while (0)

void assert_string(char* pattern, char* value);

#define ASSERT_INT(__value, __pattern)              \
do { if ((__value) != (__pattern)) {                \
DIE("Expected '%d', got '%d'", __value, __pattern); \
}} while (0)

#define ASSERT_STRING(__value, __pattern)           \
do { if (strcmp(__value, __pattern) != 0) {         \
DIE("Expected '%d', got '%d'", __value, __pattern); \
}} while (0)

void perform_data_test();

#endif

