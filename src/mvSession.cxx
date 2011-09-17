
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "multiverse.h"
#include "mvSession.h"
#include "parser.h"

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

mv_error* mvSession::copyAttr(mv_attr* dst, mv_attr* src) {
	int ref;

	dst->type = src->type;

	switch(src->type) {
	case RAWREF:
		ref = findvar(src->value.rawref);
		if (ref == -1) {
			if (autovalidate) {
				THROW(BADVAR, "Unknown variable '%s'", src->name);
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

	return NULL;
}

mv_error* mvSession::createImpl(mv_command* cmd) {
	if (cmd->vars.used == 0) {
		return createNew(NULL, cmd->attrs);
	} else if (cmd->vars.used == 1) {
		char* varname = cmd->vars.items[0].ptr;
		int ref = vars.lookup(varname);
		if (ref != -1) {
			THROW(BADVAR, "Variable already bound");
		}
		mv_error* error;
		if ((error = createNew(&ref, cmd->attrs))) {
			return error;
		}
		vars.insert(varname, ref);
		return NULL;
	} else {
		THROW(INTERNAL, "Malformed action");
	}
}

mv_error* mvSession::createNew(int* ref, mv_attrlist attrs) {
	int i, j;
	mv_entity* entity = new mv_entity(attrs.size, 0);
	mv_error *error;
	mv_attr *src, *dst;
	//
	for (i=0; i<attrs.size; i++) {
		src = &attrs.attrs[i];
		dst = &entity->data.attrs[i];
		if ((error = copyAttr(dst, src))) {
			for (j=0; j<i; j++) mv_attr_release(&entity->data.attrs[j]);
			return error;
		}
	}
	*ref = entities.push(entity);
	return NULL;
}

mv_error* mvSession::destroyImpl(mv_command* cmd) {
	EXPECT(cmd->vars.used == 1, "Command is damaged");
	char* name = cmd->vars.items[0].ptr;
	int objref = findvar(name);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", name);
	}
	entities.drop(objref);
	if (name[0] != '#') vars.remove(name);
	return NULL;
} 

int mvSession::findvar(char* name) {
	if (name[0] == '#' && name[1] == '#') {
		int ref = atoi(name + 2);
		if ((ref < 0) ||
		    (entities.size() <= ref) ||
		    (!entities.exists(ref))) return -1;
		return ref;
	}
	return vars.lookup(name);
}

int mvSession::findclass(char* name) {
	return clsnames.lookup(name);
}

mv_error* mvSession::lookup(mvIntset& ret, mv_command* cmd) {
	mv_query query;
	mv_error* error = mv_query_compile(&query, cmd);

	if (error != NULL) return error;
	
	int i;
	for (i=0; i<entities.size(); i++) {
		if (!entities.exists(i)) continue;
		if (mv_query_match(&query, &(entities[i]))) {
			ret.put(i);
		}
	}

	mv_query_release(&query);
	return NULL;
}

void mvSession::perform(mv_strarr* script)
throw (mv_error*)
{
	int i;
	mvCommand cmd;
	for (i=0; i<script->used; i++) {
		mv_command_parse(cmd, script->items[i].ptr);
		execute(cmd);
	}
}

mv_error* mvSession::updateEntity(mv_command* cmd)
{
	EXPECT(cmd->vars.used == 1, "Command is damaged");
	char* name = cmd->vars.items[0].ptr;
	int objref = findvar(name);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", name);
	}
	return mv_entity_update(&entities[objref], cmd->attrs);
}

mv_error* mvSession::assign(mv_command* cmd)
{
	assert(cmd->vars.used == 2);
	mv_strref clsname = cmd->vars.items[0];
	char* objname = cmd->vars.items[1].ptr;
	int objref = findvar(objname);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", objname);
	}
	int clsref = findclass(clsname.ptr);
	if (clsref == -1) {
		THROW(BADVAR, "Unknown class '%s'", clsname.ptr);
	}
	mv_entity* entity = &(entities[objref]);
	mv_class* cls = &(classes[clsref]);
	FAILRET(mv_validate_assign(entity, cls));
	mv_strarr_appref(&(entity->classes), &clsname); 
	return NULL;
}

char* mvSession::show(char* name)
throw (mv_error*)
{
	int ref = findvar(name);
	if (ref != -1) {
		mv_strbuf buf(1000);
		buf << name << " = " << entities[ref];
		return buf.release();
	}
	ref = findclass(name);
	if (ref != -1) {
		mv_strbuf buf(1000);
		buf << name << " = ";
		mv_class_show(&buf, &(classes[ref]));
		return buf.release();
	}
	NEWTHROW(BADVAR, "Unknown name '%s'", name);
}

