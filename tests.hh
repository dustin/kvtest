#ifndef TESTS_H
#define TESTS_H 1

#include "base-test.hh"

class TestTest : public kvtest::Test {
public:
    bool run(kvtest::ThingUnderTest *tut);
    std::string name() { return "test test"; };
};

#endif /* TESTS_H */
