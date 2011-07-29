
#include <assert.h>
#include <ctype.h>
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

inline static void __pair__(mv_ast_entry* stack, int* count) {
	if (*count < 3) return;
	mv_ast_entry *a = &(stack[*count - 3]);
	if (!LEAF(a)) return;
	mv_ast_entry *b = &(stack[*count - 1]);
	if (!LEAF(b)) return;
	mv_ast_entry *sep = &(stack[*count - 2]);

	int typecode = 0;
	if (TEMPFIX(sep, EQUALS)) typecode = MVAST_ATTRPAIR;
	else if (TEMPFIX(sep, COLON)) typecode = MVAST_TYPESPEC;
	else return;

	mv_ast_entry* tmp = malloc(sizeof(mv_ast_entry) * 2);
	tmp[0] = *a;
	tmp[1] = *b;

	a->type = typecode;
	a->value.subtree.size = 2;
	a->value.subtree.items = tmp;
	(*count)-=2;
}

inline static void __comma__(mv_ast_entry* stack, int *count) {
	if (*count < 3) return;
	mv_ast_entry *sep = &(stack[*count - 2]);
	if (! TEMPFIX(sep, COMMA)) return;

	mv_ast_entry *a = &(stack[*count - 3]);
	mv_ast_entry *b = &(stack[*count - 1]);

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
} // style:40

inline static void __emptylist__(mv_ast_entry* stack, int* size) {
	if (*size < 2) return;

	mv_ast_entry *a = &(stack[*size - 2]), *b = &(stack[*size - 1]);

	if (TEMPFIX(a, OPENBRACE) && TEMPFIX(b, CLOSEBRACE)) {
		a->type = MVAST_ATTRLIST;
		a->value.subtree.size = 0;
		a->value.subtree.items = NULL;
		(*size)--;
	}
}

inline static void __list__(mv_ast_entry* stack, int* count) {
	if ((*count) < 3) return;

	mv_ast_entry *e1 = &(stack[*count - 3]);
	mv_ast_entry *e3 = &(stack[*count - 1]);

	if (!TEMPFIX(e1, OPENBRACE) || !TEMPFIX(e3, CLOSEBRACE)) {
		return;
	}

	mv_ast_entry *e2 = &(stack[*count - 2]);

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
		(*count) -= 2;
		return;
	}

	if (TEMPFIX(e2, ATTRLIST) || TEMPFIX(e2, ATTRSPECLIST)) {
		if (TEMPFIX(e2, ATTRLIST)) {
			e1->type = MVAST_ATTRLIST;
		} else {
			e1->type = MVAST_ATTRSPECLIST;
		}
		e1->value.subtree = e2->value.subtree;
		(*count) -= 2;
		return;
	}
}

inline static mv_ast_entry __make_token__(char* token) {
	mv_ast_entry result;

	if (STREQ(token, "{")) result.type = MVAST_TEMPOPENBRACE;
	else if (STREQ(token, "}")) result.type = MVAST_TEMPCLOSEBRACE;
	else if (STREQ(token, ",")) result.type = MVAST_TEMPCOMMA;
	else if (STREQ(token, ":")) result.type = MVAST_TEMPCOLON;
	else if (STREQ(token, "=")) result.type = MVAST_TEMPEQUALS;
	else {
		result.type = MVAST_LEAF;
		result.value.leaf = token;
		return result;
	}

	free(token);
	return result;
}

static inline void __compress__(mv_ast_entry* stack, int* size) {
	int oldsize;
	do {
		oldsize = *size;
		__emptylist__(stack, size);
		__pair__(stack, size);
		__comma__(stack, size);
		__list__(stack, size);
	} while (oldsize != *size);
}

