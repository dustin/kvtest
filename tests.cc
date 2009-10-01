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

template <typename T>
class RememberingCallback : public Callback<T> {
public:
    void callback(T &value) {
        val = value;
        fired = true;
    }
    T val;
    bool fired;
};

bool TestTest::run(ThingUnderTest *tut) {
    string testKey("some key");
    string testValue("some value");

    RememberingCallback<bool> cb;
    assertFalse(cb.fired, "Expected callback to not have fired");
    tut->set(testKey, testValue, cb);
    assertTrue(cb.fired, "Expected callback to have fired.");
    assertTrue(cb.val, string("Failed to set value."));

    RememberingCallback<std::string*> getCb;
    assertFalse(getCb.fired, "Expected callback to not have fired");
    tut->get(testKey, getCb);
    assertNotNull(getCb.val);
    assertEquals(*getCb.val, testValue);

    RememberingCallback<bool> delCb1;
    tut->del(testKey, delCb1);
    assertTrue(delCb1.val,  string("Failed to delete value."));

    RememberingCallback<bool> delCb2;
    tut->del(testKey, delCb2);
    assertFalse(delCb2.val, string("Doubly deleted value"));

    RememberingCallback<std::string*> getCb2;
    tut->get(testKey, getCb2);
    assertNull(getCb2.val);
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

        RememberingCallback<bool> cb;
        assertFalse(cb.fired, "Expected callback to not have fired.");
        tut->set(key, value, cb);
        assertTrue(cb.fired, "Expected callback to have fired.");
        assertTrue(cb.val, "Expected to set something");
    }

    std::cout << "Ran " << i << " operations in 5s ("
              << (i/5) << " ops/s)"
              << std::endl;
    return true;
}
