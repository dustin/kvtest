#include <stdio.h>
#include <iostream>
#include <map>

#include "base-test.hh"

class SimpleThing : public kvtest::ThingUnderTest {
public:
    bool set(std::string &key, std::string &val) {
        storage[key] = val;
        return true;
    }

    std::string* get(std::string &key) {
        std::map<std::string, std::string>::iterator it = storage.find(key);
        std::string *rv = it == storage.end() ? NULL : &(it->second);
        return rv;
    }

    bool del(std::string &key) {
        bool rv = true;
        if(storage.find(key) == storage.end()) {
            rv = false;
        }
        storage.erase(key);
        return rv;
    }

private:
    std::map<std::string, std::string> storage;
};

int main(int argc, char **args) {
    SimpleThing *thing = new SimpleThing();

    kvtest::TestSuite suite(thing);
    return suite.run() ? 0 : 1;
}
