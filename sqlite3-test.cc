#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <queue>

#include "base-test.hh"
#include "suite.hh"
#include "sqlite-base.hh"

using namespace kvtest;

int main(int argc, char **args) {
    Sqlite3 thing("/tmp/test.db");
    TestSuite suite(&thing);
    return suite.run() ? 0 : 1;
}
