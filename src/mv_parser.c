
#include <assert.h>
#include <ctype.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include "parser.h"

#define LIST(var)  ((var).type_is(MVAST_ATTRLIST))
#define PAIR(var)  ((var).type_is(MVAST_ATTRPAIR))
#define QPAIR(var) ((var).type_is(MVAST_ATTRQUERY))
#define SPEC(var)  ((var).type_is(MVAST_TYPESPEC))
#define SUBQ(var)  ((var).type_is(MVAST_SUBQUERY))
#define LEAF(var)  ((var).is_leaf())
#define LEAFFIX(var, text) (LEAF(var) && STREQ((var).leaf().ptr, text))
#define TEMPFIX(var, code) (var.type_is(MVAST_TEMP##code))

inline static void __pair__(mv_ast_entry* stack, int* count) {
	if (*count < 3) return;
	mv_ast_entry& a = stack[*count - 3];
	if (!LEAF(a)) return;
	mv_ast_entry& b = stack[*count - 1];
	mv_ast_entry& sep = stack[*count - 2];

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

	a.set_subtree(typecode, a, b);
	(*count)-=2;
}

inline static void __comma__(mv_ast_entry* stack, int *count) {
	if (*count < 3) return;
	mv_ast_entry& sep = stack[*count - 2];
	if (! TEMPFIX(sep, COMMA)) return;

	mv_ast_entry& a = stack[*count - 3];
	mv_ast_entry& b = stack[*count - 1];

	if (TEMPFIX(a, ATTRLIST) && PAIR(b)) {	
		a.subtree().push(b);
		*count -= 2;
	}

	if (PAIR(a) && PAIR(b)) {
		a.set_subtree(MVAST_TEMPATTRLIST, a, b);
		*count -= 2;
		return;
	}

	if (SPEC(a) && SPEC(b)) {
		a.set_subtree(MVAST_TEMPATTRSPECLIST, a, b);
		*count -= 2;
		return;
	}
} // style:40

inline static void __emptylist__(mv_ast_entry* stack, int* size) {
	if (*size < 2) return;

	mv_ast_entry& a = stack[*size - 2];
	mv_ast_entry& b = stack[*size - 1];

	if (TEMPFIX(a, OPENBRACE) && TEMPFIX(b, CLOSEBRACE)) {
		a.set_subtree(MVAST_ATTRLIST);
		(*size)--;
	}
}

inline static void __list__(mv_ast_entry* stack, int* count) {
	if ((*count) < 3) return;

	mv_ast_entry& e1 = stack[*count - 3];
	mv_ast_entry& e3 = stack[*count - 1];

	if (!TEMPFIX(e1, OPENBRACE) || !TEMPFIX(e3, CLOSEBRACE)) {
		return;
	}

	mv_ast_entry& e2 = stack[*count - 2];

	if (PAIR(e2) || SPEC(e2) || QPAIR(e2)) {
		if (PAIR(e2)) {
			e1.set_subtree(MVAST_ATTRLIST, e2);
		} else {
			e1.set_subtree(MVAST_ATTRSPECLIST, e2);
		}
		(*count) -= 2;
		return;
	}

	if (TEMPFIX(e2, ATTRLIST) || TEMPFIX(e2, ATTRSPECLIST)) {
		if (TEMPFIX(e2, ATTRLIST)) {
			e1.set(MVAST_ATTRLIST, &e2.subtree());
		} else {
			e1.set(MVAST_ATTRSPECLIST, &e2.subtree());
		}
		(*count) -= 2;
		return;
	}
}

inline static void __subquery__(mv_ast_entry* stack, int* count) {
	if ((*count) < 5) return;

	mv_ast_entry& e1 = stack[*count - 5];
	mv_ast_entry& e5 = stack[*count - 1];
	if (!TEMPFIX(e1, OPENBRACKET) || !TEMPFIX(e5, CLOSEBRACKET)) return;

	mv_ast_entry& e2 = stack[*count - 4];
	if (!LEAF(e2)) return;

	mv_ast_entry& e3 = stack[*count - 3];
	if (!LEAFFIX(e3, "with")) return;

	mv_ast_entry& e4 = stack[*count - 2];
	if (!LIST(e4)) return;

	e3.clear_leaf();

	e1.set_subtree(MVAST_SUBQUERY, e2, e4);

	(*count) -= 4;
}

