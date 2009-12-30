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

    static const char* generateValue(size_t min, size_t max) {
        size_t size = min + (random() & (max - min));
        assert(size >= min);
        assert(size <= max);

        char *rv = (char*)calloc(size + 1, sizeof(char));
        for (int i = 0; i < (int)size; i++) {
            rv[i] = nextChar();
        }
        return rv;
    }

    static void free_str(const char* p) {
        free((void*)p);
    }

    Values::Values(size_t n, size_t lower, size_t upper) : values(n) {
        for (int i = 0; i < (int)n; i++) {
            values[i] = generateValue(lower, upper);
        }
        assert(values.size() == n);

        it = values.begin();
    }

    Values::~Values() {
        std::for_each(values.begin(), values.end(), free_str);
    }

    const char *Values::nextValue() {
        const char *rv = *it;
        ++it;
        if (it == values.end()) {
            it = values.begin();
        }
        return rv;
    }

    size_t Values::length() {
        return values.size();
    }

}
