#include <stdio.h>
#include <iostream>
#include <map>

#include "base-test.hh"
#include "suite.hh"

class SimpleThing : public kvtest::KVStore {
public:
    void set(std::string &key, std::string &val,
             kvtest::Callback<bool> &cb) {
        storage[key] = val;
        bool rv = true;
        cb.callback(rv);
    }

    void get(std::string &key, kvtest::Callback<kvtest::GetValue> &cb) {
        std::map<std::string, std::string>::iterator it = storage.find(key);
        bool success = it != storage.end();
        kvtest::GetValue rv(success ? it->second : std::string(":("),
                            success);
        cb.callback(rv);
    }

    void del(std::string &key, kvtest::Callback<bool> &cb) {
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
    SimpleThing thing;
    kvtest::TestSuite suite(&thing);
    return suite.run() ? 0 : 1;
}
