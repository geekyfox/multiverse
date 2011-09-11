
CXX_MODULES  = Codebook Intset Session MemPool \
               Command Entity
CXX_HEADERS  = $(foreach i,$(CXX_MODULES),src/mv$(i).h)

CORE_HEADERS = src/multiverse.h src/error.h src/model.h src/parser.h \
$(CXX_HEADERS)
TEST_HEADERS = test/test.h

MODULES      = data parser matcher printer session validator
CORE_MODULES = $(MODULES) common local error
TEST_MODULES = tokenize astparse cmdparse $(MODULES) misc
DEMO_MODULES = demo

COPTS = -Wall -g

PERFOPTS = -Wall -O3

CXX_OBJS     = $(foreach i,$(CXX_MODULES),build/mvxx_$(i).o)
CORE_OBJS    = $(foreach i,$(CORE_MODULES),build/mv_$(i).o) $(CXX_OBJS)
OPT_OBJS     = $(foreach i,$(CORE_MODULES),build/op_$(i).o)
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
	g++ $(COPTS) $(CORE_OBJS) $(TEST_OBJS) -o testsuite

.PHONY : stylecheck
stylecheck :
	find src test -name '*.c' | xargs ./stylus.pl | sort -nr

.PHONY : memtest
memtest : testsuite
	valgrind --leak-check=full --show-reachable=yes ./testsuite

demo : $(CORE_OBJS) $(DEMO_OBJS) $(HEADERS) 
	g++ $(COPTS) $(CORE_OBJS) $(DEMO_OBJS) -o demo

build/dm_%.o : src/%.c $(CORE_HEADERS)
	g++ $(COPTS) -c $< -o $@

build/ts_%.o : build/%.c $(CORE_HEADERS) $(TEST_HEADERS)
	g++ $(COPTS) -I src -I test -c $< -o $@

build/suite.c : $(TEST_SRCS)
	./gensuite.pl $@ $(TEST_SRCS)

build/%.c : test/%.c
	./gentest.pl $< $@

build/mv_%.o : src/mv_%.c $(CORE_HEADERS)
	g++ $(COPTS) -c $< -o $@

build/op_%.o : src/mv_%.c $(CORE_HEADERS)
	g++ $(PERFOPTS) -c $< -o $@

build/mvxx_%.o : src/mv%.cxx $(CORE_HEADERS)
	g++ $(COPTS) -c $< -o $@

build/perftest.o : test/perftest.c $(CORE_HEADERS) $(TEST_HEADERS)
	g++ $(PERFOPTS) -I src -I test -c $< -o $@

perftest : $(OPT_OBJS) $(HEADERS) build/perftest.o
	g++ $(PERFOPTS) $(OPT_OBJS) build/perftest.o -o perftest

.PHONY : clean
clean :
	rm -f build/*.o
	rm -f build/*.c
	rm -f testsute
	rm -f demo
	rm -f perftest


