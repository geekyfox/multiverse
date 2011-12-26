
#include <assert.h>
#include <ctype.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include "parser.h"

#define LIST(var)  (var == AttrList)
#define PAIR(var)  (var == AttrPair)
#define QPAIR(var) (var == AttrQuery)
#define SPEC(var)  (var == TypeSpec)
#define SUBQ(var)  (var == SubQuery)
#define LEAF(var)  (var == Leaf)
#define TEMPFIX(var, code) (var.type_is(MVAST_TEMP##code))

inline static void __pair__(mv_ast_entry* stack, int* count) {
	if (*count < 3) return;
	mv_ast_entry& a = stack[*count - 3];
	if (!LEAF(a)) return;
	mv_ast_entry& b = stack[*count - 1];
	mv_ast_entry& sep = stack[*count - 2];

	mvAstType typecode;
	if (sep == Equals)
	{
		if (LEAF(b)) {
			typecode = MVAST_ATTRPAIR;
		} else if (SUBQ(b)) {
			typecode = MVAST_ATTRQUERY;
		} else {
			return;
		}
	}
	else if (sep == Colon)
	{
		if (LEAF(b)) {
			typecode = MVAST_TYPESPEC;
		} else {
			return;
		}
	} else return;

	sep.reset();

	a.set_subtree(typecode, a, b);
	(*count)-=2;
}

inline static void __comma__(mv_ast_entry* stack, int *count) {
	if (*count < 3) return;
	mv_ast_entry& sep = stack[*count - 2];
	if (sep != Comma) return;

	mv_ast_entry& a = stack[*count - 3];
	mv_ast_entry& b = stack[*count - 1];

	if (a == AttrListTMP && PAIR(b)) {	
		a.subtree_push(b);
		*count -= 2;
		sep.reset();
		return;
	}

	if (PAIR(a) && PAIR(b)) {
		a.set_subtree(MVAST_TEMPATTRLIST, a, b);
		*count -= 2;
		sep.reset();
		return;
	}

	if (SPEC(a) && SPEC(b)) {
		a.set_subtree(MVAST_TEMPATTRSPECLIST, a, b);
		*count -= 2;
		sep.reset();
		return;
	}
} // style:40

inline static void __emptylist__(mv_ast_entry* stack, int* size) {
	if (*size < 2) return;

	mv_ast_entry& a = stack[*size - 2];
	mv_ast_entry& b = stack[*size - 1];

	if (a == OpenBrace && b == CloseBrace)
	{
		a = AttrList;
		b.reset();
		(*size)--;
	}
}

inline static void __list__(mv_ast_entry* stack, int& count) {
	if (count < 3) return;

	mv_ast_entry& e1 = stack[count - 3];
	mv_ast_entry& e3 = stack[count - 1];

	if (e1 != OpenBrace || e3 != CloseBrace)
	{
		return;
	}

	mv_ast_entry& e2 = stack[count - 2];

	if (PAIR(e2) || SPEC(e2) || QPAIR(e2)) {
		if (PAIR(e2)) {
			e1.set_subtree(AttrList, e2);
		} else {
			e1.set_subtree(MVAST_ATTRSPECLIST, e2);
		}
		e3.reset();
		count -= 2;
		return;
	}

	if (e2 == AttrListTMP || e2 == AttrSpecListTMP)
	{
		e2.subtree_fix();
		e1 = e2;
		e3.reset();
		count -= 2;
		return;
	}
}

inline static void __subquery__(mv_ast_entry* stack, int* count) {
	if ((*count) < 5) return;

	mv_ast_entry& e1 = stack[*count - 5];
	mv_ast_entry& e5 = stack[*count - 1];
	if (e1 != OpenBracket || e5 != CloseBracket) return;

	mv_ast_entry& e2 = stack[*count - 4];
	if (!LEAF(e2)) return;

	mv_ast_entry& e3 = stack[*count - 3];
	if (e3 != "with") return;

	mv_ast_entry& e4 = stack[*count - 2];
	if (!LIST(e4)) return;

	e3.clear_leaf();

	e1.set_subtree(MVAST_SUBQUERY, e2, e4);

	e5.reset();

	(*count) -= 4;
}

