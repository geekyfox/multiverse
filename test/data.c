
#include <string.h>
#include "test.h"

TEST mv_strbuf_test() {
	mv_strbuf buf;
	mv_strbuf_alloc(&buf, 1000);
	mv_strbuf_append(&buf, "foo");
	mv_strbuf_append(&buf, " + bar");
	mv_strbuf_append(&buf, " = baz");
	char* text = mv_strbuf_align(&buf);
	ASSERT_STRING(text, "foo + bar = baz");
	free(text);
}

TEST mv_varbind_test() {
	mv_varbind tmp;
	mv_varbind_alloc(&tmp, 2);
	ASSERT_INT(tmp.size, 2);
	ASSERT_INT(tmp.used, 0);
	int t = mv_varbind_lookup(&tmp, "foo");
	ASSERT_INT(t, -1);
	mv_varbind_insert(&tmp, "foo", 42);
	ASSERT_INT(tmp.used, 1);
	t = mv_varbind_lookup(&tmp, "foo");
	ASSERT_INT(t, 42);
	mv_varbind_insert(&tmp, "bar", 4242);
	ASSERT_INT(tmp.used, 2);
	ASSERT_INT(tmp.size, 4);
	t = mv_varbind_lookup(&tmp, "foo");
	ASSERT_INT(t, 42);
	t = mv_varbind_lookup(&tmp, "bar");
	ASSERT_INT(t, 4242);
	mv_varbind_insert(&tmp, "baz", 4242);
	mv_varbind_insert(&tmp, "quux", 424242);
	ASSERT_INT(tmp.size, 8);
	mv_varbind_insert(&tmp, "foo", 12345);
	ASSERT_INT(tmp.used, 4);
	t = mv_varbind_lookup(&tmp, "foo");
	ASSERT_INT(t, 12345);
	mv_varbind_remove(&tmp, "baz");
	ASSERT_INT(tmp.used, 3);
	t = mv_varbind_lookup(&tmp, "baz");
	ASSERT_INT(t, -1);
	t = mv_varbind_lookup(&tmp, "quux");
	ASSERT_INT(t, 424242);
	mv_varbind_release(&tmp);
}

TEST intset_test() {
	mv_intset set;
	mv_intset_alloc(&set, 1);
	ASSERT_INT(set.size, 1);
	ASSERT_INT(set.used, 0);
	ASSERT_NOTNULL(set.items);

	int ret = mv_intset_contains(&set, 12);
	ASSERT_INT(ret, 0);

	mv_intset_put(&set, 12);
	ASSERT_INT(set.used, 1);
	ret = mv_intset_contains(&set, 12);
	ASSERT_INT(ret, 1);

	mv_intset_put(&set, 6);
	ASSERT_INT(set.used, 2);
	ASSERT_NOLESS(set.size, set.used);

	mv_intset_remove(&set, 6);
	
	ASSERT_INT(set.used, 1);
	mv_intset_release(&set);
}

