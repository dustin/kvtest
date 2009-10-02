#ifndef SUITE_H
#define SUITE_H 1

#include "base-test.hh"
#include "tests.hh"

namespace kvtest {

    /**
     * A series of tests to run.
     */
    class TestSuite {
    public:

        /**
         * Construct a test suite with the given thing under test.
         */
        TestSuite(KVStore *t);

        /**
         * Run the test suite.
         */
        bool run();

        /**
         * Add a test to the test suite.
         * (note: this is typically not necessary to do manually)
         */
        void addTest(Test *test);

    private:
        KVStore   *tut;
        std::list<Test*>  tests;
    };

}

#endif /* SUITE_H */
