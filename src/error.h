
#ifndef __MULTIVERSE_ERROR_HEADER__
#define __MULTIVERSE_ERROR_HEADER__

#include <stdio.h>
#include <stdlib.h>

#define MVERROR_SYNTAX 1
#define MVERROR_INTERNAL 2
#define MVERROR_BADVAR 3
#define MVERROR_BADCMD 4

typedef struct {
	char* message;
	int code;
} mv_error;

void      mv_error_display(mv_error* error, FILE* file);
char*     mv_error_show(mv_error* error);
void      mv_error_release(mv_error* error);
mv_error* mv_error_unmatched(int objcode, char* command);

#define PREPARE_ERROR(__errvar, __errcd, ...) do { \
__errvar = malloc(sizeof(mv_error));               \
asprintf(&(__errvar->message), __VA_ARGS__);       \
__errvar->code = MVERROR_##__errcd; } while (0)    \

#define THROW(__errcd, ...) do {                 \
mv_error* __errtmp__;                            \
PREPARE_ERROR(__errtmp__, __errcd, __VA_ARGS__); \
return __errtmp__; } while (0)

#define DIE(...) do {                              \
fprintf(stderr, "[ !!! Fatal error | %s %s:%d ] ", \
__func__, __FILE__, __LINE__);                     \
fprintf(stderr, __VA_ARGS__);                      \
fprintf(stderr, "\n");                             \
abort(); } while (0)

#define EXPECT(cond, ...) do { if (!(cond)) \
DIE(__VA_ARGS__); } while (0)

#define FAILRET(expr) do { mv_error* __err = (expr); \
if (__err != NULL) return __err; } while (0)

#endif

