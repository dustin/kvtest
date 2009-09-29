#include <stdio.h>
#include <iostream>

#include "base-test.hh"

using namespace kvtest;

//
// ----------------------------------------------------------------------
// An individual test.
// ----------------------------------------------------------------------
//
class TestTest : public Test {
public:
    bool run(ThingUnderTest *tut);
    std::string name() { return "test test"; };
};

bool TestTest::run(ThingUnderTest *tut) {
    // actual execution goes here.
}

//
// ----------------------------------------------------------------------
// Build and return a ready test suite.
// ----------------------------------------------------------------------
//
TestSuite::TestSuite(ThingUnderTest *t) {
    tut = t;

    addTest(new TestTest());
}
