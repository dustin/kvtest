#include <iostream>
#include <iterator>
#include <set>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "keys.hh"

#define KEY_LEN 16

namespace kvtest {

    std::string* generateKey();

    Keys::Keys(size_t n) : keys(n) {

        std::generate_n(keys.begin(), n, generateKey);
        assert(keys.size() == n);

        it = keys.begin();
    }

    const std::string* Keys::nextKey() {
        const std::string *rv = *it;
        ++it;
        if (it == keys.end()) {
            it = keys.begin();
        }
        return rv;
    }

    size_t Keys::length() {
        return keys.size();
    }

    char nextChar() {
        static const char *keyvals = "abcdefghijklmnopqrstuvwyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789";
        return keyvals[random() % strlen(keyvals)];
    }

    std::string* generateKey() {
        std::string *rv = new std::string(KEY_LEN, char());
        std::generate_n(rv->begin(), KEY_LEN, nextChar);
        return rv;
    }

}
