
#ifndef __MULTIVERSE_SESSION_HEADER__
#define __MULTIVERSE_SESSION_HEADER__

#include "mvClass.h"

class mvSession {
private:
	mvCodebook vars;
	mvCodebook clsnames;
	//
	int autovalidate;
	void createImpl(mvCommand&) throw (mvError*);
	int createNew(mvAttrlist& attrs) throw (mvError*);
	void destroyImpl(mvCommand&) throw (mvError*);
	void copyAttr(mvAttr* dst, mvAttr* src) throw (mvError*);
	void updateEntity(mvCommand& cmd) throw (mvError*);
	void assign(mvCommand& cmd) throw (mvError*);
public:
	mvEntityCache entities;
	mvClassCache classes;
	mvSession();
	~mvSession();
	int findvar(const char* name);
	int findclass(const char* name);
	void execute(mvCommand& cmd) throw (mvError*);
    mvError* lookup(mvIntset&, mvCommand&);
	int varcount() { return vars.cardinality(); }
	int clscount() { return clsnames.cardinality(); }
	void perform(mvStrArray& script) throw (mvError*);
	char* show(const char* name) throw (mvError*);
};

#endif

