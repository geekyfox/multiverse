
#ifndef __MULTIVERSE_HEADER__
#define __MULTIVERSE_HEADER__

#include <stdio.h>

#ifdef MEMWATCH
#include "memwatch/memwatch.h"
#endif

/*******************************/
/* Common utility functions    */
/*                             */
/* Implementation: mv_common.c */
/*******************************/

/******************/
/* Error handling */
/******************/

#define MVERROR_SYNTAX 1
#define MVERROR_INTERNAL 2
#define MVERROR_BADVAR 3

typedef struct {
	char* message;
	int code;
} mv_error;

void      mv_error_display(mv_error* error, FILE* file);
char*     mv_error_show(mv_error* error);
mv_error* mv_error_raise(int code, char* message);
mv_error* mv_error_raiseform(int code, char* format, char* message);
void      mv_error_release(mv_error* error);

/***************************/
/* Common string functions */
/***************************/

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

/*******************/
/* Name-value pair */
/*******************/

#define MVTYPE_STRING 1
#define MVTYPE_RAWREF 2
#define MVTYPE_REF    3

typedef struct {
	char* name;
	int type;
	union {
		char* string;
		char* rawref;
		int ref;
	} value;
} mv_attr;

void mv_attr_release(mv_attr* attr);
void mv_attr_show(mv_strbuf* buf, mv_attr* attr);

/****************************************/
/* Fixed-size array of name-value pairs */
/****************************************/

typedef struct {
	mv_attr* attrs;
	int size;
} mv_attrlist;

void mv_attrlist_alloc(mv_attrlist* ptr, int size);
void mv_attrlist_release(mv_attrlist* ptr);
void mv_attrlist_show(mv_strbuf* buf, mv_attrlist* ptr);

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

/**********************/
/* Executable command */
/**********************/

#define MVCMD_DO_NOTHING -2
#define MVCMD_QUIT -1
#define MVCMD_CREATE_ENTITY 1
#define MVCMD_SHOW 2

typedef struct {
	int code;
	mv_attrlist attrs;
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
} mv_entity;

void mv_entity_show(mv_strbuf* buf, mv_entity* entity);

/*********************/
/* Cache of entities */
/*********************/

typedef struct {
	int size;
	int used;
	mv_entity* items;
} mv_entcache;

void mv_entcache_alloc(mv_entcache* ptr, int size);
int  mv_entcache_put(mv_entcache* ptr, mv_attrlist* obj);
void mv_entcache_release(mv_entcache* ptr);


/*******************************/
/* Text parsing                */
/*                             */
/* Implementation: mv_parser.c */
/*******************************/

#define MVAST_TEMPCLOSEBRACE -3
#define MVAST_TEMPOPENBRACE -2
#define MVAST_TEMPATTRLIST -1
#define MVAST_LEAF 1
#define MVAST_ATTRLIST 2
#define MVAST_ATTRPAIR 3

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
int       mv_ast_to_attrlist(mv_attrlist* target, mv_ast* source);

void       mv_attr_parse(mv_attr* target, char* name, char* value);
mv_error*  mv_command_parse(mv_command* target, char* request);
mv_error*  mv_tokenize(mv_strarr* target, char* request);

typedef struct {
	mv_varbind vars;
	mv_entcache entities;
} mv_session;

void      mv_session_init(mv_session* state);
mv_error* mv_session_execute(mv_session* state, mv_command* action);
int       mv_session_findvar(mv_session* session, char* name);
void      mv_session_release(mv_session* state);
mv_error* mv_session_show(char** target, mv_session* source, char* name);


void      mv_local_end();
mv_error* mv_local_read(mv_command* target);
void      mv_local_execute(mv_command* cmd);
void      mv_local_start();

#endif
