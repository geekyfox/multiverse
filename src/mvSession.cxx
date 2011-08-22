
#include <stdio.h>
#include <string.h>
#include "multiverse.h"
#include "mvSession.h"

mvSession::mvSession() {
	mv_varbind_alloc(&vars, 8);
	mv_varbind_alloc(&clsnames, 8);
	mv_entcache_alloc(&entities, 8);
	mv_clscache_alloc(&classes, 8);
	autovalidate = 1;
}

mvSession::~mvSession() {
	mv_varbind_release(&vars);
	mv_varbind_release(&clsnames);
	mv_entcache_release(&entities);
	mv_clscache_release(&classes);
}

mv_error* mvSession::copyAttr(mv_attr* dst, mv_attr* src) {
	int ref;

	dst->type = src->type;

	switch(src->type) {
	case MVTYPE_RAWREF:
		ref = findvar(src->value.rawref);
		if (ref == -1) {
			if (autovalidate) {
				THROW(BADVAR, "Unknown variable '%s'", src->name);
			} else {
				dst->type = MVTYPE_RAWREF;
				dst->value.rawref = strdup(src->value.rawref);	
			}
		} else {
			dst->type = MVTYPE_REF;
			dst->value.ref = ref;
		}
		break;
	case MVTYPE_REF:
		dst->value.ref = src->value.ref;
		break;
	case MVTYPE_INTEGER:
		dst->value.integer = src->value.integer;
		break;
	case MVTYPE_STRING:
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
		int ref = mv_varbind_lookup(&vars, varname);
		if (ref != -1) {
			THROW(BADVAR, "Variable already bound");
		}
		mv_error* error;
		if ((error = createNew(&ref, cmd->attrs))) {
			return error;
		}
		mv_varbind_insert(&vars, varname, ref);
		return NULL;
	} else {
		THROW(INTERNAL, "Malformed action");
	}
}

mv_error* mvSession::createNew(int* ref, mv_attrlist attrs) {
	int i, j;
	mv_entity entity;
	mv_error *error;
	mv_attr *src, *dst;
	//
	mv_entity_alloc(&entity, attrs.size, 0);
	entity.exist = 1;
	for (i=0; i<attrs.size; i++) {
		src = &attrs.attrs[i];
		dst = &entity.data.attrs[i];
		if ((error = copyAttr(dst, src))) {
			for (j=0; j<i; j++) mv_attr_release(&entity.data.attrs[j]);
			return error;
		}
	}
	mv_entcache_put(&entities, ref, &entity);
	return NULL;
}

mv_error* mvSession::destroyImpl(mv_command* cmd) {
	EXPECT(cmd->vars.used == 1, "Command is damaged");
	char* name = cmd->vars.items[0].ptr;
	int objref = findvar(name);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", name);
	}
	mv_entity_release(&entities.items[objref]);
	entities.items[objref].exist = 0;
	if (name[0] != '#') {
		mv_varbind_remove(&vars, name);
	}
	return NULL;
} 

int mvSession::findvar(char* name) {
	if (name[0] == '#' && name[1] == '#') {
		int ref = atoi(name + 2);
		if ((ref < 0) ||
		    (entities.used <= ref) ||
		    (entities.items[ref].exist == 0)) return -1;
		return ref;
	}
	return mv_varbind_lookup(&vars, name);
}

int mvSession::findclass(char* name) {
	return mv_varbind_lookup(&clsnames, name);
}

mv_error* mvSession::lookup(mvIntset& ret, mv_command* cmd) {
	mv_query query;
	mv_error* error = mv_query_compile(&query, cmd);

	if (error != NULL) return error;
	
	int i;
	for (i=0; i<entities.used; i++) {
		if (!entities.items[i].exist) continue;
		if (mv_query_match(&query, &(entities.items[i]))) {
			ret.put(i);
		}
	}

	mv_query_release(&query);
	return NULL;
}

