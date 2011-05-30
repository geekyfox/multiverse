
#include "test.h"

void mv_tokenize_REQ1() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, REQ1);
	FAIL(error);
	ASSERT_INT(tokens.size, 8);
	ASSERT_INT(tokens.used, 8);
	ASSERT_NOTNULL(tokens.items);
	char* expect[] = {
		"create", "entity", "{", "name", "=", "'Umberto Eco", "}",
        "umberto_eco"
	};
	int i;
	for (i=0; i<8; i++) ASSERT_STRING(tokens.items[i], expect[i]);
	mv_strarr_release(&tokens);
}

void mv_tokenize_REQ2() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, REQ2);
	FAIL(error);
	ASSERT_INT(tokens.size, 12);
	ASSERT_INT(tokens.used, 12);
	ASSERT_NOTNULL(tokens.items);
	char* expect[] = {
		"create", "entity", "{", "country", "=", "italy", ",", "name",
        "=", "'Umberto Eco", "}", "umberto_eco"
	};
	int i;
	for (i=0; i<12; i++) ASSERT_STRING(tokens.items[i], expect[i]);
	mv_strarr_release(&tokens);
}

void mv_tokenize_REQ7() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, REQ7);
	FAIL(error);
	ASSERT_INT(tokens.size, 8);
	ASSERT_INT(tokens.used, 8);
	ASSERT_NOTNULL(tokens.items);
	char* expect[] = {
		"create", "class", "person", "{", "name", ":", "string", "}"
	};
	int i;
	for (i=0; i<8; i++) ASSERT_STRING(tokens.items[i], expect[i]);
	mv_strarr_release(&tokens);
}

void mv_tokenize_fails() {
	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, BADREQ1);
	ASSERT_NOTNULL(error);
	ASSERT_INT(error->code, MVERROR_SYNTAX);
	mv_error_release(error);
}

void mv_tokenize_test() {
	mv_tokenize_REQ1();
	mv_tokenize_REQ2();
	mv_tokenize_REQ7();
	mv_tokenize_fails();

	mv_strarr tokens;
	mv_error* error = mv_tokenize(&tokens, "\nquit");
	FAIL(error);
	ASSERT_INT(tokens.used, 1);
	mv_strarr_release(&tokens);

	printf("mv_tokenize_test PASSED\n");
}

void perform_parser_test() {
	mv_tokenize_test();
}

