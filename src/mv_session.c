
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
                            mvSession* sess,
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
	mv_class* c = new mv_class;
	c->data = cls;
	*ref = sess->classes.push(c);
	return NULL;
}

void mvSession::execute(mvCommand& action)
throw (mv_error*) {
	int ref;
	mv_error* error = NULL;
	char* clsname;

	switch (action.code) {
	case ASSIGN:
		error = assign(&action);
		break;
	case CREATE_ENTITY:
		error = createImpl(&action);
		break;
	case CREATE_CLASS:
		if (action.vars.used != 1) {
			NEWTHROW(INTERNAL, "Strange number of variables");
		}
		clsname = action.vars.items[0].ptr;
		ref = clsnames.lookup(clsname);
		if (ref != -1) {
			NEWTHROW(BADVAR, "Class '%s' already defined", clsname);
		}
		if ((error = __mv_create_class(&ref, this, &(action.spec)))) {
			throw error;
		}
		clsnames.insert(clsname, ref);
		return;
	case DESTROY_ENTITY:
		error = destroyImpl(&action);
		break;
	case UPDATE_ENTITY:
		error = updateEntity(&action);
		break;
	default:
		NEWTHROW(INTERNAL, "Unknown action (%d)", action.code);
	}

	if (error != NULL) throw error;
}


