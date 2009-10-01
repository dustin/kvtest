#include <stdio.h>
#include <iostream>
#include <map>

#include "base-test.hh"

class SimpleThing : public kvtest::ThingUnderTest {
public:
    void set(std::string &key, std::string &val,
             kvtest::Callback<bool> cb) {
        storage[key] = val;
        cb.callback(true);
    }

    void get(std::string &key, kvtest::Callback<std::string*> cb) {
        std::map<std::string, std::string>::iterator it = storage.find(key);
        std::string *rv = it == storage.end() ? NULL : &(it->second);
        cb.callback(rv);
    }

    void del(std::string &key, kvtest::Callback<bool> cb) {
        bool rv = true;
        if(storage.find(key) == storage.end()) {
            rv = false;
        }
        storage.erase(key);
        cb.callback(rv);
    }

private:
    std::map<std::string, std::string> storage;
};

int main(int argc, char **args) {
    SimpleThing *thing = new SimpleThing();

    kvtest::TestSuite suite(thing);
    return suite.run() ? 0 : 1;
}
