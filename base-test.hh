#ifndef BASE_TEST_H
#define BASE_TEST_H 1

#include <assert.h>
#include <stdbool.h>
#include <stdexcept>

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
        virtual std::string* get(std::string &key) = 0;

        /**
         * Delete a value for a key.
         *
         * @param key the key
         * @return true if the value was there and is now deleted
         */
        virtual bool del(std::string &key) = 0;
    };

    /**
     * Assertion errors.
     */
    class AssertionError : public std::runtime_error {
    public:
        AssertionError(const char *s) : std::runtime_error(s) { };
        AssertionError(const std::string s) : std::runtime_error(s) { };
    };

    /**
     * Assertion mixins.
     */
    class Assertions {
    public:
        inline void assertTrue(bool v, std::string msg) {
            if(!v) {
                throw AssertionError(msg);
            }
        };

        inline void assertFalse(bool v, std::string msg) {
            assertTrue(!v, msg);
        };

        inline void assertEquals(std::string s1, std::string s2) {
            assertTrue(s1 == s2, "failed string compare");
        };

        inline void assertNull(std::string *s) {
            if (s != NULL) {
                std::string msg = "expected null, got ``" + *s + "''";
                throw AssertionError(msg);
            }
        };
    };

    /**
     * A test to run.
     */
    class Test : public Assertions {
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
