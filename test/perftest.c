
#include <multiverse.h>
#include "test.h"

static inline void __parse_entry__(char* text) {
	mv_command cmd;
	FAILFAST(mv_command_parse(&cmd, text));
	mv_command_release(&cmd);
}

static inline void __parse_all() {
	__parse_entry__(REQ1);
	__parse_entry__(REQ2);
	__parse_entry__(REQ3);
	__parse_entry__(REQ4);
	__parse_entry__(REQ5);
	__parse_entry__(REQ6);
	__parse_entry__(REQ7);
	__parse_entry__(REQ8);
	__parse_entry__(REQ9);
	__parse_entry__(REQ10);
	__parse_entry__(REQ11);
	__parse_entry__(REQ12);
	__parse_entry__(REQ13);
	__parse_entry__(REQ14);
	__parse_entry__(REQ15);
	__parse_entry__(REQ16);
	__parse_entry__(REQ17);
	__parse_entry__(REQ18);
	__parse_entry__(REQ19);
	__parse_entry__(REQ20);
	__parse_entry__(REQ21);
//	__parse_entry__(REQ22);
	__parse_entry__(REQ23);
	__parse_entry__(REQ24);
}

int main() {
	int i;
	for (i=0; i<100000; i++) __parse_all();
}




