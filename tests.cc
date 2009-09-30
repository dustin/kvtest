#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <sstream>

#include "base-test.hh"
#include "tests.hh"

using namespace kvtest;
using namespace std;

bool alarmed = false;

static void caught_alarm(int which)
{
    alarmed = true;
}

static void setup_alarm(int duration) {
    struct sigaction sig_handler;

    sig_handler.sa_handler = caught_alarm;
    sig_handler.sa_flags = 0;

    sigaction(SIGALRM, &sig_handler, NULL);

    alarmed = false;
    alarm(duration);
}

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
    int i = 0;
    setup_alarm(5);
    for(i = 0 ; !alarmed; i++) {
        std::stringstream kStream;
        std::stringstream vStream;
        kStream << "testKey" << i;
        vStream << "testValue" << i;

        std::string key = kStream.str();
        std::string value = vStream.str();

        assertTrue(tut->set(key, value), "Expected to set something");
    }

    std::cout << "Ran " << i << " operations in 5s ("
              << (i/5) << " ops/s)"
              << std::endl;
    return true;
}
