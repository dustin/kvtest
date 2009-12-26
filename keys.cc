#include <iostream>
#include <iterator>
#include <algorithm>

#include "keys.hh"

#define KEY_LEN 16

namespace kvtest {

    std::string* generateKey();

    Keys::Keys(size_t n) {
        len = n;

        while (keys.size() < n) {
            keys.insert(generateKey());
        }

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
        return len;
    }

    std::string* generateKey() {
        const char *keyvals = "abcdefghijklmnopqrstuvwyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789";
        std::string *rv = new std::string();
        while (rv->length() < KEY_LEN) {
            rv->append(1, (char)keyvals[random() % strlen(keyvals)]);
        }
        return rv;
    }

}
