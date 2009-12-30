#ifndef VALUES_HH
#define VALUES_HH 1

#include <vector>

#include <iostream>

#include "base-test.hh"

namespace kvtest {

    /**
     * A collection of values to be reused for a test.
     */
    class Values {
    public:
        /**
         * Instantiate a Values object that generates the given number
         * of keys ranging between low and high bytes.
         */
        Values(size_t numVals, size_t lower, size_t upper);

        /**
         * Clean up the allocated values.
         */
        ~Values();

        /**
         * Grab a value.
         */
        const char *nextValue();

        /**
         * Get the number of values in this Values object.
         */
        size_t length();
    private:
        std::vector<const char *> values;
        std::vector<const char *>::iterator it;
    };

}

#endif /* VALUES_HH */
