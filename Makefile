CFLAGS=-O3 -g
LDFLAGS=-g

COMMON=base-test.hh tests.hh
OBJS=tests.o suite.o
PROG_OBJS=example-test.o sqlite3-test.o
ALL_OBJS=$(OBJS) $(PROG_OBJS)

.PHONY: clean

all: example-test sqlite3-test

example-test: example-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ example-test.o $(OBJS) $(LDFLAGS) -lsqlite3

sqlite3-test: sqlite3-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ sqlite3-test.o $(OBJS) $(LDFLAGS) -lsqlite3

clean:
	-rm $(ALL_OBJS)

.cc.o: $< $(COMMON)
	$(CXX) $(CFLAGS) -c -o $@ $<
