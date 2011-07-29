
CORE_HEADERS = src/multiverse.h src/error.h src/consts.h
TEST_HEADERS = test/test.h

MODULES      = data parser matcher printer session
CORE_MODULES = $(MODULES) common local error
TEST_MODULES = $(MODULES) misc cmdparse astparse
DEMO_MODULES = demo

COPTS = -Wall -g

CORE_OBJS    = $(foreach i,$(CORE_MODULES),build/mv_$(i).o)
DEMO_OBJS    = $(foreach i,$(DEMO_MODULES),build/dm_$(i).o)
TEST_SRCS    = $(foreach i,$(TEST_MODULES),build/$i.c)
TEST_OBJS    = $(foreach i,$(TEST_MODULES) suite,build/ts_$(i).o)

.PHONY : all
all : selftest
#all : demo test

.PHONY : selftest
selftest : testsuite
	./testsuite > /dev/null

testsuite : $(CORE_OBJS) $(TEST_OBJS) $(HEADERS)
	gcc $(CORE_OBJS) $(TEST_OBJS) -o testsuite

.PHONY : stylecheck
stylecheck :
	find src test -name '*.c' | xargs ./stylus.pl | sort -nr

.PHONY : memtest
memtest : testsuite
	valgrind --leak-check=full --show-reachable=yes ./testsuite

demo : $(CORE_OBJS) $(DEMO_OBJS) $(HEADERS)
	gcc $(COPTS) $(CORE_OBJS) $(DEMO_OBJS) -o demo

build/dm_%.o : src/%.c $(CORE_HEADERS)
	gcc $(COPTS) -c $< -o $@

build/ts_%.o : build/%.c $(CORE_HEADERS) $(TEST_HEADERS)
	gcc $(COPTS) -I src -I test -c $< -o $@

build/suite.c : $(TEST_SRCS)
	./gensuite.pl $@ $(TEST_SRCS)

build/%.c : test/%.c
	./gentest.pl $< $@

build/mv_%.o : src/mv_%.c $(CORE_HEADERS)
	gcc $(COPTS) -c $< -o $@

.PHONY : clean
clean :
	rm -f build/*.o
	rm -f build/*.c
	rm -f testsute
	rm -f demo

