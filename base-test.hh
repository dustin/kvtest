#ifndef BASE_TEST_H
#define BASE_TEST_H 1

#include <assert.h>
#include <stdbool.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <list>

#include "locks.hh"
#include "callbacks.hh"

/**
 * General kvtest interfaces.
 */
namespace kvtest {

    /**
     * Value for callback for GET operations.
     */
    class GetValue {
    public:
        GetValue() { }

        GetValue(std::string v, bool s) {
            value = v;
            success = s;
        }

        friend std::ostream& operator<<(std::ostream &o, GetValue &gv) {
            return o << "{GetValue success=" << gv.success
                     << ", value=\"" << gv.value << "\"}";
        }

        /**
         * The value retrieved for the key.
         */
        std::string value;
        /**
         * True if a value was successfully retrieved.
         */
        bool success;
    };

    /**
     * An individual kv storage (or way to access a kv storage).
     */
    class ThingUnderTest {
    public:
        /**
         * Called after each test to reinitialize the test.
         */
        virtual void reset() {}

        /**
         * Method that should not return until the driver has done its job.
         *
         * @param c the callback that will fire when the noop is evalutated
         */
        virtual void noop(Callback<bool> &c) {
            bool t = true;
            c.callback(t);
        }

        /**
         * Set a given key and value.
         *
         * @param key the key to set
         * @param val the value to set
         * @param cb callback that will fire with true if the set succeeded
         */
        virtual void set(std::string &key, std::string &val,
                         Callback<bool> &cb) = 0;

        /**
         * Get the value for the given key.
         *
         * @param key the key
         * @param cb callback that will fire with the retrieved value
         */
        virtual void get(std::string &key, Callback<GetValue> &cb) = 0;

        /**
         * Delete a value for a key.
         *
         * @param key the key
         * @param cb callback that will fire with true if the value
         *           existed and then was deleted
         */
        virtual void del(std::string &key, Callback<bool> &cb) = 0;

    };

    /**
     * Assertion errors.
     */
    class AssertionError : public std::runtime_error {
    public:
        AssertionError(const char *s) : std::runtime_error(s) { }
        AssertionError(const std::string s) : std::runtime_error(s) { }
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
        }

        inline void assertFalse(bool v, std::string msg) {
            assertTrue(!v, msg);
        }

        inline void assertEquals(std::string s1, std::string s2) {
            assertTrue(s1.compare(s2) == 0, "failed string compare");
        }

        inline void assertEquals(int i1, int i2) {
            std::stringstream ss;
            ss << "Expected " << i1 << " got " << i2;
            assertTrue(i1 == i2, ss.str());
        }

        inline void assertNull(std::string *s) {
            if (s != NULL) {
                std::string msg = "expected null, got ``" + *s + "''";
                throw AssertionError(msg);
            }
        }

        inline void assertNotNull(std::string *s) {
            assertTrue(s != NULL, "expected nonnull");
        }
    };
}

#endif /* BASE_TEST_H */
