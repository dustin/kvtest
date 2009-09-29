CFLAGS=-O3
LDFLAGS=-lsqlite3

COMMON=base-test.hh
OBJS=main.o tests.o

.PHONY: clean

sqlite3-test: $(OBJS) $(COMMON)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

clean:
	-rm $(OBJS)

main.o: main.cc $(COMMON)
tests.o: tests.cc $(COMMON)
