#ifndef BASE_TEST_H
#define BASE_TEST_H 1

#include <assert.h>
#include <stdbool.h>

#include <list>

/**
 * General kvtest interfaces.
 */
namespace kvtest {

    /**
     * An individual kv storage (or way to access a kv storage).
     */
    class ThingUnderTest {
    public:
        /**
         * Called after each test to reinitialize the test.
         */
        virtual void reset() {};

        /**
         * Set a given key and value.
         *
         * @param key the key to set
         * @param val the value to set
         * @return true if the set succeeded.
         */
        virtual bool set(std::string &key, std::string &val) = 0;

        /**
         * Get the value for the given key.
         *
         * @param key the key
         * @return the value or NULL if there's no value under this key
         */
        virtual std::string get(std::string &key) = 0;

        /**
         * Delete a value for a key.
         *
         * @param key the key
         * @return true if the value was there and is now deleted
         */
        virtual bool del(std::string &key) = 0;
    };

    /**
     * A test to run.
     */
    class Test {
    public:
        /**
         * Run the test.
         */
        virtual bool run(ThingUnderTest *tut) = 0;
        /**
         * Name of the test.
         */
        virtual std::string name() = 0;
        /**
         * Tests print out their names.
         */
        friend std::ostream& operator<<(std::ostream& s, Test &t) {
            return s << t.name();
        }
    };

    /**
     * A series of tests to run.
     */
    class TestSuite {
    public:

        /**
         * Construct a test suite with the given thing under test.
         */
        TestSuite(ThingUnderTest *t);

        /**
         * Run the test suite.
         */
        bool run() {
            std::list<Test*>::iterator it;
            for (it=tests.begin() ; it != tests.end(); it++ ) {
                Test *t = *it;
                std::cout << "Running test ``" << *t << "''" << std::endl;
                t->run(tut);
                tut->reset();
            }
        }

        /**
         * Add a test to the test suite.
         * (note: this is typically not necessary to do manually)
         */
        void addTest(Test *test) {
            tests.push_back(test);
        }

    private:
        ThingUnderTest   *tut;
        std::list<Test*>  tests;
    };

}

#endif /* BASE_TEST_H */
