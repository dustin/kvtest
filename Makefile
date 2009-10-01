CFLAGS=-g
LDFLAGS=-g

COMMON=base-test.hh suite.hh tests.hh
OBJS=tests.o suite.o
PROG_OBJS=example-test.o sqlite3-test.o
ALL_OBJS=$(OBJS) $(PROG_OBJS)
ALL_PROGS=example-test sqlite3-test

.PHONY: clean

all: $(ALL_PROGS)

example-test: example-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ example-test.o $(OBJS) $(LDFLAGS) -lsqlite3

sqlite3-test: sqlite3-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ sqlite3-test.o $(OBJS) $(LDFLAGS) -lsqlite3

clean:
	-rm $(ALL_OBJS) $(ALL_PROGS)

.cc.o: $< $(COMMON)
	$(CXX) $(CFLAGS) -c -o $@ $<

$(OBJS): $(COMMON)

