CFLAGS=-g
LDFLAGS=-g

COMMON=base-test.hh locks.hh callbacks.hh suite.hh tests.hh
OBJS=tests.o suite.o
PROG_OBJS=example-test.o sqlite3-test.o sqlite3-async-test.o
SQLITE_OBJS=sqlite-base.o
SQLITE_COMMON=sqlite-base.hh

BDB_VER=4.8
BDB_CFLAGS=-I/usr/local/BerkeleyDB.$(BDB_VER)/include
BDB_LDFLAGS=-L/usr/local/BerkeleyDB.$(BDB_VER)/lib -ldb-$(BDB_VER)

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

bdb-test: bdb-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ bdb-test.o $(OBJS) $(LDFLAGS) $(BDB_LDFLAGS)

clean:
	-rm $(ALL_OBJS) $(ALL_PROGS)

.cc.o: $< $(COMMON)
	$(CXX) $(CFLAGS) -c -o $@ $<

$(OBJS): $(COMMON)
$(PROG_OBJS): $(COMMON)

$(SQLITE_OBJS): $(SQLITE_COMMON) $(COMMON)
sqlite3-async-test.o: async.hh

bdb-test.o: bdb-test.cc
	$(CXX) $(CFLAGS) $(BDB_CFLAGS) -c -o $@ $<
