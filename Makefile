
CORE_HEADERS = src/multiverse.h
TEST_HEADERS = test/test.h

MODULES      = data parser printer
CORE_MODULES = $(MODULES) session common local
TEST_MODULES = $(MODULES) test 
DEMO_MODULES = demo

COPTS = -Wall -g

CORE_OBJS    = $(foreach i,$(CORE_MODULES),build/mv_$(i).o)
DEMO_OBJS    = $(foreach i,$(DEMO_MODULES),build/dm_$(i).o)
TEST_OBJS    = $(foreach i,$(TEST_MODULES),build/ts_$(i).o)

.PHONY : all
all : selftest
#all : demo test

.PHONY : selftest
selftest : testsuite
	./testsuite

testsuite : $(CORE_OBJS) $(TEST_OBJS) $(HEADERS)
	gcc $(CORE_OBJS) $(TEST_OBJS) -o testsuite

.PHONY : memtest
memtest : testsuite
	valgrind --leak-check=full --show-reachable=yes ./testsuite

demo : $(CORE_OBJS) $(DEMO_OBJS) $(HEADERS)
	gcc $(COPTS) $(CORE_OBJS) $(DEMO_OBJS) -o demo

build/dm_%.o : src/%.c $(CORE_HEADERS)
	gcc $(COPTS) -c $< -o $@

build/ts_%.o : test/%.c $(CORE_HEADERS) $(TEST_HEADERS)
	gcc $(COPTS) -c $< -o $@

build/mv_%.o : src/mv_%.c $(CORE_HEADERS)
	gcc $(COPTS) -c $< -o $@

.PHONY : clean
clean :
	rm -f build/*.o
	rm -f testsute
	rm -f demo

