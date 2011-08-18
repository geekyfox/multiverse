
#ifndef __MULTIVERSE_HEADER__
#define __MULTIVERSE_HEADER__

#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "model.h"

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
mv_strref mv_strslice(char* source, int start, int end);

mv_strref mv_strref_alloc(int size);
mv_strref mv_strref_wrap(char* str);
void      mv_strref_free(mv_strref* ref);
mv_strref mv_strref_copy(mv_strref* ref);

/*****************************/
/* Common data structures    */
/*                           */
/* Implementation: mv_data.c */
/*****************************/

typedef struct {
	char* data;
	int size;
	int used;
	int pad;
} mv_strbuf;

char* mv_strbuf_align(mv_strbuf* ptr);
void  mv_strbuf_alloc(mv_strbuf* ptr, int size);
void  mv_strbuf_append(mv_strbuf* ptr, char* text);
void  mv_strbuf_appendi(mv_strbuf* ptr, int num);

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
void        mv_attrlist_show(mv_strbuf* buf, mv_attrlist* ptr);


void mv_typespec_release(mv_typespec* spec);
void mv_typespec_show(mv_strbuf* buf, mv_typespec* spec);

void mv_query_release(mv_query* query);
void mv_query_show(mv_strbuf* buf, mv_query* query);


void mv_attrspec_release(mv_attrspec* ptr);
void mv_attrspec_show(mv_strbuf* buf, mv_attrspec* ptr);


void mv_speclist_alloc(mv_speclist* ptr, int size);
void mv_speclist_release(mv_speclist* ptr);
void mv_speclist_show(mv_strbuf* buf, mv_speclist* ptr);

/******************************/
/* Expandable array of string */
/******************************/

void mv_strarr_alloc(mv_strarr* ptr, int size);
void mv_strarr_append(mv_strarr* ptr, char* value);
void mv_strarr_appslice(mv_strarr* ptr, char* source, int start, int end);
void mv_strarr_appref(mv_strarr* ptr, mv_strref* ref);
void mv_strarr_release(mv_strarr* ptr);

typedef struct {
	int* items;
	int size;
	int used;
} mv_intset;

void mv_intset_alloc(mv_intset* ptr, int size);
int  mv_intset_contains(mv_intset* ptr, int value);
void mv_intset_put(mv_intset* ptr, int value);
void mv_intset_release(mv_intset* ptr);
void mv_intset_remove(mv_intset* ptr, int value);

/**********************/
/* Executable command */
/**********************/

void mv_command_release(mv_command* action);

/**********************************/
/* String to integer code mapping */
/**********************************/

typedef struct {
	char** keys;
	int* values;
	int* hashes;
	int size;
	int used;
} mv_varbind;

void mv_varbind_alloc(mv_varbind* ptr, int size);
void mv_varbind_insert(mv_varbind* ptr, char* key, int value);
int  mv_varbind_lookup(mv_varbind* ptr, char* key);
void mv_varbind_release(mv_varbind* ptr);
void mv_varbind_remove(mv_varbind* ptr, char* key);
void mv_varbind_show(mv_varbind* ptr);

/*****************/
/* Single entity */
/*****************/


void mv_entity_alloc(mv_entity* entity, int attrs, int classes);
void mv_entity_show(mv_strbuf* buf, mv_entity* entity);
void mv_entity_release(mv_entity* entity);
mv_error* mv_entity_update(mv_entity* entity, mv_attrlist attrs);

/*********************/
/* Cache of entities */
/*********************/

typedef struct {
	int size;
	int used;
	mv_entity* items;
} mv_entcache;

void mv_entcache_alloc(mv_entcache* ptr, int size);
void mv_entcache_put(mv_entcache* ptr, int* ref, mv_entity* obj);
void mv_entcache_release(mv_entcache* ptr);

typedef struct {
	int exist;
	mv_speclist data;
} mv_class;

void mv_class_show(mv_strbuf* buf, mv_class* class);

typedef struct {
	int size;
	int used;
	mv_class* items;
} mv_clscache;

void mv_clscache_alloc(mv_clscache* ptr, int size);
void mv_clscache_put(mv_clscache* ptr, int* ref, mv_speclist* obj);
void mv_clscache_release(mv_clscache* ptr);

mv_error* mv_validate_assign(mv_entity* entity, mv_class* class);

typedef struct {
	mv_varbind vars;
	mv_entcache entities;
	mv_varbind clsnames;
	mv_clscache classes;
	int autovalidate;
} mv_session;

void      mv_session_init(mv_session* state);
mv_error* mv_session_execute(mv_session* state, mv_command* action);
int       mv_session_findclass(mv_session* session, char* name);
int       mv_session_findvar(mv_session* session, char* name);
mv_error* mv_session_lookup(mv_intset*, mv_session*, mv_command*);
mv_error* mv_session_perform(mv_session* state, mv_strarr* script);
void      mv_session_release(mv_session* state);
mv_error* mv_session_show(char** target, mv_session* source, char* name);

mv_error* mv_query_compile(mv_query* pat, mv_command* cmd);
int       mv_query_match(mv_query* pat, mv_entity* e);
void      mv_query_release(mv_query* pat);

void      mv_local_end();
mv_error* mv_local_read(mv_command* target);
void      mv_local_execute(mv_command* cmd);
void      mv_local_start();

#endif
