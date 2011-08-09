
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#include "parser.h"

mv_error* __mv_copy_attr(mv_attr* dst, mv_attr* src, mv_session* sess) {
	int ref;

	dst->type = src->type;

	switch(src->type) {
	case MVTYPE_RAWREF:
		ref = mv_session_findvar(sess, src->value.rawref);
		if (ref == -1) {
			if (sess->autovalidate) {
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

mv_error* __mv_create_entity(int* ref,
                             mv_session* sess,
                             mv_attrlist attrs)
{
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
		if ((error = __mv_copy_attr(dst, src, sess))) {
			for (j=0; j<i; j++) mv_attr_release(&entity.data.attrs[j]);
			return error;
		}
	}
	mv_entcache_put(&sess->entities, ref, &entity);
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
	char* clsname = cmd->vars.items[0];
	char* objname = cmd->vars.items[1];
	int objref = mv_session_findvar(state, objname);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", objname);
	}
	int clsref = mv_session_findclass(state, clsname);
	if (clsref == -1) {
		THROW(BADVAR, "Unknown class '%s'", clsname);
	}
	mv_entity* entity = &(state->entities.items[objref]);
	mv_class* class = &(state->classes.items[clsref]);
	FAILRET(mv_validate_assign(entity, class));
	mv_strarr_append(&(entity->classes), clsname); 
	return NULL;
}

static mv_error* __create_enty_ex(mv_session* state, mv_command* cmd) {
	if (cmd->vars.used == 0) {
		return __mv_create_entity(NULL, state, cmd->attrs);
	} else if (cmd->vars.used == 1) {
		char* varname = cmd->vars.items[0];
		int ref = mv_varbind_lookup(&state->vars, varname);
		if (ref != -1) {
			THROW(BADVAR, "Variable already bound");
		}
		mv_error* error;
		if ((error = __mv_create_entity(&ref, state, cmd->attrs))) {
			return error;
		}
		mv_varbind_insert(&state->vars, varname, ref);
		return NULL;
	} else {
		THROW(INTERNAL, "Malformed action");
	}
}

inline static mv_error* __destroy_enty__(mv_session* state, mv_command* cmd) {
	EXPECT(cmd->vars.used == 1, "Command is damaged");
	char* name = cmd->vars.items[0];
	int objref = mv_session_findvar(state, name);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", name);
	}
	mv_entity_release(&state->entities.items[objref]);
	state->entities.items[objref].exist = 0;
	if (name[0] != '#') {
		mv_varbind_remove(&state->vars, name);
	}
	return NULL;
} 

inline static mv_error* __update_entity__(mv_session* state, mv_command* cmd) {
	EXPECT(cmd->vars.used == 1, "Command is damaged");
	char* name = cmd->vars.items[0];
	int objref = mv_session_findvar(state, name);
	if (objref == -1) {
		THROW(BADVAR, "Unknown variable '%s'", name);
	}
	return mv_entity_update(&state->entities.items[objref], cmd->attrs);
}

mv_error* mv_session_execute(mv_session* state, mv_command* action) {
	mv_error* error;
	char* clsname;

	switch (action->code) {
	case MVCMD_ASSIGN:
		return __assign__(state, action);
	case MVCMD_CREATE_ENTITY:
		return __create_enty_ex(state, action);
	case MVCMD_CREATE_CLASS:
		if (action->vars.used != 1) {
			THROW(INTERNAL, "Strange number of variables");
		}
		clsname = action->vars.items[0];
		int ref = mv_varbind_lookup(&state->clsnames, clsname);
		if (ref != -1) {
			THROW(BADVAR, "Class '%s' already defined", clsname);
		}
		if ((error = __mv_create_class(&ref, state, &(action->spec)))) {
			return error;
		}
		mv_varbind_insert(&state->clsnames, clsname, ref);
		return NULL;
	case MVCMD_DESTROY_ENTITY:
		return __destroy_enty__(state, action);
	case MVCMD_UPDATE_ENTITY:
		return __update_entity__(state, action);
	default:
		THROW(INTERNAL, "Unknown action (%d)", action->code);
	}
}

void mv_session_init(mv_session* state) {
	mv_varbind_alloc(&state->vars, 8);
	mv_varbind_alloc(&state->clsnames, 8);
	mv_entcache_alloc(&state->entities, 8);
	mv_clscache_alloc(&state->classes, 8);
	state->autovalidate = 1;
}

int mv_session_findvar(mv_session* session, char* name) {
	if (name[0] == '#' && name[1] == '#') {
		int ref = atoi(name + 2);
		if ((ref < 0) || (session->entities.used <= ref) || (session->entities.items[ref].exist == 0)) return -1;
		return ref;
	}
	return mv_varbind_lookup(&(session->vars), name);
}

int mv_session_findclass(mv_session* session, char* name) {
	return mv_varbind_lookup(&(session->clsnames), name);
}

mv_error* mv_session_lookup(mv_intset* tr, mv_session* s, mv_command* c) {
	mv_query query;
	mv_error* error = mv_query_compile(&query, c);

	if (error != NULL) return error;
	
	int i;
	for (i=0; i<s->entities.used; i++) {
		if (!s->entities.items[i].exist) continue;
		if (mv_query_match(&query, &(s->entities.items[i]))) {
			mv_intset_put(tr, i);
		}
	}

	mv_query_release(&query);
	return NULL;
}

mv_error* mv_session_perform(mv_session* session, mv_strarr* script) {
	int i;
	mv_command cmd;
	for (i=0; i<script->used; i++) {
		mv_error* error = mv_command_parse(&cmd, script->items[i]);
		if (error != NULL) return error;
		error = mv_session_execute(session, &cmd);
		mv_command_release(&cmd);
		if (error != NULL) return error;
	}
	return NULL;
}

void mv_session_release(mv_session* state) {
	mv_varbind_release(&state->vars);
	mv_varbind_release(&state->clsnames);
	mv_entcache_release(&(state->entities));
	mv_clscache_release(&state->classes);
}

mv_error* mv_session_show(char** target, mv_session* session, char* name) {
	int ref = mv_session_findvar(session, name);
	if (ref != -1) {
		mv_strbuf buf;
		mv_strbuf_alloc(&buf, 1000);
		mv_strbuf_append(&buf, name);
		mv_strbuf_append(&buf, " = ");
		mv_entity_show(&buf, &(session->entities.items[ref]));
		*target = mv_strbuf_align(&buf);
		return NULL;
	}
	ref = mv_session_findclass(session, name);
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

