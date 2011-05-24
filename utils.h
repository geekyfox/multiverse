
#ifndef __MULTIVERSE_UTILS_HEADER__
#define __MULTIVERSE_UTILS_HEADER__

#define MALLOC_ONE(type) \
(type*)malloc(sizeof(type))

#define MALLOC_MANY(type, size) \
(type*)malloc(sizeof(type) * (size))

#define CREATE_ONE(var, type) \
type* var = MALLOC_ONE(type)

#define CREATE_MANY(var, type, size) \
type* var = MALLOC_MANY(type, size)

#define REALLOC(var, size) \
var = (typeof(var))realloc(var, sizeof(var[0]) * (size))

#define LIST_EXPAND(var, extra) \
REALLOC(var.data, var.size + extra); var.size += extra

#define PTRLIST_EXPAND(var, extra) \
var->data = REALLOC(var->data, var->size + extra); var->size += extra

#define LIST_ALLOC(var, sz) \
var.data = MALLOC_MANY(typeof(var.data[0]), sz); var.size = (sz)

#define LIST_LOCAL(listtype, var, sz) \
listtype var; LIST_ALLOC(var, sz)

#define LIST_LOCAL1(listtype, var, a) \
LIST_LOCAL(listtype, var, 1); var.data[0] = a

#define LIST_LOCAL2(listtype, var, a, b) \
LIST_LOCAL(listtype, var, 2); var.data[0] = a; var.data[1] = b

#define PTRLIST_ALLOC(var, itemtype, sz) \
var->data = MALLOC_MANY(itemtype, sz); var->size = (sz)

#define LIST_INIT(var) \
var.data = NULL; var.size = 0

#endif