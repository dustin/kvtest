#include <stdio.h>
#include <iostream>

#include "base-test.hh"

class SimpleThing : public testing::ThingUnderTest {
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

    testing::TestSuite suite(thing);
    suite.run();

    return 0;
}
