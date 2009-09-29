#include <stdio.h>
#include <iostream>

#include "base-test.hh"

using namespace kvtest;
using namespace std;

//
// ----------------------------------------------------------------------
// An individual test.
// ----------------------------------------------------------------------
//
class TestTest : public Test {
public:
    bool run(ThingUnderTest *tut);
    string name() { return "test test"; };
};

bool TestTest::run(ThingUnderTest *tut) {
    string testKey("some key");
    string testValue("some value");
    assertTrue(tut->set(testKey, testValue), string("Failed to set value."));
    assertEquals(*(tut->get(testKey)), testValue);
    assertTrue(tut->del(testKey),  string("Failed to delete value."));
    assertFalse(tut->del(testKey), string("Doubly deleted value"));
    assertNull(tut->get(testKey));
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
