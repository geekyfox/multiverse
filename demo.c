
#include <stdio.h>
#include "multiverse.h"

int main() {
	mv_local_start();

	while (1) {
		mv_command cmd;
		mv_error* error = mv_local_read(&cmd);
		if (error != NULL) {
			mv_error_display(error, stderr);
			continue;
		}
		if (cmd.code == MVCMD_QUIT) break;
		if (cmd.code == MVCMD_DO_NOTHING) continue;
		mv_local_execute(&cmd);
	}

	mv_local_end();
}