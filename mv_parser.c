
#include <assert.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include "utils.h"

int __mvp_ispair(mv_ast_entry* entry) {
	return (entry->type == MVAST_ATTRPAIR);
}

int __mvp_isleaf(mv_ast_entry* entry) {
	return (entry->type == MVAST_LEAF);
}

int __mvp_istemplist(mv_ast_entry* entry) {
	return (entry->type == MVAST_TEMPATTRLIST);
}

int __mvp_isfixed(mv_ast_entry* entry, char* text) {
	return (entry->type == MVAST_LEAF) &&
	       (strcmp(entry->value.leaf, text) == 0);
}

#define FIXED(var, code) ((var->type) == MVAST_TEMP##code)

mv_error* mv_ast_parse(mv_ast* target, char* data) {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, data);
	if (error != NULL) {
		return error;
	}
	mv_ast_entry* stack = (mv_ast_entry*)malloc(sizeof(mv_ast_entry) * tokens.used);
	int scan = 0, size = 0;

	while (scan < tokens.used) {
		if (strcmp(tokens.items[scan], "{") == 0) {
			stack[size].type = MVAST_TEMPOPENBRACE;
		} else if (strcmp(tokens.items[scan], "}") == 0) {
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

			if (FIXED(e1, OPENBRACE) && FIXED(e2, CLOSEBRACE)) {
				e1->type = MVAST_ATTRLIST;
				e1->value.subtree.size = 0;
				e1->value.subtree.items = NULL;
				size--;
				continue;
			}

			if (size < 3) break;

			e1 = stack + size - 3;
			e2 = e1 + 1;
			mv_ast_entry *e3 = e1 + 2;
			
			if (__mvp_isleaf(e1) && __mvp_isfixed(e2, "=") && __mvp_isleaf(e3)) {
				mv_ast_entry* tmp = (mv_ast_entry*)malloc(sizeof(mv_ast_entry) * 2);
				tmp[0] = *e1;
				tmp[1] = *e3;
				free(e2->value.leaf);
				e1->type = MVAST_ATTRPAIR;
				e1->value.subtree.size = 2;
				e1->value.subtree.items = tmp;
				size -= 2;
				continue;
			}
			
			if (FIXED(e1, OPENBRACE) && __mvp_ispair(e2) && FIXED(e3, CLOSEBRACE)) {
				mv_ast_entry* tmp = (mv_ast_entry*)malloc(sizeof(mv_ast_entry));
				tmp[0] = *e2;
				e1->type = MVAST_ATTRLIST;
				e1->value.subtree.size = 1;
				e1->value.subtree.items = tmp;
				size -= 2;
				continue;				
			}

			if (FIXED(e1, OPENBRACE) && __mvp_istemplist(e2) && FIXED(e3, CLOSEBRACE)) {
				e1->type = MVAST_ATTRLIST;
				e1->value.subtree = e2->value.subtree;
				size -= 2;
				continue;				
			}
			
			if (__mvp_ispair(e1) && __mvp_isfixed(e2, ",") && __mvp_ispair(e3)) {
				mv_ast_entry* tmp = (mv_ast_entry*)malloc(sizeof(mv_ast_entry) * 2);
				tmp[0] = *e1;
				tmp[1] = *e3;
				free(e2->value.leaf);
				e1->type = MVAST_TEMPATTRLIST;
				e1->value.subtree.size = 2;
				e1->value.subtree.items = tmp;
				size -= 2;
				continue;		
			}
		} while (oldsize != size);
	}
	
	stack = (mv_ast_entry*)realloc(stack, sizeof(mv_ast_entry) * size);
	free(tokens.items);
	
	target->size = size;
	target->items = stack;

	int i;
	for (i=0; i < size; i++) {
		if (stack[i].type < 0) {
			mv_ast_release(target);
			return mv_error_raise(MVERROR_SYNTAX, "Unmatched temporary objects");
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

int mv_ast_to_attrlist(mv_attrlist* target, mv_ast* source) {
	mv_attrlist_alloc(target, source->size);
	int i;
	for (i=0; i<source->size; i++) {
		assert(source->items[i].type == MVAST_ATTRPAIR);
		mv_ast astpair = source->items[i].value.subtree;
		assert(astpair.size == 2);
		assert(astpair.items[0].type == MVAST_LEAF);
		assert(astpair.items[1].type == MVAST_LEAF);
		mv_attr_parse(target->attrs + i, astpair.items[0].value.leaf, astpair.items[1].value.leaf);
	}
	return 0;
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
	
mv_error* mv_command_parse(mv_command* target, char* data) {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, data);
	if (error != NULL) return error;

	if (__mvp_isfixed(ast.items, "create") &&
	    __mvp_isfixed(ast.items + 1, "entity"))
	{
		target->code = MVCMD_CREATE_ENTITY;
		if (ast.items[2].type != MVAST_ATTRLIST) {
			char buffer[200];
			mv_ast_release(&ast);
			sprintf(buffer, "Expected AttrList, got %d", ast.items[2].type);
			return mv_error_raise(MVERROR_INTERNAL, buffer);
		}
		assert(ast.items[2].type == MVAST_ATTRLIST);
		assert(ast.size >= 3);
		assert(ast.size <= 4);
		mv_ast_to_attrlist(&(target->attrs), &(ast.items[2].value.subtree));
		if (ast.size == 4) {
			assert(ast.items[3].type == MVAST_LEAF);
			mv_strarr_alloc(&target->vars, 1);
			mv_strarr_append(&target->vars, ast.items[3].value.leaf);
		} else {
			mv_strarr_alloc(&target->vars, 0);
		}
		mv_ast_release(&ast);
		return NULL;
	}
	if (__mvp_isfixed(ast.items, "show"))
	{
		assert(ast.size == 2);
		assert(ast.items[1].type == MVAST_LEAF);
		target->code = MVCMD_SHOW;
		target->attrs.size = 0;
		target->attrs.attrs = NULL;
		mv_strarr_alloc(&target->vars, 1);
		mv_strarr_append(&target->vars, ast.items[1].value.leaf);
		mv_ast_release(&ast);
		return NULL;
	}
	if (__mvp_isfixed(ast.items, "quit"))
	{
		assert(ast.size == 1);
		target->code = MVCMD_QUIT;
		target->attrs.size = 0;
		target->vars.size = 0;
		target->vars.used = 0;
		mv_ast_release(&ast);
		return NULL;
	}

	return mv_error_raise(MVERROR_SYNTAX, "Syntax error");
}

mv_error* mv_tokenize(mv_strarr* target, char* data) {
	assert(data != NULL);
	assert(target != NULL);
	int count = 0;
	// states: 0 - in whitespace, 1 - in literal, 2 - in token
	int state = 0;
	int scan, base, fill;
	for (scan = 0; data[scan] != '\0'; scan++) {
		switch (data[scan]) {
		case ' ': case '\t': case '\n':
			if (state == 2) state = 0;
			break;
		case '\'':
			if (state == 1) state = 0;
			else {
				state = 1;
				count++;
			}
			break;
		case ',': case '{': case '}':
			if ((state == 0) || (state == 2)) {
				state = 0;
				count++;
			}
			break;
		default:
			if (state == 0) {
				state = 2;
				count++;
			}
		}
	}
	if (state == 1) {
		return mv_error_raise(MVERROR_SYNTAX, "Unmatched \"'\"");
	}
	mv_strarr_alloc(target, count);
	state = 0;
	base = 0;
	fill = 0;
	for (scan = 0; data[scan] != '\0'; scan++) {
		switch (data[scan]) {
		case ' ': case '\t': case '\n':
			if (state == 2) {
				state = 0;
				mv_strarr_appslice(target, data, base, scan);
			}
			break;
		case '\'':
			if (state == 0) {
				base = scan;
				state = 1;
			} else if (state == 1) {
				mv_strarr_appslice(target, data, base, scan);			
				state = 0;
			} else if (state == 2) {
				mv_strarr_appslice(target, data, base, scan - 1);			
				state = 1;
			}
			break;
		case ',': case '{': case '}':
			if (state == 0) {
				mv_strarr_appslice(target, data, scan, scan + 1);
			} else if (state == 2) {
				mv_strarr_appslice(target, data, base, scan);
				mv_strarr_appslice(target, data, scan, scan + 1);
				state = 0;
			}
			break;
		default:
			if (state == 0) {
				state = 2;
				base = scan;
			}
		}	
	}
	if (state == 2) {
		mv_strarr_appslice(target, data, base, scan);
	}
	return NULL;
}