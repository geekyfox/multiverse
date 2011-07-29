
#ifndef __MULTIVERSE_HEADER__
#define __MULTIVERSE_HEADER__

#include <stdio.h>
#include <stdlib.h>

#include "consts.h"
#include "error.h"

/*******************************/
/* Common utility functions    */
/*                             */
/* Implementation: mv_common.c */
/*******************************/

/***************************/
/* Common string functions */
/***************************/

#define STREQ(x, y) (strcmp((x), (y)) == 0)

int   mv_strhash(char* str);
char* mv_strslice(char* source, int start, int end);

/*****************************/
/* Common data structures    */
/*                           */
/* Implementation: mv_data.c */
/*****************************/

typedef struct {
	char* data;
	int size;
	int used;
} mv_strbuf;

char* mv_strbuf_align(mv_strbuf* ptr);
void  mv_strbuf_alloc(mv_strbuf* ptr, int size);
void  mv_strbuf_append(mv_strbuf* ptr, char* text);
void  mv_strbuf_appendi(mv_strbuf* ptr, int num);

/*******************/
/* Name-value pair */
/*******************/

#define MVTYPE_STRING  1001
#define MVTYPE_RAWREF  1002
#define MVTYPE_REF     1003
#define MVTYPE_INTEGER 1004

typedef struct {
	char* name;
	int type;
	union {
		char* string;
		char* rawref;
		int ref;
		int integer;
	} value;
} mv_attr;

mv_attr mv_attr_copy(mv_attr* attr);
void    mv_attr_release(mv_attr* attr);
void    mv_attr_show(mv_strbuf* buf, mv_attr* attr);

/****************************************/
/* Fixed-size array of name-value pairs */
/****************************************/

typedef struct {
	mv_attr* attrs;
	int size;
} mv_attrlist;

void        mv_attrlist_alloc(mv_attrlist* ptr, int size);
mv_attrlist mv_attrlist_copy(mv_attrlist* src);
void        mv_attrlist_release(mv_attrlist* ptr);
void        mv_attrlist_show(mv_strbuf* buf, mv_attrlist* ptr);

typedef struct {
	int type;
	char* classname;
} mv_typespec;

void mv_typespec_release(mv_typespec* spec);
void mv_typespec_show(mv_strbuf* buf, mv_typespec* spec);

#define MVSPEC_TYPE  4001

typedef struct {
	char* name;
	int type;
	union {
		mv_typespec typespec;
	} value;	
} mv_attrspec;

void mv_attrspec_release(mv_attrspec* ptr);
void mv_attrspec_show(mv_strbuf* buf, mv_attrspec* ptr);

typedef struct {
	mv_attrspec* specs;
	int size;
} mv_speclist;

void mv_speclist_alloc(mv_speclist* ptr, int size);
void mv_speclist_release(mv_speclist* ptr);
void mv_speclist_show(mv_strbuf* buf, mv_speclist* ptr);

/******************************/
/* Expandable array of string */
/******************************/

typedef struct {
	char** items;
	int size;
	int used;
} mv_strarr;

void mv_strarr_alloc(mv_strarr* ptr, int size);
void mv_strarr_append(mv_strarr* ptr, char* value);
void mv_strarr_appslice(mv_strarr* ptr, char* source, int start, int end);
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

typedef struct {
	int code;
	mv_attrlist attrs;
	mv_speclist spec;
	mv_strarr vars;
} mv_command;

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

typedef struct {
	int exist;
	mv_attrlist data;
	mv_strarr classes;
} mv_entity;

void mv_entity_alloc(mv_entity* entity, int attrs, int classes);
void mv_entity_show(mv_strbuf* buf, mv_entity* entity);
void mv_entity_release(mv_entity* entity);

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

/*******************************/
/* Text parsing                */
/*                             */
/* Implementation: mv_parser.c */
/*******************************/

typedef struct {
	int size;
	struct mv_ast_entry* items;
} mv_ast;

typedef struct mv_ast_entry {
	int type;
	union {
		char* leaf;
		mv_ast subtree;
	} value;
} mv_ast_entry;

mv_error* mv_ast_parse(mv_ast* target, char* request);
void      mv_ast_release(mv_ast* ast);
void      mv_ast_to_attrlist(mv_attrlist* target, mv_ast* source);
void      mv_ast_to_speclist(mv_speclist* target, mv_ast* source);

void       mv_attr_parse(mv_attr* target, char* name, char* value);
void       mv_spec_parse(mv_attrspec* ptr, char* key, char* value, int rel);
mv_error*  mv_command_parse(mv_command* target, char* request);
mv_error*  mv_tokenize(mv_strarr* target, char* request);

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

typedef struct {
	char* clsname;
	mv_attrlist attrs;
} mv_pattern;

mv_error* mv_pattern_compile(mv_pattern* pat, mv_command* cmd);
int       mv_pattern_match(mv_pattern* pat, mv_entity* e);
void      mv_pattern_release(mv_pattern* pat);

void      mv_local_end();
mv_error* mv_local_read(mv_command* target);
void      mv_local_execute(mv_command* cmd);
void      mv_local_start();

#endif
