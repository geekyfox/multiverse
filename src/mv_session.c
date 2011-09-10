
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#include "parser.h"

mv_error* __mv_copy_spec(mv_attrspec* dst, mv_attrspec* src) {
	mv_typespec *dtype, *stype;
	mv_query *dquery, *squery;
	dst->type = src->type;

	switch (src->type) {
	case MVSPEC_TYPE:
		dtype = &(dst->value.typespec);
		stype = &(src->value.typespec);
		dtype->type = stype->type;
		dtype->classname = (stype->classname == NULL) ? NULL :
			strdup(stype->classname);
		break;
	case MVSPEC_SUBQUERY:
		dquery = &(dst->value.subquery);
		squery = &(src->value.subquery);
		dquery->classname = strdup(squery->classname);
		dquery->attrs = mv_attrlist_copy(&(squery->attrs));
		break;
	default:
		THROW(INTERNAL, "Unknown attrspec code (%d)", src->type);
	}

	dst->name = strdup(src->name);
	return NULL;
}

mv_error* __mv_create_class(int* ref,
                            mv_session* sess,
                            mv_speclist* specs)
{
	int i, j;
	mv_speclist cls;
	mv_error *error;
	mv_attrspec *src, *dst;
	//
	mv_speclist_alloc(&cls, specs->size);
	for (i=0; i<specs->size; i++) {
		src = &specs->specs[i];
		dst = &cls.specs[i];
		if ((error = __mv_copy_spec(dst, src))) {
			for (j=0; j<i; j++) mv_attrspec_release(&cls.specs[j]);
			return error;
		}
	}
	mv_clscache_put(&sess->classes, ref, &cls);
	return NULL;
}

static mv_error* __assign__(mv_session* state, mv_command* cmd) {
	assert(cmd->vars.used == 2);
	mv_strref clsname = cmd->vars.items[0];
	char* objname = cmd->vars.items[1].ptr;
	int objref = state->findvar(objname);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", objname);
	}
	int clsref = state->findclass(clsname.ptr);
	if (clsref == -1) {
		THROW(BADVAR, "Unknown class '%s'", clsname.ptr);
	}
	mv_entity* entity = &(state->entities.items[objref]);
	mv_class* cls = &(state->classes.items[clsref]);
	FAILRET(mv_validate_assign(entity, cls));
	mv_strarr_appref(&(entity->classes), &clsname); 
	return NULL;
}


inline static mv_error* __update_entity__(mv_session* state, mv_command* cmd) {
	EXPECT(cmd->vars.used == 1, "Command is damaged");
	char* name = cmd->vars.items[0].ptr;
	int objref = state->findvar(name);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", name);
	}
	return mv_entity_update(&state->entities.items[objref], cmd->attrs);
}

mv_error* mvSession::execute(mv_command* action) {
	int ref;
	mv_error* error;
	char* clsname;

	switch (action->code) {
	case ASSIGN:
		return __assign__(this, action);
	case CREATE_ENTITY:
		return createImpl(action);
	case CREATE_CLASS:
		if (action->vars.used != 1) {
			THROW(INTERNAL, "Strange number of variables");
		}
		clsname = action->vars.items[0].ptr;
		ref = clsnames.lookup(clsname);
		if (ref != -1) {
			THROW(BADVAR, "Class '%s' already defined", clsname);
		}
		if ((error = __mv_create_class(&ref, this, &(action->spec)))) {
			return error;
		}
		clsnames.insert(clsname, ref);
		return NULL;
	case DESTROY_ENTITY:
		return destroyImpl(action);
	case UPDATE_ENTITY:
		return __update_entity__(this, action);
	default:
		THROW(INTERNAL, "Unknown action (%d)", action->code);
	}
}


mv_error* mv_session_show(char** target, mv_session* session, char* name) {
	int ref = session->findvar(name);
	if (ref != -1) {
		mv_strbuf buf;
		mv_strbuf_alloc(&buf, 1000);
		mv_strbuf_append(&buf, name);
		mv_strbuf_append(&buf, " = ");
		mv_entity_show(&buf, &(session->entities.items[ref]));
		*target = mv_strbuf_align(&buf);
		return NULL;
	}
	ref = session->findclass(name);
	if (ref != -1) {
		mv_strbuf buf;
		mv_strbuf_alloc(&buf, 1000);
		mv_strbuf_append(&buf, name);
		mv_strbuf_append(&buf, " = ");
		mv_class_show(&buf, &(session->classes.items[ref]));
		*target = mv_strbuf_align(&buf);
		return NULL;
	}
	THROW(BADVAR, "Unknown name '%s'", name);
}

