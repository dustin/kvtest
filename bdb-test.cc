#include <stdio.h>
#include <string.h>
#include <iostream>

#define HAVE_CXX_STDHEADERS 1
#include <db.h>

#include "base-test.hh"
#include "suite.hh"
#include "bdb-base.hh"

using namespace std;
using namespace kvtest;

int main(int argc, char **args) {
    BDBStore thing("/tmp/test.bdb");

    TestSuite suite(&thing);
    return suite.run() ? 0 : 1;
}
