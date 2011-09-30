
#include <string.h>
#include <ctype.h>
#include "mvParser.h"

inline static int __parse_number__(mv_attr& target, const char* value) {
	int ival = 0;
	if (*value == '\0') return 0;
	while (1) {
		if (isdigit(*value)) {
			ival = ival * 10 + (*value - '0');
			value++;
			continue;
		} else if (*value == '\0') {
			target.type = INTEGER;
			target.value.integer = ival;
			return 1;
		} else {
			return 0;
		}
	}
}

void mvParser::parse(mv_attr& target, const char* name, const char* value) {
	target.name = strdup(name);
	if (value[0] == '\'') {
		target.type = STRING;
		target.value.string = strdup(value + 1);
		return;
	}
	if (__parse_number__(target, value)) {
		return;
	}
	target.type = RAWREF;
	target.value.rawref = strdup(value);
}

mvParser singletonParser;

