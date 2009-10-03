#include <stdio.h>
#include <string.h>
#include <iostream>

#include "base-test.hh"
#include "suite.hh"
#include "tokyo-base.hh"
#include "async.hh"

using namespace std;
using namespace kvtest;

int main(int argc, char **args) {
    TokyoStore tt("/tmp/casket.tch", false);
    QueuedKVStore thing(&tt);

    TestSuite suite(&thing);
    return suite.run() ? 0 : 1;
}
