#include <stdio.h>
#include <signal.h>
#include <pthread.h>
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

    RememberingCallback<bool> cb;
    assertFalse(cb.fired, "Expected callback to not have fired");
    tut->set(testKey, testValue, cb);
    cb.waitForValue();
    assertTrue(cb.fired, "Expected callback to have fired.");
    assertTrue(cb.val, string("Failed to set value."));

    RememberingCallback<GetValue> getCb;
    assertFalse(getCb.fired, "Expected callback to not have fired");
    tut->get(testKey, getCb);
    getCb.waitForValue();
    assertTrue(getCb.val.success, "Expected success getting initial value");
    assertEquals(getCb.val.value, testValue);

    RememberingCallback<bool> delCb1;
    tut->del(testKey, delCb1);
    delCb1.waitForValue();
    assertTrue(delCb1.val,  string("Failed to delete value."));

    RememberingCallback<bool> delCb2;
    tut->del(testKey, delCb2);
    delCb2.waitForValue();
    assertFalse(delCb2.val, string("Doubly deleted value"));

    RememberingCallback<GetValue> getCb2;
    tut->get(testKey, getCb2);
    getCb2.waitForValue();
    assertFalse(getCb2.val.success, "Expected failure getting final value.");
}

class CountingCallback : public kvtest::Callback<bool> {
public:
    int x;

    CountingCallback() {
        x = 0;
        if(pthread_mutex_init(&mutex, NULL) != 0) {
            throw std::runtime_error("Failed to create mutex.");
        }
    }

    ~CountingCallback() {
        pthread_mutex_destroy(&mutex);
    }

    void callback(bool &val) {
        LockHolder lh(&mutex);
        x++;
    }
private:
    pthread_mutex_t mutex;
};

bool WriteTest::run(ThingUnderTest *tut) {
    int i = 0;
    setup_alarm(5);
    CountingCallback cb;
    time_t start = time(NULL);

    for(i = 0 ; !alarmed; i++) {
        std::stringstream kStream;
        std::stringstream vStream;
        kStream << "testKey" << i;
        vStream << "testValue" << i;

        std::string key = kStream.str();
        std::string value = vStream.str();

        tut->set(key, value, cb);
    }

    // Last one to wait for a commit.
    RememberingCallback<bool> cbLast;
    std::string aKey("nonexistent");
    tut->del(aKey, cbLast);
    cbLast.waitForValue();
    assertFalse(cbLast.val, string("Deleted something that wasn't there?"));
    time_t end = time(NULL);

    i++; // Did another transaction

    int delta = end - start;
    std::cout << "Ran " << i << "(" << cb.x << ") operations in "
              << delta << "s ("
              << (i/delta) << " ops/s)"
              << std::endl;
    return true;
}
