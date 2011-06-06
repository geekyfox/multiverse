
#include <assert.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#define PAIR(var) ((var->type) == MVAST_ATTRPAIR)
#define SPEC(var) ((var->type) == MVAST_TYPESPEC)
#define LEAF(var) ((var)->type == MVAST_LEAF)
#define LEAFFIX(var, text) (LEAF(var) && STREQ((var)->value.leaf, text))
#define TEMPFIX(var, code) ((var->type) == MVAST_TEMP##code)

mv_ast_entry
__mv_ast_mergepair(mv_ast_entry* a, mv_ast_entry* b, int typecode) {
	mv_ast_entry* tmp = malloc(sizeof(mv_ast_entry) * 2);
	tmp[0] = *a;
	tmp[1] = *b;

	mv_ast_entry result;
	result.type = typecode;
	result.value.subtree.size = 2;
	result.value.subtree.items = tmp;

	return result;
}

static void __merge(mv_ast_entry* a, mv_ast_entry* b, int *count) {
	if (TEMPFIX(a, ATTRLIST) && PAIR(b)) {	
		mv_ast* stree = &(a->value.subtree);
		int sz = ++(stree->size);
		stree->items = realloc(stree->items, sizeof(mv_ast_entry)*sz);
		stree->items[sz - 1] = *b;
		*count -= 2;
	}

	if (PAIR(a) && PAIR(b)) {
		mv_ast_entry* tmp = malloc(sizeof(mv_ast_entry) * 2);
		tmp[0] = *a;
		tmp[1] = *b;
		a->type = MVAST_TEMPATTRLIST;
		a->value.subtree.size = 2;
		a->value.subtree.items = tmp;
		*count -= 2;
		return;
	}

	if (SPEC(a) && SPEC(b)) {
		mv_ast_entry* tmp = malloc(sizeof(mv_ast_entry) * 2);
		tmp[0] = *a;
		tmp[1] = *b;
		a->type = MVAST_TEMPATTRSPECLIST;
		a->value.subtree.size = 2;
		a->value.subtree.items = tmp;
		*count -= 2;
		return;
	}
}

mv_error* mv_ast_parse(mv_ast* target, char* data) {
	mv_strarr tokens;
	mv_error* error;
	if ((error = mv_tokenize(&tokens, data))) {
		return error;
	}
	mv_ast_entry* stack = malloc(sizeof(mv_ast_entry) * tokens.used);
	int scan = 0, size = 0;

	while (scan < tokens.used) {
		if (STREQ(tokens.items[scan], "{")) {
			stack[size].type = MVAST_TEMPOPENBRACE;
		} else if (STREQ(tokens.items[scan], "}")) {
			stack[size].type = MVAST_TEMPCLOSEBRACE;
		} else {
			stack[size].type = MVAST_LEAF;
			stack[size].value.leaf = tokens.items[scan];
		}
		if (stack[size].type != MVAST_LEAF) {
			free(tokens.items[scan]);
		}
		size++;
		scan++;
		int oldsize;
		do {
			oldsize = size;
		
			if (size < 2) break;

			mv_ast_entry *e1 = stack + size - 2, *e2 = e1 + 1;

			if (TEMPFIX(e1, OPENBRACE) && TEMPFIX(e2, CLOSEBRACE)) {
				e1->type = MVAST_ATTRLIST;
				e1->value.subtree.size = 0;
				e1->value.subtree.items = NULL;
				size--;
				continue;
			}

			if (size < 3) break;

			e1--;
			e2--;
			mv_ast_entry *e3 = e1 + 2;
			
			if (LEAF(e1) && LEAF(e3)) {
				int typecode = 0;

				if (LEAFFIX(e2, "=")) {
					typecode = MVAST_ATTRPAIR;
				} else if (LEAFFIX(e2, ":")) {
					typecode = MVAST_TYPESPEC;
				}

				if (typecode != 0) {
					free(e2->value.leaf);
					*e1 = __mv_ast_mergepair(e1, e3, typecode);
					size -= 2;
					continue;	
				}
			}

			if (TEMPFIX(e1, OPENBRACE) && TEMPFIX(e3, CLOSEBRACE)) {
				if (PAIR(e2) || SPEC(e2)) {
					mv_ast_entry* tmp = malloc(sizeof(mv_ast_entry));
					tmp[0] = *e2;
					if (PAIR(e2)) {
						e1->type = MVAST_ATTRLIST;
					} else {
						e1->type = MVAST_ATTRSPECLIST;
					}
					e1->value.subtree.size = 1;
					e1->value.subtree.items = tmp;
					size -= 2;
					continue;				
				}

				if (TEMPFIX(e2, ATTRLIST) || TEMPFIX(e2, ATTRSPECLIST)) {
					if (TEMPFIX(e2, ATTRLIST)) {
						e1->type = MVAST_ATTRLIST;
					} else {
						e1->type = MVAST_ATTRSPECLIST;
					}
					e1->value.subtree = e2->value.subtree;
					size -= 2;
					continue;				
				}
			}
			
			if (LEAFFIX(e2, ",")) {
				int prevsz = size;
				__merge(e1, e3, &size);
				if (prevsz != size) {
					free(e2->value.leaf);
					continue;
				}
			}
		} while (oldsize != size);
	}
	
	stack = realloc(stack, sizeof(mv_ast_entry) * size);
	free(tokens.items);
	
	target->size = size;
	target->items = stack;

	int i;
	for (i=0; i < size; i++) {
		if (stack[i].type < 0) {
			mv_ast_release(target);
			return mv_error_unmatched(stack[i].type, data);
		}
	}
	return NULL;
}

