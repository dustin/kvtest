#include <stdbool.h>

#include <iostream>
#include <list>

#include "base-test.hh"
#include "tests.hh"

using namespace kvtest;

TestSuite::TestSuite(ThingUnderTest *t) {
    tut = t;

    addTest(new TestTest());
}

bool TestSuite::run() {
    std::list<Test*>::iterator it;
    bool success = true;
    for (it=tests.begin() ; it != tests.end(); it++ ) {
        Test *t = *it;
        std::cout << "Running test ``" << *t << "'' ";
        try {
            t->run(tut);
            std::cout << "PASS" << std::endl;
        } catch(AssertionError &e) {
            success = false;
            std::cout << "FAIL: " << e.what() << std::endl;
        }
        tut->reset();
    }
    return success;
}

void TestSuite::addTest(Test *test) {
    tests.push_back(test);
}