inline static bool __make_char_token__(mv_ast_entry& e, char token) {
	int code;
	switch (token) {
	case '{': code = MVAST_TEMPOPENBRACE; break;
	case '}': code = MVAST_TEMPCLOSEBRACE; break;
	case ',': code = MVAST_TEMPCOMMA; break;
	case ':': code = MVAST_TEMPCOLON; break;
	case '=': code = MVAST_TEMPEQUALS; break;
	case '[': code = MVAST_TEMPOPENBRACKET; break;
	case ']': code = MVAST_TEMPCLOSEBRACKET; break;
	default:  return false;
	}
	e.set_type(code);
	return true;
}

void mv_attrlist_parse(mv_attrlist* target, mv_ast& source) {
	mv_attrlist_alloc(target, source.size());
	int i;
	for (i=0; i<source.size(); i++) {
		mv_ast_entry srcitem = source[i];
		EXPECT(srcitem.type_is(MVAST_ATTRPAIR), "AttrPair expected");
		mv_ast& astpair = srcitem.subtree();
		EXPECT(astpair.size() == 2, "Two elements expected");
		EXPECT(LEAF(astpair[0]), "Leafs expected");
		EXPECT(LEAF(astpair[1]), "Leafs expected");
		char *key = astpair[0].leaf().ptr;
		char *value = astpair[1].leaf().ptr;
		mv_attr_parse(&target->attrs[i], key, value);
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
	cmd->vars.clear();
}

mv_error* __create_entity__(mv_command* target, mv_ast& ast) {
	if (ast.size() < 3) {
		THROW(SYNTAX, "'create entity' command is incomplete");
	} else if (ast.size() > 4) {
		THROW(SYNTAX, "'create entity' command is malformed");
	}

	if (!LIST(ast[2])) {
		THROW(INTERNAL,
		     "Expected AttrList in 'create entity', got %d",
		     ast[2].type());
	}

	if (ast.size() == 4) {
		if (!LEAF(ast[3])) {
			THROW(INTERNAL,
			      "Expected Leaf for 'create entity' command, got %d",
			      ast[3].type());
		}
		__clear__(target, CREATE_ENTITY, 1);
		target->vars.push(ast[3].leaf());
	} else {
		__clear__(target, CREATE_ENTITY, 0);
	}

	mv_attrlist_parse(&target->attrs, ast[2].subtree());
	return NULL;
}

inline static mv_ast_entry __make_token__(mv_strref& token) {
	mv_ast_entry result;

	if (strlen(token.ptr) == 1) {
		if (__make_char_token__(result, token.ptr[0]))
		{
			return result;
		}
	}
	result.set_leaf(token);
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

mvAst::mvAst(const char* data)
throw (mv_error*) {
	mv_strarr tokens(10);
	FAILTHROW(mv_tokenize(&tokens, data));
	mv_ast_entry* stack = new mv_ast_entry[tokens.size()];
	int i, scan = 0, size = 0;

	while (scan < tokens.size()) {
		stack[size++] = __make_token__(tokens[scan++]);
		__compress__(stack, &size);
	}
	
	set(stack, size);
	delete[] stack;

	for (i=0; i < size; i++) {
		mv_ast_entry& ref = (*this)[i];
		if (ref.is_leaf()) continue;
		if (ref.type() < 0) {
			throw mv_error_unmatched(ref.type(), data);
		}
	}
}
mv_error* __create__(mv_command* target, mv_ast& ast) {
	if ((ast.size() == 1) || !(LEAF(ast[1]))) {
		THROW(INTERNAL, "Malformed 'create' command");
	}

	if (LEAFFIX(ast[1], "entity")) {
		return __create_entity__(target, ast);
	}

	if (LEAFFIX(ast[1], "class")) {
		target->code = CREATE_CLASS;
		if (!LEAF(ast[2])) {
			THROW(INTERNAL,
			      "Expected class name, got %d",
			      ast[2].type());
		}
		if (!ast[3].type_is(MVAST_ATTRSPECLIST)) {
			THROW(INTERNAL,
			      "Expected AttrSpecList, got %d",
			      ast[3].type());
		}
		ast[3].subtree().populate(target->spec);
		target->vars.push(ast[2].leaf());
		target->attrs.size = 0;
		target->attrs.attrs = NULL;
		return NULL;
	}
	
	THROW(SYNTAX,
	      "Invalid task for create - '%s'",
	      ast[1].leaf());
}

mv_error* __destroy__(mv_command* target, mv_ast& ast) {
	if ((ast.size() != 3) || !LEAFFIX(ast[1], "entity") ||
	    !LEAF(ast[2]))
	{
		THROW(SYNTAX, "Malformed 'destroy' command");
	}
	__clear__(target, DESTROY_ENTITY, 1);
	target->vars.push(ast[2].leaf());
	return NULL;	
} 

inline static mv_error* __show__(mv_command* cmd, mv_ast& ast) {
	assert(ast.size() == 2);
	assert(ast[1].is_leaf());
	__clear__(cmd, SHOW, 1);
	cmd->vars.push(ast[1].leaf());
	return NULL;
}

inline static mv_error* __quit__(mv_command* cmd, mv_ast& ast) {
	if (ast.size() != 1) {
		THROW(SYNTAX, "Malformed 'quit' command");
	}
	__clear__(cmd, QUIT, 0);
	return NULL;
}

inline static mv_error* __assign__(mv_command* cmd, mv_ast& ast) {
	int size = ast.size();
	assert(size == 4);
	assert(ast[1].is_leaf());
	assert(LEAFFIX((ast[2]), "to"));
	assert(ast[3].is_leaf());
	__clear__(cmd, ASSIGN, 2);
	cmd->vars.push(ast[1].leaf());
	cmd->vars.push(ast[3].leaf());
	return NULL;
}

inline static mv_error* __lookup__(mv_command* cmd, mv_ast& ast) {
	assert(ast[1].is_leaf());
	if (ast.size() == 2) {
		__clear__(cmd, LOOKUP, 1);
		cmd->vars.push(ast[1].leaf());
		return NULL;
	}
	assert(ast.size() == 4);
	assert(LEAFFIX((ast[2]), "with"));
	assert(ast[3].type_is(MVAST_ATTRLIST));
	__clear__(cmd, LOOKUP, 1);
	cmd->vars.push(ast[1].leaf());
	mv_attrlist_parse(&cmd->attrs, ast[3].subtree());
	return NULL;
}

inline static
mv_error* __update_entity__(mv_command* target, mv_ast& ast) {
	if (ast.size() != 5) goto wrong; 
	if (!ast[2].is_leaf()) goto wrong;
	if (LEAFFIX(ast[3], "with")) {
		if (!LIST(ast[4])) goto wrong;
		__clear__(target, UPDATE_ENTITY, 1);
		target->vars.push(ast[2].leaf());
		mv_attrlist_parse(&target->attrs, ast[4].subtree());
		return NULL;
	} 

wrong:
	THROW(SYNTAX, "'update entity' command is malformed");
}

inline static
mv_error* __update__(mv_command* target, mv_ast& ast) {
	if ((ast.size() == 1) || !(ast[1].is_leaf()))
	{
		THROW(INTERNAL, "Malformed 'update' command");
	}

	if (LEAFFIX(ast[1], "entity")) {
		return __update_entity__(target, ast);
	}	

	mv_error* err;
	PREPARE_ERROR(err, SYNTAX,
	      "Invalid task for update - '%s'",
		  ast[1].leaf().ptr);
	return err;
}

void mv_command_parse(mvCommand& command, const char* data)
throw (mv_error*)
{
	mv_ast ast(data);
	command.destroy();
	mv_command* cmd = &command;
	if (!LEAF((ast[0]))) {
		NEWTHROW(SYNTAX, "Syntax error");
	}
	char* cmdname = ast[0].leaf().ptr;

	mv_error* error;
	if (STREQ(cmdname, "assign"))  error = __assign__(cmd, ast);
	else if (STREQ(cmdname, "create"))  error = __create__(cmd, ast);
	else if (STREQ(cmdname, "destroy")) error = __destroy__(cmd, ast);
	else if (STREQ(cmdname, "lookup"))  error = __lookup__(cmd, ast);
	else if (STREQ(cmdname, "show"))    error = __show__(cmd, ast);
	else if (STREQ(cmdname, "quit"))    error = __quit__(cmd, ast);
	else if (STREQ(cmdname, "update"))  error = __update__(cmd, ast);
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
	ptr->value.subquery.classname = strdup(value[0].leaf().ptr);
	mv_attrlist_parse(&ptr->value.subquery.attrs, value[1].subtree());
	ptr->name = strdup(key);
}

mv_error* mv_tokenize(mv_strarr* target, const char* data) {
	assert(data != NULL);
	assert(target != NULL);
	enum { WHITESPACE, LITERAL, TOKEN } state = WHITESPACE;
	int base = 0, scan;
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
		return mv_error_unmatched(MVAST_TEMPAPOSTROPHE, data);
	}
	if (state == 2) {
		mv_strarr_appslice(target, data, base, scan);
	}
	target->pack();
	return NULL;
} // style:60
