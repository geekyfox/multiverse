
#include <assert.h>
#include <ctype.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include <ctype.h>
#include "mvParser.h"

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

inline static int __parse_number__(mvAttr& target, const char* value) {
	int ival = 0;
	if (*value == '\0') return 0;
	while (1) {
		if (isdigit(*value)) {
			ival = ival * 10 + (*value - '0');
			value++;
			continue;
		} else if (*value == '\0') {
			target.type = INTEGER;
			target.value.integer = ival;
			return 1;
		} else {
			return 0;
		}
	}
}

void mvParser::parse(mvAttr& target, const char* name, const char* value) {
	target.name = strdup(name);
	if (value[0] == '\'') {
		target.type = STRING;
		target.value.string = strdup(value + 1);
		return;
	}
	if (__parse_number__(target, value)) {
		return;
	}
	target.type = RAWREF;
	target.value.rawref = strdup(value);
}

void mvParser::parse(mvCommand& target, const char* data)
throw (mvError*)
{
	mvAst ast(data);
	target.destroy();
	if (ast.size() == 0 || ast[0] != Leaf)
	{
		NEWTHROW(SYNTAX, "Syntax error");
	}
	mvStrref& cmdname = ast[0].leaf();

	if (cmdname == "assign")
	{
		_parse_assign(target, ast);
		return;
	}
	if (cmdname == "create")
	{
		_parse_create(target, ast);
		return;
	}
	if (cmdname == "destroy")
	{
		_parse_destroy(target, ast);
		return;
	}
	if (cmdname == "lookup")
	{
		_parse_lookup(target, ast);
		return;
	}
	if (cmdname == "quit")
	{
		_parse_quit(target, ast);
		return;
	}
	if (cmdname == "show")
	{
		_parse_show(target, ast);
		return;
	}
	if (cmdname == "update")
	{
		_parse_update(target, ast);
		return;
	}
	NEWTHROW(BADCMD, "%s", cmdname.ptr);
}

void mvParser::_parse_assign(mvCommand& cmd, const mvAst& ast)
throw (mvError*)
{
	int size = ast.size();
	if ((size != 4)      ||
	    (ast[1] != Leaf) ||
	    (ast[2] != "to") ||
	    (ast[3] != Leaf))
	{
		NEWTHROW(SYNTAX, "Syntax error");
	}
	cmd = ASSIGN;
	cmd.vars.push(ast[1].leaf());
	cmd.vars.push(ast[3].leaf());
}

void mvParser::_parse_create(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if ((ast.size() == 1) || (ast[1] != Leaf)) {
		NEWTHROW(INTERNAL, "Malformed 'create' command");
	}

	if (ast[1] == "entity") {
		_parse_create_entity(target, ast);
		return;
	}

	if (ast[1] == "class") {
		target = CREATE_CLASS;
		if (ast[2] != Leaf)
		{
			NEWTHROW(INTERNAL,
			      "Expected class name, got %d",
			      ast[2].type());
		}
		if (ast[3] != AttrSpecList)
		{
			NEWTHROW(INTERNAL,
			      "Expected AttrSpecList, got %d",
			      ast[3].type());
		}
		ast[3].subtree().populate(target.spec);
		target.vars.push(ast[2].leaf());
		target.attrs.clear();
		target.init_done();
		return;
	}
	
	NEWTHROW(SYNTAX,
	      "Invalid task for create - '%s'",
	      ast[1].leaf().ptr);
}

void mvParser::_parse_create_entity(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if (ast.size() < 3) {
		NEWTHROW(SYNTAX, "'create entity' command is incomplete");
	} else if (ast.size() > 4) {
		NEWTHROW(SYNTAX, "'create entity' command is malformed");
	}

	if (ast[2] != AttrList) {
		NEWTHROW(INTERNAL,
		     "Expected AttrList in 'create entity', got %d",
		     ast[2].type());
	}

	if (ast.size() == 4) {
		if (ast[3] != Leaf) {
			NEWTHROW(INTERNAL,
			      "Expected Leaf for 'create entity' command, got %d",
			      ast[3].type());
		}
		target = CREATE_ENTITY;
		target.vars.push(ast[3].leaf());
	} else {
		target = CREATE_ENTITY;
	}

	ast[2].subtree().populate(target.attrs);
	target.init_done();
}

void mvParser::_parse_destroy(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if ((ast.size() != 3) || (ast[1] != "entity") ||
	    (ast[2] != Leaf))
	{
		NEWTHROW(SYNTAX, "Malformed 'destroy' command");
	}
	target = DESTROY_ENTITY;
	target.vars.push(ast[2].leaf());
}

void mvParser::_parse_lookup(mvCommand& cmd, const mvAst& ast)
throw (mvError*)
{
	if (ast[1] != Leaf)
	{
		NEWTHROW(SYNTAX, "Syntax error");
	}
	bool set_class = (ast[1] != "entity");
	bool set_attrs = (ast.size() != 2);
	if (set_attrs)
	{
		if (ast[2] != "with" || ast[3] != AttrList)
		{
			NEWTHROW(SYNTAX, "Syntax error");
		}
	}
	cmd = LOOKUP;
	if (set_class)
	{
		cmd.vars.push(ast[1].leaf());
	}
	if (set_attrs)
	{
		ast[3].subtree().populate(cmd.attrs);
	}
}

