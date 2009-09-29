#include <stdio.h>
#include <iostream>

#include "base-test.hh"

class SimpleThing : public kvtest::ThingUnderTest {
public:
    void reset() {
    }

    bool set(std::string &key, std::string &val) {
        return true;
    }

    std::string get(std::string &key) {
        return NULL;
    }

private:
    bool openState;
};

int main(int argc, char **args) {
    SimpleThing *thing = new SimpleThing();

    kvtest::TestSuite suite(thing);
    suite.run();

    return 0;
}
