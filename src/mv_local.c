
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
	case CREATE_ENTITY:
		printf ("OK, entity created\n");
		break;
	case CREATE_CLASS:
		printf ("OK, class created\n");
		break;
	case ASSIGN:
		printf ("OK, class '%s' assigned to '%s'\n",
		        cmd->vars.items[0].ptr,
		        cmd->vars.items[1].ptr);
		break;
	case UPDATE_ENTITY:
		printf ("OK, entity updated\n");
		break;
	}
}

void mv_local_execute(mv_command* cmd) {
	mv_error* error = NULL;
	char *tmpstr = NULL;

	switch(cmd->code) {
	case DO_NOTHING:
		break;
	case ASSIGN:
	case CREATE_CLASS:
	case CREATE_ENTITY:
	case DESTROY_ENTITY:
	case UPDATE_ENTITY:
		error = __LOCAL_SESSION__->execute(cmd);
		if (error == NULL) __display_success(cmd);
		break;
	case SHOW:
		error = mv_session_show(&tmpstr,
                                __LOCAL_SESSION__,
                                cmd->vars.items[0].ptr);
		if (error != NULL) break;
		printf("%s", tmpstr);
		fflush(stdout);
		free(tmpstr);
		break;
	case LOOKUP:
		error = __local_lookup(cmd);
		break;
	default:
		DIE("Unknown command code %d\n", cmd->code);
	}

	if (error != NULL) {
		mv_error_display(error, stderr);
	}
}

void mv_local_end() {
	delete __LOCAL_SESSION__;
}

mv_error* mv_local_read(mvCommand& cmd) {
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
		mv_command_parse(cmd, buffer);
		free(buffer);
		return NULL;
	} catch (mv_error* err) {
		free(buffer);
		return err;
	}
}

