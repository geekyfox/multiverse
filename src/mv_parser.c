
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

inline static void __pair__(mvAstEntry* stack, int* count) {
	if (*count < 3) return;
	mvAstEntry& a = stack[*count - 3];
	if (!LEAF(a)) return;
	mvAstEntry& b = stack[*count - 1];
	mvAstEntry& sep = stack[*count - 2];

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

inline static void __comma__(mvAstEntry* stack, int *count) {
	if (*count < 3) return;
	mvAstEntry& sep = stack[*count - 2];
	if (sep != Comma) return;

	mvAstEntry& a = stack[*count - 3];
	mvAstEntry& b = stack[*count - 1];

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

inline static void __emptylist__(mvAstEntry* stack, int* size) {
	if (*size < 2) return;

	mvAstEntry& a = stack[*size - 2];
	mvAstEntry& b = stack[*size - 1];

	if (a == OpenBrace && b == CloseBrace)
	{
		a = AttrList;
		b.reset();
		(*size)--;
	}
}

inline static void __list__(mvAstEntry* stack, int& count) {
	if (count < 3) return;

	mvAstEntry& e1 = stack[count - 3];
	mvAstEntry& e3 = stack[count - 1];

	if (e1 != OpenBrace || e3 != CloseBrace)
	{
		return;
	}

	mvAstEntry& e2 = stack[count - 2];

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

inline static void __subquery__(mvAstEntry* stack, int* count) {
	if ((*count) < 5) return;

	mvAstEntry& e1 = stack[*count - 5];
	mvAstEntry& e5 = stack[*count - 1];
	if (e1 != OpenBracket || e5 != CloseBracket) return;

	mvAstEntry& e2 = stack[*count - 4];
	if (!LEAF(e2)) return;

	mvAstEntry& e3 = stack[*count - 3];
	if (e3 != "with") return;

	mvAstEntry& e4 = stack[*count - 2];
	if (!LIST(e4)) return;

	e3.clear_leaf();

	e1.set_subtree(MVAST_SUBQUERY, e2, e4);

	e5.reset();

	(*count) -= 4;
}

inline static void __clear__(mvCommand* cmd, mvCommandType code, int vars) {
	cmd->code = code;
	cmd->attrs.clear();
	cmd->spec.clear();
	cmd->vars.clear();
}

mvError* __create_entity__(mvCommand* target, const mvAst& ast) {
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

inline static void __compress__(mvAstEntry* stack, int& size) {
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
throw (mvError*) :
	_type(Command)
{
	mvTokenizer tokens(data);
	mvAstEntry* stack = new mvAstEntry[tokens.size()];
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

	mvError* err = NULL;
	for (i=0; i < size; i++) {
		mvAstEntry& ref = (*this)[i];
		if (ref != Leaf && ref != Subtree && err == NULL)
		{
			err = mvError_unmatched(ref.type(), data);
		}
	}
	if (err != NULL) throw err;
}
mvError* __create__(mvCommand* target, const mvAst& ast) {
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

mvError* __destroy__(mvCommand* target, const mvAst& ast) {
	if ((ast.size() != 3) || (ast[1] != "entity") ||
	    !LEAF(ast[2]))
	{
		THROW(SYNTAX, "Malformed 'destroy' command");
	}
	__clear__(target, DESTROY_ENTITY, 1);
	target->vars.push(ast[2].leaf());
	return NULL;	
} 

inline static mvError* __show__(mvCommand* cmd, const mvAst& ast) {
	assert(ast.size() == 2);
	assert(ast[1] == Leaf);
	__clear__(cmd, SHOW, 1);
	cmd->vars.push(ast[1].leaf());
	return NULL;
}

inline static mvError* __quit__(mvCommand* cmd, const mvAst& ast) {
	if (ast.size() != 1) {
		THROW(SYNTAX, "Malformed 'quit' command");
	}
	__clear__(cmd, QUIT, 0);
	return NULL;
}

inline static mvError* __assign__(mvCommand* cmd, const mvAst& ast) {
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
mvError* __update_entity__(mvCommand* target, const mvAst& ast) {
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
mvError* __update__(mvCommand* target, const mvAst& ast) {
	if ((ast.size() == 1) || ast[1] != Leaf)
	{
		THROW(INTERNAL, "Malformed 'update' command");
	}

	if (ast[1] == "entity") {
		return __update_entity__(target, ast);
	}	

	mvError* err;
	PREPARE_ERROR(err, SYNTAX,
	      "Invalid task for update - '%s'",
		  ast[1].leaf().ptr);
	return err;
}

void mvCommand_parse(mvCommand& command, 
                      const mvStrref& cmdname,
                      const mvAst& ast)
throw (mvError*)
{
	mvCommand* cmd = &command;

	mvError* error;
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

void mv_spec_parse(mvAttrSpec* ptr, const mvStrref& key, const mvStrref& value, mvAstType rel) {
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

void mvAttrquery_parse(mvAttrSpec* ptr, const mvStrref& key, const mvAst& value) {
	value.populate(ptr->subquery_mutable());
	ptr->name = key;
}

mvTokenizer::mvTokenizer(const char* data)
throw (mvError*) : mvStrArray(2)
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
		throw mvError_unmatched(MVAST_TEMPAPOSTROPHE, data);
	}
	if (state == 2) {
		append(data, base, scan);
	}
	pack();
} // style:60
