
#ifndef __MULTIVERSE_VALIDATOR_HEADER__
#define __MULTIVERSE_VALIDATOR_HEADER__

class mvValidator {
public:
	void validate(const mvEntity& enty,
	              const mvClass& cls) throw (mvError*);
};

extern mvValidator singletonValidator;

#endif

