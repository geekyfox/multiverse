
#ifndef __MULTIVERSE_SESSION_HEADER__
#define __MULTIVERSE_SESSION_HEADER__

class mvSession {
private:
	mvCodebook vars;
	mvCodebook clsnames;
	int autovalidate;
	mv_error* createImpl(mv_command*);
	mv_error* createNew(int* ref, mv_attrlist attrs);
	mv_error* destroyImpl(mv_command*);
	mv_error* copyAttr(mv_attr* dst, mv_attr* src);
public:
	mv_entcache entities;
	mv_clscache classes;
	mvSession();
	~mvSession();
	int findvar(char* name);
	int findclass(char* name);
	mv_error* execute(mv_command* cmd);
    mv_error* lookup(mvIntset&, mv_command*);
	int varcount() { return vars.cardinality(); }
	int clscount() { return clsnames.cardinality(); }
};

typedef mvSession mv_session;


#endif

