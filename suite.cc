#include <stdbool.h>

#include <iostream>
#include <list>

#include "base-test.hh"
#include "tests.hh"
#include "suite.hh"

using namespace kvtest;

TestSuite::TestSuite(KVStore *t) {
    tut = t;

    addTest(new TestTest());
    addTest(new WriteTest());
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