inline static void __clear__(mv_command* cmd, mvCommandType code, int vars) {
	cmd->code = code;
	cmd->attrs.clear();
	cmd->spec.clear();
	cmd->vars.clear();
}

mv_error* __create_entity__(mv_command* target, const mv_ast& ast) {
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

	ast[2].subtree().populate(target->attrs);
	return NULL;
}

inline static void __compress__(mv_ast_entry* stack, int& size) {
	int oldsize;
	do {
		oldsize = size;
		__emptylist__(stack, &size);
		for (int i=size; i<oldsize; i++)
		{
			assert(stack[i] == Unset);
		}
		__pair__(stack, &size);
		for (int i=size; i<oldsize; i++)
		{
			assert(stack[i] == Unset);
		}
		__comma__(stack, &size);
		for (int i=size; i<oldsize; i++)
		{
			assert(stack[i] == Unset);
		}
		__list__(stack, size);
		for (int i=size; i<oldsize; i++)
		{
			assert(stack[i] == Unset);
		}
		__subquery__(stack, &size);
		for (int i=size; i<oldsize; i++)
		{
			assert(stack[i] == Unset);
		}
	} while (oldsize != size);
}

mvAst::mvAst(const char* data)
throw (mv_error*) :
	_type(Command)
{
	mvTokenizer tokens(data);
	mv_ast_entry* stack = new mv_ast_entry[tokens.size()];
	int i, scan = 0, size = 0;

	while (scan < tokens.size()) {
		stack[size++] = tokens[scan++];
		__compress__(stack, size);
	}
	
	set(stack, size);
	int die = 0;
	for (int i=0; i<tokens.size(); i++)
	{
		if (stack[i] != Unset) die = 1;
	}
	if (die) abort();
	delete[] stack;

	mv_error* err = NULL;
	for (i=0; i < size; i++) {
		mv_ast_entry& ref = (*this)[i];
		if (ref != Leaf && ref != Subtree && err == NULL)
		{
			err = mv_error_unmatched(ref.type(), data);
		}
	}
	if (err != NULL) throw err;
}
mv_error* __create__(mv_command* target, const mv_ast& ast) {
	if ((ast.size() == 1) || !(LEAF(ast[1]))) {
		THROW(INTERNAL, "Malformed 'create' command");
	}

	if (ast[1] == "entity") {
		return __create_entity__(target, ast);
	}

	if (ast[1] == "class") {
		target->code = CREATE_CLASS;
		if (!LEAF(ast[2])) {
			THROW(INTERNAL,
			      "Expected class name, got %d",
			      ast[2].type());
		}
		if (ast[3] != AttrSpecList)
		{
			THROW(INTERNAL,
			      "Expected AttrSpecList, got %d",
			      ast[3].type());
		}
		ast[3].subtree().populate(target->spec);
		target->vars.push(ast[2].leaf());
		target->attrs.clear();
		return NULL;
	}
	
	THROW(SYNTAX,
	      "Invalid task for create - '%s'",
	      &ast[1].leaf());
}

mv_error* __destroy__(mv_command* target, const mv_ast& ast) {
	if ((ast.size() != 3) || (ast[1] != "entity") ||
	    !LEAF(ast[2]))
	{
		THROW(SYNTAX, "Malformed 'destroy' command");
	}
	__clear__(target, DESTROY_ENTITY, 1);
	target->vars.push(ast[2].leaf());
	return NULL;	
} 

inline static mv_error* __show__(mv_command* cmd, const mv_ast& ast) {
	assert(ast.size() == 2);
	assert(ast[1] == Leaf);
	__clear__(cmd, SHOW, 1);
	cmd->vars.push(ast[1].leaf());
	return NULL;
}

inline static mv_error* __quit__(mv_command* cmd, const mv_ast& ast) {
	if (ast.size() != 1) {
		THROW(SYNTAX, "Malformed 'quit' command");
	}
	__clear__(cmd, QUIT, 0);
	return NULL;
}

