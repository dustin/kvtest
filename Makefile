CFLAGS=-O3 -g
LDFLAGS=-g -lsqlite3

COMMON=base-test.hh
OBJS=main.o tests.o

.PHONY: clean

sqlite3-test: $(OBJS) $(COMMON)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

clean:
	-rm $(OBJS)

.cc.o: $< $(COMMON)
	$(CXX) -c -o $@ $(CFLAGS) $<
