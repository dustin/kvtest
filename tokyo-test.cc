#include <stdio.h>
#include <string.h>
#include <iostream>

#define HAVE_CXX_STDHEADERS 1

#include "base-test.hh"
#include "suite.hh"
#include "tokyo-base.hh"

using namespace std;
using namespace kvtest;

int main(int argc, char **args) {
    TokyoStore thing("/tmp/casket.tch");

    TestSuite suite(&thing);
    return suite.run() ? 0 : 1;
}
