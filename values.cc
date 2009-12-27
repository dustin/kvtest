#include <iostream>
#include <iterator>
#include <set>
#include <vector>
#include <algorithm>
#include <assert.h>

#include <string.h>

#include "values.hh"

namespace kvtest {

    static char nextChar() {
        static const char *keyvals = "abcdefghijklmnopqrstuvwyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789";
        return keyvals[random() % strlen(keyvals)];
    }

    /* The generator. */
    class ARockAPlanetAnAtomBomb {
    public:

        ARockAPlanetAnAtomBomb(size_t minSize, size_t maxSize) {
            min = minSize;
            max = maxSize;
        }

        std::string *operator()() {
            size_t size = min + (random() & (max - min));
            assert(size >= min);
            assert(size <= max);

            std::string *rv = new std::string(size, char());
            std::generate_n(rv->begin(), size, nextChar);
            return rv;
        };

    private:
        std::string *data;
        size_t min;
        size_t max;
    };

    static void killValue(std::string *v);

    Values::Values(size_t n, size_t lower, size_t upper) : values(n) {
        ARockAPlanetAnAtomBomb generator(lower, upper);
        std::generate_n(values.begin(), n, generator);
        assert(values.size() == n);

        it = values.begin();
    }

    Values::~Values() {
        std::for_each(values.begin(), values.end(), killValue);
    }

    std::string *Values::nextValue() {
        std::string *rv = *it;
        ++it;
        if (it == values.end()) {
            it = values.begin();
        }
        return rv;
    }

    size_t Values::length() {
        return values.size();
    }

    static void killValue(std::string *v) {
        delete v;
    }

}
