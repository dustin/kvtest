#include <stdio.h>
#include <iostream>

#include "base-test.hh"

using namespace testing;

//
// ----------------------------------------------------------------------
// An individual test.
// ----------------------------------------------------------------------
//
class TestTest : public testing::Test {
public:
    bool run(testing::ThingUnderTest *tut);
    std::string name() { return "test test"; };
};

bool TestTest::run(testing::ThingUnderTest *tut) {
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