void mvParser::_parse_quit(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if (ast.size() != 1) {
		NEWTHROW(SYNTAX, "Malformed 'quit' command");
	}
	target = QUIT;
}

void mvParser::_parse_show(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if (ast.size() != 2 || ast[1] != Leaf)
	{
		NEWTHROW(SYNTAX, "Malformed 'show' command");
	}
	target = SHOW;
	target.vars.push(ast[1].leaf());
}

void mvParser::_parse_update(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if ((ast.size() == 1) || ast[1] != Leaf)
	{
		NEWTHROW(INTERNAL, "Malformed 'update' command");
	}

	if (ast[1] == "entity") {
		_parse_update_entity(target, ast);
		return;
	}	

	NEWTHROW(SYNTAX, "Invalid task for update - '%s'",
		             ast[1].leaf().ptr);
}

void mvParser::_parse_update_entity(mvCommand& target, const mvAst& ast)
throw (mvError*)
{
	if ((ast.size() == 5)  &&
	    (ast[2] == Leaf)   &&
	    (ast[3] == "with") &&
	    (ast[4] == AttrList))
	{
		target = UPDATE_ENTITY;
		target.vars.push(ast[2].leaf());
		ast[4].subtree().populate(target.attrs);
		return;
	}

	NEWTHROW(SYNTAX, "'update entity' command is malformed");
}

void mvParser::parse(mvAttrSpec& spec,
                     const mvStrref& key,
                     const mvStrref& value,
                     mvAstType rel)
{
	switch(rel) {
		case TypeSpec:
			if (value == "string") {
				spec.set_typespec(STRING);
			} else if (value == "integer") {
				spec.set_typespec(INTEGER);
			} else {
				spec.set_typespec(value);
			}
			break;
		default:
			DIE("Unknown relationship code (%d)", rel);
	}
	spec.name = key;
}

mvParser singletonParser;



void mvAstStack::_pair_rule()
{
	if (last < 3) return;
	mvAstEntry& fst = get(-3);
	if (fst != Leaf) return;
	mvAstEntry& snd = get(-1);
	mvAstEntry& sep = get(-2);

	mvAstType typecode;
	if (sep == Equals)
	{
		if (snd == Leaf)
		{
			typecode = AttrPair;
		}
		else if (snd == SubQuery)
		{
			typecode = AttrQuery;
		}
		else return;
	}
	else if (sep == Colon)
	{
		if (snd == Leaf)
		{
			typecode = TypeSpec;
		}
		else return;
	}
	else return;

	fst.set_subtree(typecode, fst, snd);
	purge(2);
}

void mvAstStack::_comma_rule()
{
	if (last < 3) return;
	mvAstEntry& sep = get(-2);
	if (sep != Comma) return;

	mvAstEntry& a = get(-3);
	mvAstEntry& b = get(-1);

	if (a == AttrListTMP && b == AttrPair) {	
		a.subtree_push(b);
		purge(2);
		return;
	}

	if (a == AttrPair && b == AttrPair) {
		a.set_subtree(AttrListTMP, a, b);
		purge(2);
		return;
	}

	if (a == TypeSpec && b == TypeSpec) {
		a.set_subtree(AttrSpecListTMP, a, b);
		purge(2);
		return;
	}
} // style:40

void mvAstStack::_emptylist_rule()
{
	if (last >= 2 && get(-2) == OpenBrace && get(-1) == CloseBrace)
	{
		get(-2) = AttrList;
		purge(1);
	}
}

void mvAstStack::_list_rule()
{
	if (last < 3) return;

	mvAstEntry& e1 = get(-3);
	mvAstEntry& e3 = get(-1);

	if (e1 != OpenBrace || e3 != CloseBrace)
	{
		return;
	}

	mvAstEntry& e2 = get(-2);

	if (e2 == AttrPair || e2 == TypeSpec || e2 == AttrQuery) {
		if (e2 == AttrPair) {
			e1.set_subtree(AttrList, e2);
		} else {
			e1.set_subtree(AttrSpecList, e2);
		}
		purge(2);
		return;
	}

	if (e2 == AttrListTMP || e2 == AttrSpecListTMP)
	{
		e2.subtree_fix();
		e1 = e2;
		purge(2);
		return;
	}
}

void mvAstStack::_subquery_rule()
{
	if (last < 5) return;

	mvAstEntry& e1 = get(-5);
	mvAstEntry& e5 = get(-1);
	if (e1 != OpenBracket || e5 != CloseBracket) return;

	mvAstEntry& e2 = get(-4);
	if (e2 != Leaf) return;

	mvAstEntry& e3 = get(-3);
	if (e3 != "with") return;

	mvAstEntry& e4 = get(-2);
	if (e4 != AttrList) return;

	e3.clear_leaf();

	e1.set_subtree(SubQuery, e2, e4);

	e5.reset();

	purge(4);
}

void mvAstStack::_compress()
{
	mvAstStack& stack = (*this);
	int oldsize;
	do {
		oldsize = stack.last;
		_emptylist_rule();
		_pair_rule();
		_comma_rule();
		_list_rule();
		_subquery_rule();
	} while (oldsize != stack.last);
}

void mvAstStack::add(mvStrref& token)
{
	data[last++] = token;
	_compress();
}

