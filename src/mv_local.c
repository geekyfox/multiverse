
#include <assert.h>
#include <stdlib.h>
#include "multiverse.h"
#include "parser.h"

mv_session* __LOCAL_SESSION__ = NULL;

void mv_local_start() {
	__LOCAL_SESSION__ = new mvSession();
}

mv_error* __local_lookup(mv_command* c) {
	mvIntset res(1);
	
	mv_error* error = __LOCAL_SESSION__->lookup(res, c);
	if (error == NULL) {
		int size = res.cardinality();
		if (size == 0) {
			printf ("OK, no matching objects found\n");
		} else {
			int i;
			printf ("OK, matching objects found: [%d", res.get(0));
			for (i=size - 1; i>=1; i--) printf(", %d", res.get(i));
			printf ("]\n");
		}
	}
	return error;
}

inline static void __display_success(mv_command* cmd) {
	switch(cmd->code) {
	case MVCMD_CREATE_ENTITY:
		printf ("OK, entity created\n");
		break;
	case MVCMD_CREATE_CLASS:
		printf ("OK, class created\n");
		break;
	case MVCMD_ASSIGN:
		printf ("OK, class '%s' assigned to '%s'\n",
		        cmd->vars.items[0].ptr,
		        cmd->vars.items[1].ptr);
		break;
	case MVCMD_UPDATE_ENTITY:
		printf ("OK, entity updated\n");
		break;
	}
}

void mv_local_execute(mv_command* cmd) {
	mv_error* error = NULL;
	char *tmpstr = NULL;

	switch(cmd->code) {
	case MVCMD_DO_NOTHING:
		break;
	case MVCMD_ASSIGN:
	case MVCMD_CREATE_CLASS:
	case MVCMD_CREATE_ENTITY:
	case MVCMD_DESTROY_ENTITY:
	case MVCMD_UPDATE_ENTITY:
		error = __LOCAL_SESSION__->execute(cmd);
		if (error == NULL) __display_success(cmd);
		break;
	case MVCMD_SHOW:
		error = mv_session_show(&tmpstr,
                                __LOCAL_SESSION__,
                                cmd->vars.items[0].ptr);
		if (error != NULL) break;
		printf("%s", tmpstr);
		fflush(stdout);
		free(tmpstr);
		break;
	case MVCMD_LOOKUP:
		error = __local_lookup(cmd);
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
	delete __LOCAL_SESSION__;
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

	try {
		*cmd = mv_command_parse(buffer);
		free(buffer);
		return NULL;
	} catch (mv_error* err) {
		free(buffer);
		return err;
	}
}

