
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"
#include "utils.h"

mv_error* __mv_create_entity(int* ref, mv_session* state, mv_attrlist source) {
	mv_attrlist entity;
	mv_attrlist_alloc(&entity, source.size);
	int i;
	for (i=0; i<source.size; i++) {
		switch(source.attrs[i].type) {
		case MVTYPE_STRING:
			entity.attrs[i].type = MVTYPE_STRING;
			entity.attrs[i].name = strdup(source.attrs[i].name);
			entity.attrs[i].value.string = strdup(source.attrs[i].value.string);
			break;
		default:
			return mv_error_raise(MVERROR_INTERNAL, "Unknown type");
		}
	}
	i = mv_entcache_put(&state->entities, &entity);
	if (ref != NULL) *ref = i;
	return NULL;
}

mv_error* mv_session_execute(mv_session* state, mv_command* action) {
	switch (action->code) {
		case MVCMD_CREATE_ENTITY:
			if (action->vars.used == 0) {
				return __mv_create_entity(NULL, state, action->attrs);
			} else if (action->vars.used == 1) {
				int ref = mv_varbind_lookup(&state->vars, action->vars.items[0]);
				if (ref != -1) {
					return mv_error_raise(MVERROR_BADVAR, "Variable already bound");
				}
				mv_error* error = __mv_create_entity(&ref, state, action->attrs);
				if (error != NULL) return error;
				mv_varbind_insert(&state->vars, action->vars.items[0], ref);
				return NULL;
			} else {
				return mv_error_raise(MVERROR_INTERNAL, "Malformed action");
			}
		default:
			return mv_error_raise(MVERROR_INTERNAL, "Unknown action");
	}
}

void mv_session_init(mv_session* state) {
	mv_varbind_alloc(&state->vars, 8);
	mv_entcache_alloc(&state->entities, 8);
}

int mv_session_findvar(mv_session* session, char* name) {
	if (name[0] == '#' && name[1] == '#') {
		int ref = atoi(name + 2);
		if ((ref < 0) || (session->entities.used <= ref) || (session->entities.items[ref].exist == 0)) return -1;
		return ref;
	}
	return mv_varbind_lookup(&(session->vars), name);
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
	return mv_error_raiseform(MVERROR_BADVAR, "Unknown name '%s'", name);
}

void mv_session_release(mv_session* state) {
	mv_varbind_release(&state->vars);
	mv_entcache_release(&(state->entities));
}

