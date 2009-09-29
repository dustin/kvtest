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

    std::string get(std::string &key) {
        return storage.find(key)->second;
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
    suite.run();

    return 0;
}