mv_error* mv_ast_parse(mv_ast* target, char* data) {
	mv_strarr tokens;
	FAILRET(mv_tokenize(&tokens, data));
	mv_ast_entry* stack = malloc(sizeof(mv_ast_entry) * tokens.used);
	int i, scan = 0, size = 0;

	while (scan < tokens.used) {
		stack[size++] = __make_token__(tokens.items[scan++]);
		__compress__(stack, &size);
	}
	
	free(tokens.items);

	target->size = size;
	stack = realloc(stack, sizeof(mv_ast_entry) * size);
	target->items = stack;

	for (i=0; i < size; i++) {
		if (stack[i].type < 0) {
			mv_error* err = mv_error_unmatched(stack[i].type, data);
			mv_ast_release(target);
			return err;
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
		case MVAST_TEMPCOMMA:
		case MVAST_TEMPCOLON:
		case MVAST_TEMPEQUALS:
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
		EXPECT(srcitem.type == MVAST_ATTRPAIR, "AttrPair expected");
		mv_ast astpair = srcitem.value.subtree;
		EXPECT(astpair.size == 2, "Two elements expected");
		mv_ast_entry* items = astpair.items;
		EXPECT(LEAF(&items[0]), "Leafs expected");
		EXPECT(LEAF(&items[1]), "Leafs expected");
		char *key = items[0].value.leaf, *value = items[1].value.leaf;
		mv_attr_parse(&target->attrs[i], key, value);
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

static inline int __parse_number__(mv_attr* target, char* value) {
	int ival = 0;
	if (*value == '\0') return 0;
	while (1) {
		if (isdigit(*value)) {
			ival = ival * 10 + (*value - '0');
			value++;
			continue;
		} else if (*value == '\0') {
			target->type = MVTYPE_INTEGER;
			target->value.integer = ival;
			return 1;
		} else {
			return 0;
		}
	}
}

void mv_attr_parse(mv_attr* target, char* name, char* value) {
	target->name = strdup(name);
	if (value[0] == '\'') {
		target->type = MVTYPE_STRING;
		target->value.string = strdup(value + 1);
		return;
	}
	if (__parse_number__(target, value)) {
		return;
	}
	target->type = MVTYPE_RAWREF;
	target->value.rawref = strdup(value);
}

inline static void __clear__(mv_command* cmd, int code, int vars) {
	cmd->code = code;
	cmd->attrs.size = 0;
	cmd->attrs.attrs = NULL;
	cmd->spec.size = 0;
	cmd->spec.specs = NULL;
	if (vars != 0) {
		mv_strarr_alloc(&cmd->vars, vars);
	} else {
		cmd->vars.used = 0;
		cmd->vars.size = 0;
		cmd->vars.items = NULL;
	}
}

mv_error* __createentity_parse(mv_command* target, mv_ast* ast) {
	if (ast->size < 3) {
		THROW(SYNTAX, "'create entity' command is incomplete");
	} else if (ast->size > 4) {
		THROW(SYNTAX, "'create entity' command is malformed");
	} else if (ast->items[2].type != MVAST_ATTRLIST) {
		THROW(INTERNAL,
		     "Expected AttrList in 'create entity', got %d",
		     ast->items[2].type);
	} else if (ast->size == 4) {
		if (ast->items[3].type != MVAST_LEAF) {
			THROW(INTERNAL,
			      "Expected Leaf for 'create entity' command, got %d",
			      ast->items[3].type);
		}
		__clear__(target, MVCMD_CREATE_ENTITY, 1);
		mv_strarr_append(&target->vars, ast->items[3].value.leaf);
	} else {
		__clear__(target, MVCMD_CREATE_ENTITY, 0);
	}

	mv_ast_to_attrlist(&target->attrs, &ast->items[2].value.subtree);
	mv_ast_release(ast);
	return NULL;
}

mv_error* __create__(mv_command* target, mv_ast* ast) {
	int size = ast->size;
	mv_ast_entry* items = ast->items;
	if ((size == 1) || !(LEAF(&items[1]))) {
		THROW(INTERNAL, "Malformed 'create' command");
	}

	if (LEAFFIX(items + 1, "entity")) {
		return __createentity_parse(target, ast);
	}

	if (LEAFFIX(items + 1, "class")) {
		target->code = MVCMD_CREATE_CLASS;
		if (!LEAF(&items[2])) {
			THROW(INTERNAL,
			      "Expected class name, got %d",
			      items[2].type);
		}
		if (items[3].type != MVAST_ATTRSPECLIST) {
			THROW(INTERNAL,
			      "Expected AttrSpecList, got %d",
			      items[3].type);
		}
		mv_ast_to_speclist(&(target->spec), &(items[3].value.subtree));
		mv_strarr_alloc(&target->vars, 1);
		mv_strarr_append(&target->vars, items[2].value.leaf);
		target->attrs.size = 0;
		target->attrs.attrs = NULL;
		mv_ast_release(ast);
		return NULL;
	}
	
	THROW(SYNTAX, "Invalid task for create - '%s'", items[1].value.leaf);
}

mv_error* __destroy__(mv_command* target, mv_ast* ast) {
	int size = ast->size;
	mv_ast_entry* items = ast->items;
	if ((size != 3) || !LEAFFIX(&items[1], "entity") ||
	    !LEAF(&items[2]))
	{
		THROW(SYNTAX, "Malformed 'destroy' command");
	}
	__clear__(target, MVCMD_DESTROY_ENTITY, 1);
	mv_strarr_append(&target->vars, ast->items[2].value.leaf);
	mv_ast_release(ast);
	return NULL;	
} 

mv_error* __showcmd_parse(mv_command* cmd, mv_ast* ast) {
	int size = ast->size;
	mv_ast_entry* items = ast->items;
	assert(size == 2);
	assert(items[1].type == MVAST_LEAF);
	__clear__(cmd, MVCMD_SHOW, 1);
	mv_strarr_append(&cmd->vars, items[1].value.leaf);
	mv_ast_release(ast);
	return NULL;
}

static mv_error* __quitcmd_parse(mv_command* cmd, mv_ast* ast) {
	if (ast->size != 1) {
		THROW(SYNTAX, "Malformed 'quit' command");
	}
	__clear__(cmd, MVCMD_QUIT, 0);
	mv_ast_release(ast);
	return NULL;
}

inline static mv_error* __assign__(mv_command* cmd, mv_ast* ast) {
	int size = ast->size;
	mv_ast_entry* items = ast->items;
	assert(size == 4);
	assert(LEAF(&(items[1])));
	assert(LEAFFIX(&(items[2]), "to"));
	assert(LEAF(&(items[3])));
	__clear__(cmd, MVCMD_ASSIGN, 2);
	mv_strarr_append(&cmd->vars, items[1].value.leaf);
	mv_strarr_append(&cmd->vars, items[3].value.leaf);
	mv_ast_release(ast);
	return NULL;
}

inline static mv_error* __lookup__(mv_command* cmd, mv_ast* ast) {
	assert(LEAF(&(ast->items[1])));
	if (ast->size == 2) {
		__clear__(cmd, MVCMD_LOOKUP, 1);
		mv_strarr_append(&cmd->vars, ast->items[1].value.leaf);
		mv_ast_release(ast);
		return NULL;
	}
	assert(ast->size == 4);
	assert(LEAFFIX(&(ast->items[2]), "with"));
	assert(ast->items[3].type == MVAST_ATTRLIST);
	__clear__(cmd, MVCMD_LOOKUP, 1);
	mv_strarr_append(&cmd->vars, ast->items[1].value.leaf);
	mv_ast_to_attrlist(&cmd->attrs, &ast->items[3].value.subtree);
	mv_ast_release(ast);
	return NULL;
}

mv_error* mv_command_parse(mv_command* cmd, char* data) {
	mv_ast ast;
	mv_error* error = mv_ast_parse(&ast, data);
	if (error != NULL) return error;
	if (!LEAF(&(ast.items[0]))) {
		THROW(SYNTAX, "Syntax error");
	}
	char* cmdname = ast.items[0].value.leaf;

	if (STREQ(cmdname, "assign")) return __assign__(cmd, &ast);
	if (STREQ(cmdname, "create")) return __create__(cmd, &ast);
	if (STREQ(cmdname, "destroy")) return __destroy__(cmd, &ast);
	if (STREQ(cmdname, "lookup")) return __lookup__(cmd, &ast);
	if (STREQ(cmdname, "show")) return __showcmd_parse(cmd, &ast);
	if (STREQ(cmdname, "quit")) return __quitcmd_parse(cmd, &ast);

	THROW(BADCMD, ast.items[0].value.leaf);
}

void mv_spec_parse(mv_attrspec* ptr, char* key, char* value, int rel) {
	int code;

	switch(rel) {
		case MVAST_TYPESPEC:
			ptr->type = MVSPEC_TYPE;
			code = -1;
			ptr->value.typespec.classname = NULL;
			if (STREQ(value, "string")) {
				code = MVTYPE_STRING;
			} else if (STREQ(value, "integer")) {
				code = MVTYPE_INTEGER;
			} else {
				code = MVTYPE_RAWREF;
				ptr->value.typespec.classname = strdup(value);
			}
			ptr->value.typespec.type = code;
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
} // style:60
