
#include <assert.h>
#include <ctype.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include "parser.h"

#define LIST(var)  ((var)->type == MVAST_ATTRLIST)
#define PAIR(var)  ((var)->type == MVAST_ATTRPAIR)
#define QPAIR(var) ((var)->type == MVAST_ATTRQUERY)
#define SPEC(var)  ((var)->type == MVAST_TYPESPEC)
#define SUBQ(var)  ((var)->type == MVAST_SUBQUERY)
#define LEAF(var)  ((var)->type == MVAST_LEAF)
#define LEAFFIX(var, text) (LEAF(var) && STREQ((var)->value.leaf->ptr, text))
#define TEMPFIX(var, code) ((var->type) == MVAST_TEMP##code)

mv_ast::mv_ast() :
	mvStaticArray<mv_ast_entry>(0)
{
}

mv_ast::mv_ast(mv_ast_entry& item) :
	mvStaticArray<mv_ast_entry>(1)
{
	items[0] = item;
}

mv_ast::mv_ast(mv_ast_entry& fst, mv_ast_entry& snd) :
	mvStaticArray<mv_ast_entry>(2)
{
	items[0] = fst;
	items[1] = snd;
}

inline static void __pair__(mv_ast_entry* stack, int* count) {
	if (*count < 3) return;
	mv_ast_entry *a = &(stack[*count - 3]);
	if (!LEAF(a)) return;
	mv_ast_entry *b = &(stack[*count - 1]);
	mv_ast_entry *sep = &(stack[*count - 2]);

	int typecode = 0;
	if (TEMPFIX(sep, EQUALS)) {
		if (LEAF(b)) {
			typecode = MVAST_ATTRPAIR;
		} else if (SUBQ(b)) {
			typecode = MVAST_ATTRQUERY;
		} else {
			return;
		}
	} else if (TEMPFIX(sep, COLON)) {
		if (LEAF(b)) {
			typecode = MVAST_TYPESPEC;
		} else {
			return;
		}
	} else return;

	a->value.subtree = new mv_ast(*a, *b);
	a->type = typecode;
	(*count)-=2;
}

inline static void __comma__(mv_ast_entry* stack, int *count) {
	if (*count < 3) return;
	mv_ast_entry *sep = &(stack[*count - 2]);
	if (! TEMPFIX(sep, COMMA)) return;

	mv_ast_entry *a = &(stack[*count - 3]);
	mv_ast_entry *b = &(stack[*count - 1]);

	if (TEMPFIX(a, ATTRLIST) && PAIR(b)) {	
		a->value.subtree->push(*b);
		
		*count -= 2;
	}

	if (PAIR(a) && PAIR(b)) {
		a->value.subtree = new mv_ast(*a, *b);
		a->type = MVAST_TEMPATTRLIST;
		*count -= 2;
		return;
	}

	if (SPEC(a) && SPEC(b)) {
		a->value.subtree = new mv_ast(*a, *b);
		a->type = MVAST_TEMPATTRSPECLIST;
		*count -= 2;
		return;
	}
} // style:40

