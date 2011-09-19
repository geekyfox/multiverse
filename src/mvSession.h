
#ifndef __MULTIVERSE_SESSION_HEADER__
#define __MULTIVERSE_SESSION_HEADER__

class mvSession {
private:
	mvCodebook vars;
	mvCodebook clsnames;
	//
	int autovalidate;
	mv_error* createImpl(mv_command*);
	mv_error* createNew(int* ref, mv_attrlist attrs);
	mv_error* destroyImpl(mv_command*);
	mv_error* copyAttr(mv_attr* dst, mv_attr* src);
	mv_error* updateEntity(mv_command* cmd);
	void assign(mvCommand& cmd) throw (mv_error*);
public:
	mvEntityCache entities;
	mvClassCache classes;
	mvSession();
	~mvSession();
	int findvar(char* name);
	int findclass(char* name);
	void execute(mvCommand& cmd) throw (mv_error*);
    mv_error* lookup(mvIntset&, mv_command*);
	int varcount() { return vars.cardinality(); }
	int clscount() { return clsnames.cardinality(); }
	void perform(mv_strarr& script) throw (mv_error*);
	char* show(char* name) throw (mv_error*);
};

#endif

