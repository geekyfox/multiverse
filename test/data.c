
#include <string.h>
#include "test.h"

TEST mv_strbuf_test() {
	mv_strbuf buf(1000);
	buf.append("foo");
	buf.append(" + bar");
	buf.append(" = baz");
	char* text = buf.release();
	ASSERT_STRING(text, "foo + bar = baz");
	free(text);
}

TEST mv_varbind_test() {
	mvCodebook tmp(2);
	ASSERT_INT(tmp.cardinality(), 0);
	int t = tmp.lookup("foo");
	ASSERT_INT(t, -1);
	tmp.insert("foo", 42);
	ASSERT_INT(tmp.cardinality(), 1);
	t = tmp.lookup("foo");
	ASSERT_INT(t, 42);
	tmp.insert("bar", 4242);
	ASSERT_INT(tmp.cardinality(), 2);
	t = tmp.lookup("foo");
	ASSERT_INT(t, 42);
	t = tmp.lookup("bar");
	ASSERT_INT(t, 4242);
	tmp.insert("baz", 4242);
	tmp.insert("quux", 424242);
	tmp.insert("foo", 12345);
	ASSERT_INT(tmp.cardinality(), 4);
	t = tmp.lookup("foo");
	ASSERT_INT(t, 12345);
	tmp.remove("baz");
	ASSERT_INT(tmp.cardinality(), 3);
	t = tmp.lookup("baz");
	ASSERT_INT(t, -1);
	t = tmp.lookup("quux");
	ASSERT_INT(t, 424242);
}

TEST intset_test() {
	mvIntset set(1);
	ASSERT_INT(set.cardinality(), 0);

	int ret = set.contains(12);
	ASSERT_INT(ret, 0);

	set.put(12);
	ASSERT_INT(set.cardinality(), 1);
	ret = set.contains(12);
	ASSERT_INT(ret, 1);

	set.put(6);
	ASSERT_INT(set.cardinality(), 2);

	set.remove(6);
	ASSERT_INT(set.cardinality(), 1);
}