inline static mv_error* __assign__(mv_command* cmd, const mv_ast& ast) {
	int size = ast.size();
	assert(size == 4);
	assert(ast[1] == Leaf);
	assert(ast[2] == "to");
	assert(ast[3] == Leaf);
	__clear__(cmd, ASSIGN, 2);
	cmd->vars.push(ast[1].leaf());
	cmd->vars.push(ast[3].leaf());
	return NULL;
}

inline static
mv_error* __update_entity__(mv_command* target, const mv_ast& ast) {
	if (ast.size() != 5) goto wrong; 
	if (ast[2] != Leaf) goto wrong;
	if (ast[3] == "with") {
		if (!LIST(ast[4])) goto wrong;
		__clear__(target, UPDATE_ENTITY, 1);
		target->vars.push(ast[2].leaf());
		ast[4].subtree().populate(target->attrs);
		return NULL;
	} 

wrong:
	THROW(SYNTAX, "'update entity' command is malformed");
}

inline static
mv_error* __update__(mv_command* target, const mv_ast& ast) {
	if ((ast.size() == 1) || ast[1] != Leaf)
	{
		THROW(INTERNAL, "Malformed 'update' command");
	}

	if (ast[1] == "entity") {
		return __update_entity__(target, ast);
	}	

	mv_error* err;
	PREPARE_ERROR(err, SYNTAX,
	      "Invalid task for update - '%s'",
		  ast[1].leaf().ptr);
	return err;
}

void mv_command_parse(mvCommand& command, 
                      const mvStrref& cmdname,
                      const mvAst& ast)
throw (mv_error*)
{
	mv_command* cmd = &command;

	mv_error* error;
	if (cmdname == "assign")       error = __assign__(cmd, ast);
	else if (cmdname == "create")  error = __create__(cmd, ast);
	else if (cmdname == "destroy") error = __destroy__(cmd, ast);
	else if (cmdname == "show")    error = __show__(cmd, ast);
	else if (cmdname == "quit")    error = __quit__(cmd, ast);
	else if (cmdname == "update")  error = __update__(cmd, ast);
	else NEWTHROW(BADCMD, cmdname.ptr);

	if (error != NULL) throw error;
	command.init_done();
}

void mv_spec_parse(mv_attrspec* ptr, const mvStrref& key, const mvStrref& value, mvAstType rel) {
	switch(rel) {
		case MVAST_TYPESPEC:
			if (value == "string") {
				ptr->set_typespec(STRING);
			} else if (value == "integer") {
				ptr->set_typespec(INTEGER);
			} else {
				ptr->set_typespec(value);
			}
			break;
		default:
			DIE("Unknown relationship code (%d)", rel);
	}
	ptr->name = key;
}

void mv_attrquery_parse(mv_attrspec* ptr, const mvStrref& key, const mv_ast& value) {
	value.populate(ptr->subquery_mutable());
	ptr->name = key;
}

mvTokenizer::mvTokenizer(const char* data)
throw (mv_error*) : mv_strarr(2)
{
	assert(data != NULL);
	enum { WHITESPACE, LITERAL, TOKEN } state = WHITESPACE;
	int base = 0, scan;
	for (scan = 0; data[scan] != '\0'; scan++) {
		switch (data[scan]) {
		case ' ': case '\t': case '\n':
			if (state == TOKEN) {
				state = WHITESPACE;
				append(data, base, scan);
			}
			break;
		case '\'':
			if (state == WHITESPACE) {
				base = scan;
				state = LITERAL;
			} else if (state == LITERAL) {
				append(data, base, scan);
				state = WHITESPACE;
			} else {
				append(data, base, scan - 1);
				state = LITERAL;
			}
			break;
		case ',': case '{': case '}': case ':':
		case '[': case ']':
			if (state == WHITESPACE) {
				append(data, scan, scan + 1);
			} else if (state == TOKEN) {
				append(data, base, scan);
				append(data, scan, scan + 1);
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
		throw mv_error_unmatched(MVAST_TEMPAPOSTROPHE, data);
	}
	if (state == 2) {
		append(data, base, scan);
	}
	pack();
} // style:60
