
#include <assert.h>
#include <stdlib.h>
#include "multiverse.h"

mv_session* __LOCAL_SESSION__ = NULL;

void mv_local_start() {
	__LOCAL_SESSION__ = (mv_session*)malloc(sizeof(mv_session));
	mv_session_init(__LOCAL_SESSION__);
}

void mv_local_execute(mv_command* cmd) {
	mv_error* error = NULL;
	char *tmpstr = NULL;

	switch(cmd->code) {
	case MVCMD_DO_NOTHING:
		break;
	case MVCMD_CREATE_ENTITY:
		error = mv_session_execute(__LOCAL_SESSION__, cmd);
		if (error == NULL) printf ("OK, entity created\n");
		break;
	case MVCMD_CREATE_CLASS:
		error = mv_session_execute(__LOCAL_SESSION__, cmd);
		if (error == NULL) printf ("OK, class created\n");
		break;
	case MVCMD_SHOW:
		error = mv_session_show(&tmpstr, __LOCAL_SESSION__, cmd->vars.items[0]);
		if (error != NULL) break;
		printf("%s", tmpstr);
		fflush(stdout);
		free(tmpstr);
		break;
	default:
		DIE("Unknown command code %d\n", cmd->code);
	}

	if (error != NULL) {
		mv_error_display(error, stderr);
	}

	mv_command_release(cmd);
}

void mv_local_end() {
	mv_session_release(__LOCAL_SESSION__);
	free(__LOCAL_SESSION__);
}

mv_error* mv_local_read(mv_command* cmd) {
	char *buffer = (char*)malloc(sizeof(char) * 1000);
	int point = 0, size = 1000;
	char c[10];
	int str = 0, done = 0;
	printf("# ");
	fflush(stdout);
	while (!done) {
		fgets(c, 2, stdin);
		switch (c[0]) {
			case ';':
				if (!str) done = 1;
				break;
			case '\'':
				str = !str;
		}
		if (!done) {
			buffer[point] = c[0];
			point++;
			if (point == size) {
				size += 1000;
				buffer = (char*)realloc(buffer, sizeof(char) * size);
			}
		}
	}
	buffer[point] = '\0';

	mv_error* error = mv_command_parse(cmd, buffer);
	free(buffer);
	return error;
}

