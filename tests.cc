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

class LockHolder {
public:
    LockHolder(pthread_mutex_t *m) {
        mutex = m;
        if(pthread_mutex_lock(mutex) != 0) {
            throw std::runtime_error("Failed to acquire lock.");
        }
    };

    ~LockHolder() {
        pthread_mutex_unlock(mutex);
    }

private:
    pthread_mutex_t *mutex;
};

template <typename T>
class RememberingCallback : public Callback<T> {
public:
    RememberingCallback() {
        if(pthread_mutex_init(&mutex, NULL) != 0) {
            throw std::runtime_error("Failed to initialize mutex.");
        }
        if(pthread_cond_init(&cond, NULL) != 0) {
            throw std::runtime_error("Failed to initialize condition.");
        }
        fired = false;
    };

    ~RememberingCallback() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void callback(T &value) {
        LockHolder lh(&mutex);
        val = value;
        fired = true;
        if(pthread_cond_broadcast(&cond) != 0) {
            throw std::runtime_error("Failed to broadcast change.");
        }
    };

    void waitForValue() {
        LockHolder lh(&mutex);
        if (!fired) {
            if(pthread_cond_wait(&cond, &mutex) != 0) {
                throw std::runtime_error("Failed to wait for condition.");
            }
        }
        assert(fired);
    };

    T               val;
    bool            fired;

private:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

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
    cb.waitForValue();
    assertTrue(getCb.val.success, "Expected success getting initial value");
    assertEquals(getCb.val.value, testValue);

    RememberingCallback<bool> delCb1;
    tut->del(testKey, delCb1);
    cb.waitForValue();
    assertTrue(delCb1.val,  string("Failed to delete value."));

    RememberingCallback<bool> delCb2;
    tut->del(testKey, delCb2);
    cb.waitForValue();
    assertFalse(delCb2.val, string("Doubly deleted value"));

    RememberingCallback<GetValue> getCb2;
    tut->get(testKey, getCb2);
    cb.waitForValue();
    assertFalse(getCb2.val.success, "Expected failure getting final value.");
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
