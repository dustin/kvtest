#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>

#include "base-test.hh"
#include "suite.hh"
#include "sqlite-base.hh"
#include "async.hh"

using namespace kvtest;

int main(int argc, char **args) {
    const char *env_path = getenv("SQLITE_TEST_DB");
    Sqlite3 sq(env_path ? env_path : "/tmp/test.db",
               getenv("KVSTORE_AUDITABLE"));
    QueuedKVStore thing(&sq);

    TestSuite suite(&thing);
    return suite.run() ? 0 : 1;
}