void mv_ast_release(mv_ast* ast) {
	int i;
	for (i=0; i<ast->size; i++) {
		switch(ast->items[i].type) {
		case MVAST_LEAF:
			free(ast->items[i].value.leaf);
			break;
		case MVAST_TEMPOPENBRACE:
		case MVAST_TEMPCLOSEBRACE:
			break;
		default:
			mv_ast_release(&(ast->items[i].value.subtree));
		}
	}
	free(ast->items);
}

void mv_ast_to_attrlist(mv_attrlist* target, mv_ast* source) {
	mv_attrlist_alloc(target, source->size);
	int i;
	for (i=0; i<source->size; i++) {
		mv_ast_entry srcitem = source->items[i];
		if (srcitem.type != MVAST_ATTRPAIR) {
			DIE("AttrPair expected");
		}
		mv_ast astpair = srcitem.value.subtree;
		if (astpair.size != 2) {
			DIE("Two elements expected");
		}
		mv_ast_entry* items = astpair.items;
		if (items[0].type != MVAST_LEAF || items[1].type != MVAST_LEAF) {
			DIE("Leafs expected");
		}
		mv_attr_parse(&target->attrs[i],
                      items[0].value.leaf,
                      items[1].value.leaf);
	}
}

void mv_ast_to_speclist(mv_speclist* target, mv_ast* source) {
	mv_speclist_alloc(target, source->size);
	int i;
	for (i=0; i<source->size; i++) {
		mv_ast_entry src = source->items[i];
		assert(
			(src.type == MVAST_ATTRPAIR) ||
        	(src.type == MVAST_TYPESPEC)
		);
		assert(src.value.subtree.size == 2);
		mv_ast_entry* items = src.value.subtree.items;
		assert(LEAF(&items[0]));
		assert(LEAF(&items[1]));
		//
		mv_spec_parse(
			&(target->specs[i]),
			items[0].value.leaf,
			items[1].value.leaf,
			src.type
		);
	}
}

void mv_attr_parse(mv_attr* target, char* name, char* value) {
	target->name = strdup(name);
	if (value[0] == '\'') {
		target->type = MVTYPE_STRING;
		target->value.string = strdup(value + 1);
	} else {
		target->type = MVTYPE_RAWREF;
		target->value.rawref = strdup(value);
	}
}

