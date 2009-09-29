CFLAGS=-O3
LDFLAGS=-lsqlite3

COMMON=base-test.hh
OBJS=main.o  tests.o

sqlite3-test: $(OBJS) $(COMMON)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

main.o: main.cc $(COMMON)
tests.o: tests.cc $(COMMON)
