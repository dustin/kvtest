#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <list>

#include "base-test.hh"
#include "tests.hh"
#include "suite.hh"

using namespace kvtest;

TestSuite::TestSuite(KVStore *t) {
    tut = t;

    initTests();
}

void TestSuite::initTests() {
    const char *req = getenv("KVTEST_SUITE");
    if (req == NULL || (strcmp(req, "full") == 0)) {
        addTest(new TestTest());
        addTest(new WriteTest());
    } else if (strcmp(req, "test") == 0) {
        addTest(new TestTest());
    } else if (strcmp(req, "endurance") == 0) {
        addTest(new EnduranceTest());
    }
}

bool TestSuite::run() {
    std::list<Test*>::iterator it;
    bool success = true;
    for (it=tests.begin() ; it != tests.end(); it++ ) {
        success = false;
        Test *t = *it;
        std::cout << "Running test ``" << *t << "'' ";
        try {
            tut->reset();
            t->run(tut);
            std::cout << "PASS" << std::endl;
            success = true;
        } catch(AssertionError &e) {
            std::cout << "FAIL: " << e.what() << std::endl;
        } catch(std::runtime_error &e) {
            std::cout << "EXCEPTION: " << e.what() << std::endl;
        } catch(...) {
            std::cout << "EXCEPTION (unknown)" << std::endl;
        }
    }
    return success;
}

void TestSuite::addTest(Test *test) {
    tests.push_back(test);
}
