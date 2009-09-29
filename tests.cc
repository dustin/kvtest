#include <stdio.h>
#include <iostream>

#include "base-test.hh"
#include "tests.hh"

using namespace kvtest;
using namespace std;

//
// ----------------------------------------------------------------------
// An sample test.
// ----------------------------------------------------------------------
//

bool TestTest::run(ThingUnderTest *tut) {
    string testKey("some key");
    string testValue("some value");
    assertTrue(tut->set(testKey, testValue), string("Failed to set value."));
    assertEquals(*(tut->get(testKey)), testValue);
    assertTrue(tut->del(testKey),  string("Failed to delete value."));
    assertFalse(tut->del(testKey), string("Doubly deleted value"));
    assertNull(tut->get(testKey));
}
