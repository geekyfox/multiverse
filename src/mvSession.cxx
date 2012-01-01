
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "multiverse.h"
#include "mvSession.h"
#include "parser.h"
#include "mvParser.h"
#include "mvValidator.h"

mvSession::mvSession() :
	vars(8),
	clsnames(8),
	entities(8),
	classes(8)
{
	autovalidate = 1;
}

mvSession::~mvSession()
{
}

void mvSession::copyAttr(mvAttr* dst, mvAttr* src)
throw (mvError*)
{
	int ref;

	dst->type = src->type;

	switch(src->type) {
	case RAWREF:
		ref = findvar(src->value.rawref);
		if (ref == -1) {
			if (autovalidate) {
				NEWTHROW(BADVAR, "Unknown variable '%s'", src->name);
			} else {
				dst->type = RAWREF;
				dst->value.rawref = strdup(src->value.rawref);	
			}
		} else {
			dst->type = REF;
			dst->value.ref = ref;
		}
		break;
	case REF:
		dst->value.ref = src->value.ref;
		break;
	case INTEGER:
		dst->value.integer = src->value.integer;
		break;
	case STRING:
		dst->value.string = strdup(src->value.string);
		break;
	default:
		DIE("Unknown type");
	}

	dst->name = strdup(src->name);
}

void mvSession::createImpl(mvCommand& cmd)
throw (mvError*)
{
	if (cmd.vars.size() == 0) {
		createNew(cmd.attrs);
	} else if (cmd.vars.size() == 1) {
		const char* varname = cmd.vars[0].ptr;
		int ref = vars.lookup(varname);
		if (ref != -1) {
			NEWTHROW(BADVAR, "Variable already bound");
		}
		ref = createNew(cmd.attrs);
		vars.insert(varname, ref);
	} else {
		NEWTHROW(INTERNAL, "Malformed action");
	}
}

int mvSession::createNew(mvAttrlist& attrs)
throw (mvError*)
{
	int i, j;
	mvEntity* entity = new mvEntity(attrs.size(), 0);
	mvError *error;
	mvAttr *src, *dst;
	//
	for (i=0; i<attrs.size(); i++) {
		src = &attrs[i];
		dst = &entity->data[i];
		try
		{
			copyAttr(dst, src);
		}
		catch (mvError* err)
		{
			delete entity;
			throw error;
		}
	}
	return entities.push(entity);
}

void mvSession::destroyImpl(mvCommand& cmd)
throw (mvError*) {
	EXPECT(cmd.vars.size() == 1, "Command is damaged");
	const char* name = cmd.vars[0].ptr;
	int objref = findvar(name);
	if (objref == -1) {
		NEWTHROW(BADVAR, "Unknown variable '%s'", name);
	}
	entities.drop(objref);
	if (name[0] != '#') vars.remove(name);
} 

int mvSession::findvar(const char* name) {
	if (name[0] == '#' && name[1] == '#') {
		int ref = atoi(name + 2);
		if ((ref < 0) ||
		    (entities.size() <= ref) ||
		    (!entities.exists(ref))) return -1;
		return ref;
	}
	return vars.lookup(name);
}

int mvSession::findclass(const char* name) {
	return clsnames.lookup(name);
}

mvError* mvSession::lookup(mvIntset& ret, mvCommand& cmd) {
	mvQuery query(cmd);

	int i;
	for (i=0; i<entities.size(); i++) {
		if (!entities.exists(i)) continue;
		if (query.match(entities[i])) {
			ret.put(i);
		}
	}

	return NULL;
}

void mvSession::perform(mvStrArray& script)
throw (mvError*)
{
	int i;
	mvCommand cmd;
	for (i=0; i<script.size(); i++) {
		singletonParser.parse(cmd, script[i].ptr);
		execute(cmd);
	}
}

void mvSession::updateEntity(mvCommand& cmd)
throw (mvError*)
{
	EXPECT(cmd.vars.size() == 1, "Command is damaged");
	const char* name = cmd.vars[0].ptr;
	int objref = findvar(name);
	if (objref == -1) {
		NEWTHROW(BADVAR, "Unknown variable '%s'", name);
	}
	entities[objref].update(cmd.attrs);
}

void mvSession::assign(mvCommand& cmd)
throw (mvError*)
{
	assert(cmd.vars.size() == 2);
	mvStrref& clsname = cmd.vars[0];
	const char* objname = cmd.vars[1].ptr;
	int objref = findvar(objname);
	if (objref == -1) {
		NEWTHROW(BADVAR, "Unknown variable '%s'", objname);
	}
	int clsref = findclass(clsname.ptr);
	if (clsref == -1) {
		NEWTHROW(BADVAR, "Unknown class '%s'", clsname.ptr);
	}
	mvEntity& entity = entities[objref];
	mvClass& cls = classes[clsref];
	singletonValidator.validate(entity, cls);
	entity.classes.push(clsname);
}

char* mvSession::show(const char* name)
throw (mvError*)
{
	int ref = findvar(name);
	if (ref != -1) {
		mvStrBuffer buf(1000);
		buf << name << " = " << entities[ref];
		return buf.release();
	}
	ref = findclass(name);
	if (ref != -1) {
		mvStrBuffer buf(1000);
		buf << name << " = " << classes[ref];
		return buf.release();
	}
	NEWTHROW(BADVAR, "Unknown name '%s'", name);
}

