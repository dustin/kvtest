#ifndef KEYS_HH
#define KEYS_HH 1

#include <set>

#include <iostream>

#include "base-test.hh"

namespace kvtest {

    /**
     * A collection of keys to be reused for a test.
     */
    class Keys {
    public:
        /**
         * Instantiate a Keys object that generates the given number of keys.
         */
        Keys(size_t numKeys);

        /**
         * Grab a key.
         */
        const std::string* nextKey();

        /**
         * Get the number of keys in this Keys object.
         */
        size_t length();
    private:
        std::set<const std::string *> keys;
        std::set<const std::string *>::iterator it;
        size_t len;
    };

}

#endif /* KEYS_HH */
