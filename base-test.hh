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
     * Interface for callbacks from storage APIs.
     */
    template <typename RV>
    class Callback {
    public:
        /**
         * Method called on callback.
         */
        virtual void callback(RV value) {};
    };

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
        virtual void set(std::string &key, std::string &val,
                         Callback<bool> cb) = 0;

        /**
         * Get the value for the given key.
         *
         * @param key the key
         * @return the value or NULL if there's no value under this key
         */
        virtual void get(std::string &key, Callback<std::string*> cb) = 0;

        /**
         * Delete a value for a key.
         *
         * @param key the key
         * @return true if the value was there and is now deleted
         */
        virtual void del(std::string &key, Callback<bool>) = 0;
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

        inline void assertNotNull(std::string *s) {
            assertTrue(s != NULL, "expected nonnull");
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
        bool run();

        /**
         * Add a test to the test suite.
         * (note: this is typically not necessary to do manually)
         */
        void addTest(Test *test);

    private:
        ThingUnderTest   *tut;
        std::list<Test*>  tests;
    };

}

#endif /* BASE_TEST_H */
