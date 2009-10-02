CFLAGS=-g
LDFLAGS=-g

COMMON=base-test.hh locks.hh callbacks.hh suite.hh tests.hh
OBJS=tests.o suite.o
PROG_OBJS=example-test.o sqlite3-test.o sqlite3-async-test.o
SQLITE_OBJS=sqlite-base.o
SQLITE_COMMON=sqlite-base.hh
ALL_OBJS=$(OBJS) $(SQLITE_OBJS) $(PROG_OBJS)
ALL_PROGS=example-test sqlite3-test sqlite3-async-test

.PHONY: clean

all: $(ALL_PROGS)

example-test: example-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ example-test.o $(OBJS) $(LDFLAGS) -lsqlite3

sqlite3-test: sqlite3-test.o $(SQLITE_OBJS) $(OBJS) $(COMMON)
	$(CXX) -o $@ sqlite3-test.o $(SQLITE_OBJS) $(OBJS) $(LDFLAGS) -lsqlite3

sqlite3-async-test: sqlite3-async-test.o $(SQLITE_OBJS) $(OBJS) $(COMMON)
	$(CXX) -o $@ sqlite3-async-test.o $(SQLITE_OBJS) $(OBJS) $(LDFLAGS) -lsqlite3

clean:
	-rm $(ALL_OBJS) $(ALL_PROGS)

.cc.o: $< $(COMMON)
	$(CXX) $(CFLAGS) -c -o $@ $<

$(OBJS): $(COMMON)
$(PROG_OBJS): $(COMMON)

$(SQLITE_OBJS): $(SQLITE_COMMON)