mv_error* __mv_command_create_parse(mv_command* target, mv_ast_entry* items, int size) {
	if ((size == 1) || !(LEAF(&items[1]))) {
		THROW(INTERNAL, "Malformed 'create' command");
	}

	if (LEAFFIX(items + 1, "entity")) {
		target->code = MVCMD_CREATE_ENTITY;
		if (items[2].type != MVAST_ATTRLIST) {
			THROW(INTERNAL, "Expected AttrList, got %d", items[2].type);
		}
		assert(items[2].type == MVAST_ATTRLIST);
		assert(size >= 3);
		assert(size <= 4);
		mv_ast_to_attrlist(&target->attrs, &items[2].value.subtree);
		if (size == 4) {
			assert(items[3].type == MVAST_LEAF);
			mv_strarr_alloc(&target->vars, 1);
			mv_strarr_append(&target->vars, items[3].value.leaf);
		} else {
			mv_strarr_alloc(&target->vars, 0);
		}
		target->spec.size = 0;
		target->spec.specs = NULL;
		return NULL;
	}

	if (LEAFFIX(items + 1, "class")) {
		target->code = MVCMD_CREATE_CLASS;
		if (!LEAF(&items[2])) {
			THROW(INTERNAL, "Expected class name, got %d", items[2].type);
		}
		if (items[3].type != MVAST_ATTRSPECLIST) {
			THROW(INTERNAL, "Expected AttrSpecList, got %d", items[3].type);
		}
		mv_ast_to_speclist(&(target->spec), &(items[3].value.subtree));
		mv_strarr_alloc(&target->vars, 1);
		mv_strarr_append(&target->vars, items[2].value.leaf);
		target->attrs.size = 0;
		target->attrs.attrs = NULL;
		return NULL;
	}
	
	THROW(SYNTAX, "Invalid task for create - '%s'", items[1].value.leaf);
}

mv_error* mv_command_parse(mv_command* target, char* data) {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, data);
	if (error != NULL) return error;

	if (LEAFFIX(ast.items, "create")) {
		error = __mv_command_create_parse(target, ast.items, ast.size);
		mv_ast_release(&ast);
		return error;
	}

	if (LEAFFIX(ast.items, "show"))
	{
		assert(ast.size == 2);
		assert(ast.items[1].type == MVAST_LEAF);
		target->code = MVCMD_SHOW;
		target->attrs.size = 0;
		target->attrs.attrs = NULL;
		target->spec.size = 0;
		target->spec.specs = NULL;
		mv_strarr_alloc(&target->vars, 1);
		mv_strarr_append(&target->vars, ast.items[1].value.leaf);
		mv_ast_release(&ast);
		return NULL;
	}
	if (LEAFFIX(ast.items, "quit"))
	{
		assert(ast.size == 1);
		target->code = MVCMD_QUIT;
		target->attrs.size = 0;
		target->vars.size = 0;
		target->vars.used = 0;
		mv_ast_release(&ast);
		return NULL;
	}

	THROW(SYNTAX, "Syntax error");
}

void mv_spec_parse(mv_attrspec* ptr, char* key, char* value, int rel) {
	switch(rel) {
		case MVAST_TYPESPEC:
			ptr->type = MVSPEC_TYPE;
			if (STREQ(value, "string")) {
				ptr->value.typespec.type = MVTYPE_STRING;
				ptr->value.typespec.classname = NULL;
			} else {
				ptr->value.typespec.type = MVTYPE_RAWREF;
				ptr->value.typespec.classname = strdup(value);
			}	
			break;
		default:
			DIE("Unknown relationship code (%d)", rel);
	}
	ptr->name = strdup(key);
}

mv_error* mv_tokenize(mv_strarr* target, char* data) {
	assert(data != NULL);
	assert(target != NULL);
	enum { WHITESPACE, LITERAL, TOKEN } state = WHITESPACE;
	int base = 0, scan;
	mv_strarr_alloc(target, 10);
	for (scan = 0; data[scan] != '\0'; scan++) {
		switch (data[scan]) {
		case ' ': case '\t': case '\n':
			if (state == TOKEN) {
				state = WHITESPACE;
				mv_strarr_appslice(target, data, base, scan);
			}
			break;
		case '\'':
			if (state == WHITESPACE) {
				base = scan;
				state = LITERAL;
			} else if (state == LITERAL) {
				mv_strarr_appslice(target, data, base, scan);
				state = WHITESPACE;
			} else {
				mv_strarr_appslice(target, data, base, scan - 1);
				state = LITERAL;
			}
			break;
		case ',': case '{': case '}': case ':':
			if (state == WHITESPACE) {
				mv_strarr_appslice(target, data, scan, scan + 1);
			} else if (state == TOKEN) {
				mv_strarr_appslice(target, data, base, scan);
				mv_strarr_appslice(target, data, scan, scan + 1);
				state = 0;
			}
			break;
		default:
			if (state == WHITESPACE) {
				state = TOKEN;
				base = scan;
			}
		}	
	}
	if (state == 1) {
		mv_strarr_release(target);
		return mv_error_unmatched(MVAST_TEMPAPOSTROPHE, data);
	}
	if (state == 2) {
		mv_strarr_appslice(target, data, base, scan);
	}
	target->items = realloc(target->items, sizeof(char*) * target->used);
	target->size = target->used;
	return NULL;
}
