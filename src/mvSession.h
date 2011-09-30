
#ifndef __MULTIVERSE_SESSION_HEADER__
#define __MULTIVERSE_SESSION_HEADER__

#include "mvClass.h"

class mvSession {
private:
	mvCodebook vars;
	mvCodebook clsnames;
	//
	int autovalidate;
	void createImpl(mvCommand&) throw (mv_error*);
	int createNew(mv_attrlist& attrs) throw (mv_error*);
	void destroyImpl(mvCommand&) throw (mv_error*);
	void copyAttr(mv_attr* dst, mv_attr* src) throw (mv_error*);
	void updateEntity(mvCommand& cmd) throw (mv_error*);
	void assign(mvCommand& cmd) throw (mv_error*);
public:
	mvEntityCache entities;
	mvClassCache classes;
	mvSession();
	~mvSession();
	int findvar(const char* name);
	int findclass(const char* name);
	void execute(mvCommand& cmd) throw (mv_error*);
    mv_error* lookup(mvIntset&, mvCommand&);
	int varcount() { return vars.cardinality(); }
	int clscount() { return clsnames.cardinality(); }
	void perform(mv_strarr& script) throw (mv_error*);
	char* show(const char* name) throw (mv_error*);
};

#endif

