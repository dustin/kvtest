CFLAGS=-Wall -Wextra -ansi -pendantic -Wno-unused-parameter -Werror -g
LDFLAGS=-g

COMMON=base-test.hh locks.hh callbacks.hh suite.hh tests.hh
OBJS=tests.o suite.o
PROG_OBJS=example-test.o sqlite3-test.o sqlite3-async-test.o \
	bdb-test.o bdb-async-test.o
SQLITE_OBJS=sqlite-base.o
SQLITE_COMMON=sqlite-base.hh

BDB_VER=4.8
BDB_PATH=/usr/local/BerkeleyDB.$(BDB_VER)
BDB_CFLAGS=-I$(BDB_PATH)/include
BDB_LDFLAGS=-L$(BDB_PATH)/lib -ldb
BDB_OBJS=bdb-base.o
BDB_COMMON=bdb-base.hh

ALL_OBJS=$(OBJS) $(SQLITE_OBJS) $(PROG_OBJS) $(BDB_OBJS)
ALL_PROGS=example-test sqlite3-test sqlite3-async-test
BDB_PROGS=bdb-test bdb-async-test

.PHONY: clean

all: $(ALL_PROGS)

example-test: example-test.o $(OBJS) $(COMMON)
	$(CXX) -o $@ example-test.o $(OBJS) $(LDFLAGS) -lsqlite3

sqlite3-test: sqlite3-test.o $(SQLITE_OBJS) $(OBJS) $(COMMON)
	$(CXX) -o $@ sqlite3-test.o $(SQLITE_OBJS) $(OBJS) $(LDFLAGS) -lsqlite3

sqlite3-async-test: sqlite3-async-test.o $(SQLITE_OBJS) $(OBJS) $(COMMON)
	$(CXX) -o $@ sqlite3-async-test.o $(SQLITE_OBJS) $(OBJS) $(LDFLAGS) -lsqlite3

bdb-test: bdb-test.o $(BDB_OBJS) $(OBJS) $(COMMON)
	$(CXX) -o $@ bdb-test.o $(BDB_OBJS) $(OBJS) $(LDFLAGS) $(BDB_LDFLAGS)

bdb-async-test: bdb-async-test.o $(BDB_OBJS) $(OBJS) $(COMMON)
	$(CXX) -o $@ bdb-async-test.o $(BDB_OBJS) $(OBJS) $(LDFLAGS) $(BDB_LDFLAGS)

clean:
	-rm $(ALL_OBJS) $(ALL_PROGS) $(BDB_PROGS)

.cc.o: $< $(COMMON)
	$(CXX) $(CFLAGS) -c -o $@ $<

$(OBJS): $(COMMON)
$(PROG_OBJS): $(COMMON)

$(SQLITE_OBJS): $(SQLITE_COMMON) $(COMMON)
sqlite3-async-test.o: async.hh

bdb-base.o: bdb-base.cc $(BDB_COMMON)
	$(CXX) $(CFLAGS) $(BDB_CFLAGS) -c -o $@ bdb-base.cc

bdb-test.o: bdb-test.cc $(BDB_COMMON)
	$(CXX) $(CFLAGS) $(BDB_CFLAGS) -c -o $@ bdb-test.cc

bdb-async-test.o: bdb-test.cc $(BDB_COMMON)
	$(CXX) $(CFLAGS) $(BDB_CFLAGS) -c -o $@ bdb-async-test.cc

