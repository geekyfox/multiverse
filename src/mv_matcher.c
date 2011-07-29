
#include <assert.h>
#include <string.h>
#include "multiverse.h"

mv_error* mv_pattern_compile(mv_pattern* pat, mv_command* cmd) {
	assert(cmd->code == MVCMD_LOOKUP);
	assert(cmd->vars.used == 1);
	pat->clsname = strdup(cmd->vars.items[0]);
	pat->attrs = mv_attrlist_copy(&(cmd->attrs));
	return NULL;
}

int __attrmatch(mv_attr* x, mv_attr* y) {
	if (!STREQ(x->name, y->name)) return 0;
	if (x->type != y->type) return 0;
	switch (x->type) {
	case MVTYPE_STRING:
		return STREQ(x->value.string, y->value.string);
	case MVTYPE_INTEGER:
		return x->value.integer == y->value.integer;
	default:
		DIE("Invalid code (%d)", x->type);
	}
}

int mv_pattern_match(mv_pattern* pat, mv_entity* entity) {
	int i, j, match = 0;
	for (i=0; i<entity->classes.used; i++) {
		if (STREQ(pat->clsname, entity->classes.items[i])) {
			match = 1;
			break;
		}
	}
	if (!match) return 0;

	for (i=0; i<pat->attrs.size; i++) {
		mv_attr* x = &(pat->attrs.attrs[i]);
		match = 0;
		for (j=0; j<entity->data.size; j++) {
			mv_attr* y = &(entity->data.attrs[j]);
			if (__attrmatch(x, y)) {
				match = 1;
				break;
			}
		}
		if (!match) return 0;
	}
	return 1;
}

void mv_pattern_release(mv_pattern* pat) {
	free(pat->clsname);
	mv_attrlist_release(&(pat->attrs));
}

