
#ifndef __MULTIVERSE_HEADER__
#define __MULTIVERSE_HEADER__

#include <stdio.h>
#include <stdlib.h>

#include "mvArray.h"
#include "mvIntset.h"
#include "mvCodebook.h"
#include "error.h"
#include "model.h"
#include "mvCommand.h"

/*******************************/
/* Common utility functions    */
/*                             */
/* Implementation: mv_common.c */
/*******************************/

/***************************/
/* Common string functions */
/***************************/

#define STREQ(x, y) (strcmp((x), (y)) == 0)
#define MAX(x, y) ( ((x) < (y)) ? (y) : (x) )

int       mv_strhash(char* str);
mv_strref mv_strslice(const char* source, int start, int end);

/*****************************/
/* Common data structures    */
/*                           */
/* Implementation: mv_data.c */
/*****************************/


#include "mvEntity.h"

/*******************/
/* Name-value pair */
/*******************/

mv_attr mv_attr_copy(mv_attr* attr);
void    mv_attr_update(mv_attr* dst, mv_attr* src);
void    mv_attr_release(mv_attr* attr);
void    mv_attr_show(mv_strbuf* buf, mv_attr* attr);

/****************************************/
/* Fixed-size array of name-value pairs */
/****************************************/

void        mv_attrlist_alloc(mv_attrlist* ptr, int size);
mv_attrlist mv_attrlist_copy(mv_attrlist* src);
void        mv_attrlist_realloc(mv_attrlist* ptr, int newsize);
void        mv_attrlist_release(mv_attrlist* ptr);
void        mv_attrlist_show(mv_strbuf* buf, const mv_attrlist& ptr);


void mv_typespec_release(mv_typespec* spec);
void mv_typespec_show(mv_strbuf* buf, mv_typespec* spec);

void mv_query_release(mv_query* query);
void mv_query_show(mv_strbuf* buf, mv_query* query);

void mv_attrspec_show(mv_strbuf* buf, mv_attrspec* ptr);


void mv_speclist_show(mv_strbuf* buf, mv_speclist& ptr);

/*********************/
/* Cache of entities */
/*********************/


#include "mvClass.h"

mv_error* mv_validate_assign(mv_entity* entity, mv_class* cls);

#include "mvSession.h"

mv_error* mv_query_compile(mv_query* pat, mv_command* cmd);
int       mv_query_match(mv_query* pat, mv_entity* e);
void      mv_query_release(mv_query* pat);

void      mv_local_end();
mv_error* mv_local_read(mv_command* target);
void      mv_local_execute(mv_command* cmd);
void      mv_local_start();

#endif
