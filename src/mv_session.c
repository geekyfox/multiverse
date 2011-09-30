
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "multiverse.h"

#include "parser.h"

mv_error* __mv_copy_spec(mv_attrspec* dst, mv_attrspec* src) {
	switch (src->get_type()) {
	case TYPE:
		switch (src->typespec().type)
		{
		case STRING: case INTEGER:
			dst->set_typespec(src->typespec().type); break;
		case RAWREF:
			dst->set_typespec(src->typespec().name()); break;
		default:
			assert(0);
		}
		break;
	case SUBQUERY:
		dst->subquery_mutable().classname = strdup(src->subquery().classname);
		dst->subquery_mutable().attrs.copy_from(src->subquery().attrs);
		break;
	default:
		THROW(INTERNAL, "Unknown attrspec code (%d)", src->get_type());
	}

	dst->name = src->name;
	return NULL;
}

mv_error* __mv_create_class(int* ref,
                            mvSession* sess,
                            mv_speclist& specs)
{
	mv_class* c = new mv_class(specs.size());
	mv_speclist& cls = c->data;
	mv_error *error;
	//
	for (int i=0; i<specs.size(); i++) {
		mv_attrspec* src = &specs[i];
		mv_attrspec* dst = &cls[i];
		if ((error = __mv_copy_spec(dst, src))) {
			delete c;
			return error;
		}
	}
	*ref = sess->classes.push(c);
	return NULL;
}

void mvSession::execute(mvCommand& action)
throw (mv_error*) {
	int ref;
	mv_error* error = NULL;
	const char* clsname;

	switch (action.code) {
	case ASSIGN:
		assign(action);
		return;
	case CREATE_ENTITY:
		createImpl(action);
		return;
	case CREATE_CLASS:
		if (action.vars.size() != 1) {
			NEWTHROW(INTERNAL, "Strange number of variables");
		}
		clsname = action.vars[0].ptr;
		ref = clsnames.lookup(clsname);
		if (ref != -1) {
			NEWTHROW(BADVAR, "Class '%s' already defined", clsname);
		}
		if ((error = __mv_create_class(&ref, this, action.spec))) {
			throw error;
		}
		clsnames.insert(clsname, ref);
		return;
	case DESTROY_ENTITY:
		destroyImpl(action);
		return;
	case UPDATE_ENTITY:
		updateEntity(action);
		return;
	default:
		NEWTHROW(INTERNAL, "Unknown action (%d)", action.code);
	}

	if (error != NULL) throw error;
}


