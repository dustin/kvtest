#include <stdio.h>
#include <iostream>
#include <sstream>

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
    assertNotNull(tut->get(testKey));
    assertEquals(*(tut->get(testKey)), testValue);
    assertTrue(tut->del(testKey),  string("Failed to delete value."));
    assertFalse(tut->del(testKey), string("Doubly deleted value"));
    assertNull(tut->get(testKey));
}

bool WriteTest::run(ThingUnderTest *tut) {
    for(int i = 0; i < 1000000; i++) {
        std::stringstream kStream;
        std::stringstream vStream;
        kStream << "testKey" << i;
        vStream << "testValue" << i;

        std::string key = kStream.str();
        std::string value = vStream.str();

        assertTrue(tut->set(key, value), "Expected to set something");
    }
}
