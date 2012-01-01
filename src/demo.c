
#include <stdio.h>
#include "multiverse.h"

int main() {
	mv_local_start();

	while (1) {
		mvCommand cmd;
		try
		{
			mv_local_read(cmd);
		}
		catch (mvError* error)
		{
			mvError_display(error, stderr);
			continue;
		}
		if (cmd.code == QUIT) break;
		if (cmd.code == DO_NOTHING) continue;
		mv_local_execute(cmd);
	}

	mv_local_end();
}