inline static void __emptylist__(mv_ast_entry* stack, int* size) {
	if (*size < 2) return;

	mv_ast_entry *a = &(stack[*size - 2]), *b = &(stack[*size - 1]);

	if (TEMPFIX(a, OPENBRACE) && TEMPFIX(b, CLOSEBRACE)) {
		a->type = MVAST_ATTRLIST;
		a->value.subtree = new mv_ast;
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

	if (PAIR(e2) || SPEC(e2) || QPAIR(e2)) {
		if (PAIR(e2)) {
			e1->type = MVAST_ATTRLIST;
		} else {
			e1->type = MVAST_ATTRSPECLIST;
		}
		e1->value.subtree = new mv_ast(*e2);
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

inline static void __subquery__(mv_ast_entry* stack, int* count) {
	if ((*count) < 5) return;

	mv_ast_entry *e1 = &(stack[*count - 5]);
	mv_ast_entry *e5 = &(stack[*count - 1]);
	if (!TEMPFIX(e1, OPENBRACKET) || !TEMPFIX(e5, CLOSEBRACKET)) return;

	mv_ast_entry *e2 = &(stack[*count - 4]);
	if (!LEAF(e2)) return;

	mv_ast_entry *e3 = &(stack[*count - 3]);
	if (!LEAFFIX(e3, "with")) return;

	mv_ast_entry *e4 = &(stack[*count - 2]);
	if (!LIST(e4)) return;

	mv_strref_free(e3->value.leaf);
	free(e3->value.leaf);

	e1->type = MVAST_SUBQUERY;
	e1->value.subtree = new mv_ast(*e2, *e4);

	(*count) -= 4;
}

inline static int __make_char_token__(char token) {
	switch (token) {
	case '{': return MVAST_TEMPOPENBRACE;
	case '}': return MVAST_TEMPCLOSEBRACE;
	case ',': return MVAST_TEMPCOMMA;
	case ':': return MVAST_TEMPCOLON;
	case '=': return MVAST_TEMPEQUALS;
	case '[': return MVAST_TEMPOPENBRACKET;
	case ']': return MVAST_TEMPCLOSEBRACKET;
	default:  return MVAST_LEAF;
	}
}

inline static mv_ast_entry __make_token__(mv_strref* token) {
	mv_ast_entry result;

	if (strlen(token->ptr) == 1) {
		int code = __make_char_token__(token->ptr[0]);
		if (code != MVAST_LEAF) {
			result.type = code;
			mv_strref_free(token);
			return result;	
		}
	}
	result.type = MVAST_LEAF;
	result.value.leaf = (mv_strref*)malloc(sizeof(mv_strref));
	*(result.value.leaf) = *token;
	return result;
}

inline static void __compress__(mv_ast_entry* stack, int* size) {
	int oldsize;
	do {
		oldsize = *size;
		__emptylist__(stack, size);
		__pair__(stack, size);
		__comma__(stack, size);
		__list__(stack, size);
		__subquery__(stack, size);
	} while (oldsize != *size);
}

mv_error* mv_ast_parse(mv_ast& target, const char* data) {
	mv_strarr tokens;
	FAILRET(mv_tokenize(&tokens, data));
	mv_ast_entry* stack = (mv_ast_entry*)malloc(sizeof(mv_ast_entry) * tokens.used);
	int i, scan = 0, size = 0;

	while (scan < tokens.used) {
		stack[size++] = __make_token__(&(tokens.items[scan++]));
		__compress__(stack, &size);
	}
	
	free(tokens.items);

	target.set(stack, size);
	free(stack);

	for (i=0; i < size; i++) {
		if (target[i].type < 0) {
			return mv_error_unmatched(target[i].type, data);
		}
	}

	return NULL;
}

void mv_ast_entry::clear()
{
	switch(type) {
	case MVAST_LEAF:
		mv_strref_free(value.leaf);
		free(value.leaf);
		break;
	case MVAST_TEMPOPENBRACE:
	case MVAST_TEMPCLOSEBRACE:
	case MVAST_TEMPCOMMA:
	case MVAST_TEMPCOLON:
	case MVAST_TEMPEQUALS:
		break;
	case MVAST_ATTRLIST:
	case MVAST_ATTRPAIR:
	case MVAST_TYPESPEC:
	case MVAST_ATTRSPECLIST:
	case MVAST_SUBQUERY:
	case MVAST_ATTRQUERY:
		delete value.subtree;
		break;
	default:
		printf("code = %d\n", type);
	}
}

void mv_attrlist_parse(mv_attrlist* target, mv_ast* source) {
	mv_attrlist_alloc(target, source->size());
	int i;
	for (i=0; i<source->size(); i++) {
		mv_ast_entry srcitem = source->items[i];
		EXPECT(srcitem.type == MVAST_ATTRPAIR, "AttrPair expected");
		mv_ast* astpair = srcitem.value.subtree;
		EXPECT(astpair->size() == 2, "Two elements expected");
		mv_ast_entry* items = astpair->items;
		EXPECT(LEAF(&items[0]), "Leafs expected");
		EXPECT(LEAF(&items[1]), "Leafs expected");
		char *key = items[0].value.leaf->ptr;
		char *value = items[1].value.leaf->ptr;
		mv_attr_parse(&target->attrs[i], key, value);
	}
}

void mv_speclist_parse(mv_speclist& target, mv_ast* source) {
	target.alloc(source->size());
	int i;
	for (i=0; i<source->size(); i++) {
		mv_ast_entry src = source->items[i];
		EXPECT(
		    src.value.subtree->size() == 2,
			"Two elements expected"
		);
		mv_ast_entry* items = src.value.subtree->items;
		EXPECT(LEAF(&items[0]), "Leaf expected as a first item");
		char* key = items[0].value.leaf->ptr;
		switch (src.type) {
		case MVAST_ATTRQUERY:
			EXPECT(
			    SUBQ(&items[1]),
			    "First item of AttrQuery should be a Subquery"
			);
			mv_attrquery_parse(
			    &(target[i]), key, *items[1].value.subtree
			);
			break;
		case MVAST_ATTRPAIR:
		case MVAST_TYPESPEC:
			EXPECT(LEAF(&items[1]), "Leaf expected as a second item");
			mv_spec_parse(
				&(target[i]), key, items[1].value.leaf->ptr, src.type
			);
			break;
		default:
			DIE("Invalid AST element code: %d", src.type);
		}
	}
}

inline static int __parse_number__(mv_attr* target, char* value) {
	int ival = 0;
	if (*value == '\0') return 0;
	while (1) {
		if (isdigit(*value)) {
			ival = ival * 10 + (*value - '0');
			value++;
			continue;
		} else if (*value == '\0') {
			target->type = INTEGER;
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
		target->type = STRING;
		target->value.string = strdup(value + 1);
		return;
	}
	if (__parse_number__(target, value)) {
		return;
	}
	target->type = RAWREF;
	target->value.rawref = strdup(value);
}

inline static void __clear__(mv_command* cmd, mvCommandType code, int vars) {
	cmd->code = code;
	cmd->attrs.size = 0;
	cmd->attrs.attrs = NULL;
	cmd->spec.clear();
	if (vars != 0) {
		mv_strarr_alloc(&cmd->vars, vars);
	} else {
		cmd->vars.used = 0;
		cmd->vars.size = 0;
		cmd->vars.items = NULL;
	}
}

mv_error* __create_entity__(mv_command* target, mv_ast* ast) {
	if (ast->size() < 3) {
		THROW(SYNTAX, "'create entity' command is incomplete");
	} else if (ast->size() > 4) {
		THROW(SYNTAX, "'create entity' command is malformed");
	}

	mv_ast_entry* items = ast->items;

	if (!LIST(&items[2])) {
		THROW(INTERNAL,
		     "Expected AttrList in 'create entity', got %d",
		     items[2].type);
	}

	if (ast->size() == 4) {
		if (!LEAF(&items[3])) {
			THROW(INTERNAL,
			      "Expected Leaf for 'create entity' command, got %d",
			      ast->items[3].type);
		}
		__clear__(target, CREATE_ENTITY, 1);
		mv_strarr_appref(&target->vars, ast->items[3].value.leaf);
	} else {
		__clear__(target, CREATE_ENTITY, 0);
	}

	mv_attrlist_parse(&target->attrs, ast->items[2].value.subtree);
	return NULL;
}

mv_error* __create__(mv_command* target, mv_ast* ast) {
	int size = ast->size();
	mv_ast_entry* items = ast->items;
	if ((size == 1) || !(LEAF(&items[1]))) {
		THROW(INTERNAL, "Malformed 'create' command");
	}

	if (LEAFFIX(&items[1], "entity")) {
		return __create_entity__(target, ast);
	}

	if (LEAFFIX(&items[1], "class")) {
		target->code = CREATE_CLASS;
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
		mv_speclist_parse(target->spec, items[3].value.subtree);
		mv_strarr_alloc(&target->vars, 1);
		mv_strarr_appref(&target->vars, items[2].value.leaf);
		target->attrs.size = 0;
		target->attrs.attrs = NULL;
		return NULL;
	}
	
	THROW(SYNTAX,
	      "Invalid task for create - '%s'",
	      items[1].value.leaf);
}

mv_error* __destroy__(mv_command* target, mv_ast* ast) {
	int size = ast->size();
	mv_ast_entry* items = ast->items;
	if ((size != 3) || !LEAFFIX(&items[1], "entity") ||
	    !LEAF(&items[2]))
	{
		THROW(SYNTAX, "Malformed 'destroy' command");
	}
	__clear__(target, DESTROY_ENTITY, 1);
	mv_strarr_appref(&target->vars, ast->items[2].value.leaf);
	return NULL;	
} 

inline static mv_error* __show__(mv_command* cmd, mv_ast* ast) {
	int size = ast->size();
	mv_ast_entry* items = ast->items;
	assert(size == 2);
	assert(items[1].type == MVAST_LEAF);
	__clear__(cmd, SHOW, 1);
	mv_strarr_appref(&cmd->vars, items[1].value.leaf);
	return NULL;
}

inline static mv_error* __quit__(mv_command* cmd, mv_ast* ast) {
	if (ast->size() != 1) {
		THROW(SYNTAX, "Malformed 'quit' command");
	}
	__clear__(cmd, QUIT, 0);
	return NULL;
}

inline static mv_error* __assign__(mv_command* cmd, mv_ast* ast) {
	int size = ast->size();
	mv_ast_entry* items = ast->items;
	assert(size == 4);
	assert(LEAF(&(items[1])));
	assert(LEAFFIX(&(items[2]), "to"));
	assert(LEAF(&(items[3])));
	__clear__(cmd, ASSIGN, 2);
	mv_strarr_appref(&cmd->vars, items[1].value.leaf);
	mv_strarr_appref(&cmd->vars, items[3].value.leaf);
	return NULL;
}

inline static mv_error* __lookup__(mv_command* cmd, mv_ast* ast) {
	assert(LEAF(&(ast->items[1])));
	if (ast->size() == 2) {
		__clear__(cmd, LOOKUP, 1);
		mv_strarr_appref(&cmd->vars, ast->items[1].value.leaf);
		return NULL;
	}
	assert(ast->size() == 4);
	assert(LEAFFIX(&(ast->items[2]), "with"));
	assert(ast->items[3].type == MVAST_ATTRLIST);
	__clear__(cmd, LOOKUP, 1);
	mv_strarr_appref(&cmd->vars, ast->items[1].value.leaf);
	mv_attrlist_parse(&cmd->attrs, ast->items[3].value.subtree);
	return NULL;
}

inline static
mv_error* __update_entity__(mv_command* target, mv_ast* ast) {
	mv_ast_entry* items = ast->items;
	if (ast->size() != 5) goto wrong; 
	if (!LEAF(&items[2])) goto wrong;
	if (LEAFFIX(&items[3], "with")) {
		if (!LIST(&items[4])) goto wrong;
		__clear__(target, UPDATE_ENTITY, 1);
		mv_strarr_appref(&target->vars, items[2].value.leaf);
		mv_attrlist_parse(&target->attrs, items[4].value.subtree);
		return NULL;
	} 

wrong:
	THROW(SYNTAX, "'update entity' command is malformed");
}

inline static
mv_error* __update__(mv_command* target, mv_ast* ast) {
	int size = ast->size();
	mv_ast_entry* items = ast->items;
	if ((size == 1) || !(LEAF(&items[1]))) {
		THROW(INTERNAL, "Malformed 'update' command");
	}

	if (LEAFFIX(&items[1], "entity")) {
		return __update_entity__(target, ast);
	}	

	mv_error* err;
	PREPARE_ERROR(err, SYNTAX,
	      "Invalid task for update - '%s'",
		  items[1].value.leaf);
	return err;
}

void mv_command_parse(mvCommand& command, const char* data)
throw (mv_error*)
{
	mv_ast ast;
	command.destroy();
	mv_command* cmd = &command;
	mv_error* error = mv_ast_parse(ast, data);
	if (error != NULL) throw error;
	if (!LEAF(&(ast[0]))) {
		NEWTHROW(SYNTAX, "Syntax error");
	}
	char* cmdname = ast[0].value.leaf->ptr;

	if (STREQ(cmdname, "assign"))  error = __assign__(cmd, &ast);
	else if (STREQ(cmdname, "create"))  error = __create__(cmd, &ast);
	else if (STREQ(cmdname, "destroy")) error = __destroy__(cmd, &ast);
	else if (STREQ(cmdname, "lookup"))  error = __lookup__(cmd, &ast);
	else if (STREQ(cmdname, "show"))    error = __show__(cmd, &ast);
	else if (STREQ(cmdname, "quit"))    error = __quit__(cmd, &ast);
	else if (STREQ(cmdname, "update"))  error = __update__(cmd, &ast);
	else NEWTHROW(BADCMD, cmdname);

	if (error != NULL) throw error;
	command.init_done();
}

void mv_spec_parse(mv_attrspec* ptr, char* key, char* value, int rel) {
	mvTypeCode code;

	switch(rel) {
		case MVAST_TYPESPEC:
			ptr->type = TYPE;
			ptr->value.typespec.classname = NULL;
			if (STREQ(value, "string")) {
				code = STRING;
			} else if (STREQ(value, "integer")) {
				code = INTEGER;
			} else {
				code = RAWREF;
				ptr->value.typespec.classname = strdup(value);
			}
			ptr->value.typespec.type = code;
			break;
		default:
			DIE("Unknown relationship code (%d)", rel);
	}
	ptr->name = strdup(key);
}

void mv_attrquery_parse(mv_attrspec* ptr, char* key, mv_ast& value) {
	ptr->type = SUBQUERY;
	ptr->value.subquery.classname = strdup(value[0].value.leaf->ptr);
	mv_attrlist_parse(&ptr->value.subquery.attrs, value[1].value.subtree);
	ptr->name = strdup(key);
}

mv_error* mv_tokenize(mv_strarr* target, const char* data) {
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
		case '[': case ']':
			if (state == WHITESPACE) {
				mv_strarr_appslice(target, data, scan, scan + 1);
			} else if (state == TOKEN) {
				mv_strarr_appslice(target, data, base, scan);
				mv_strarr_appslice(target, data, scan, scan + 1);
				state = WHITESPACE;
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
	target->items = (mv_strref*)realloc(target->items, sizeof(mv_strref) * target->used);
	target->size = target->used;
	return NULL;
} // style:60
