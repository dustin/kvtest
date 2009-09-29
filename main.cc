#include <stdio.h>
#include <iostream>

#include "base-test.hh"

class SimpleThing : public kvtest::ThingUnderTest {
public:
    bool set(std::string &key, std::string &val) {
        return true;
    }

    std::string get(std::string &key) {
        return NULL;
    }
};

int main(int argc, char **args) {
    SimpleThing *thing = new SimpleThing();

    kvtest::TestSuite suite(thing);
    suite.run();

    return 0;
}
