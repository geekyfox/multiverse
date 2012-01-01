
#ifndef __MULTIVERSE_ERROR_HEADER__
#define __MULTIVERSE_ERROR_HEADER__

#include <stdio.h>
#include <stdlib.h>

#define MVERROR_SYNTAX   1
#define MVERROR_INTERNAL 2
#define MVERROR_BADVAR   3
#define MVERROR_BADCMD   4
#define MVERROR_INVALID  5

class mvError {
public:
	char* message;
	int code;
};

void      mvError_display(mvError* error, FILE* file);
char*     mvError_show(mvError* error);
void      mvError_release(mvError* error);
mvError*  mvError_unmatched(int objcode, const char* command);

#define PREPARE_ERROR(__errvar, __errcd, ...) do { \
__errvar = (mvError*)malloc(sizeof(mvError));    \
asprintf(&(__errvar->message), __VA_ARGS__);       \
__errvar->code = MVERROR_##__errcd; } while (0)    \

#define THROW(__errcd, ...) do {                 \
mvError* __errtmp__;                            \
PREPARE_ERROR(__errtmp__, __errcd, __VA_ARGS__); \
return __errtmp__; } while (0)

#define NEWTHROW(__errcd, ...) do { \
mvError* __errtmp__;                            \
PREPARE_ERROR(__errtmp__, __errcd, __VA_ARGS__); \
throw __errtmp__; } while (0)

#define DIE(...) do {                              \
fprintf(stderr, "[ !!! Fatal error | %s %s:%d ] ", \
__func__, __FILE__, __LINE__);                     \
fprintf(stderr, __VA_ARGS__);                      \
fprintf(stderr, "\n");                             \
abort(); } while (0)

#define EXPECT(cond, ...) do { if (!(cond)) \
DIE(__VA_ARGS__); } while (0)

#define FAILRET(expr) do { mvError* __err = (expr); \
if (__err != NULL) return __err; } while (0)

#define FAILTHROW(expr) do { mvError* __err = (expr); \
if (__err != NULL) throw __err; } while (0)

#endif

