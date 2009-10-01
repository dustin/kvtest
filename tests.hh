#ifndef TESTS_H
#define TESTS_H 1

#include "base-test.hh"

namespace kvtest {

    /**
     * A test to run.
     */
    class Test : public Assertions {
    public:
        /**
         * Run the test.
         */
        virtual bool run(ThingUnderTest *tut) = 0;

        /**
         * Name of the test.
         */
        virtual std::string name() = 0;

        /**
         * Tests print out their names.
         */
        friend std::ostream& operator<<(std::ostream& s, Test &t) {
            return s << t.name();
        }

    };

}

class TestTest : public kvtest::Test {
public:
    bool run(kvtest::ThingUnderTest *tut);
    std::string name() { return "test test"; }
};

class WriteTest : public kvtest::Test {
public:
    bool run(kvtest::ThingUnderTest *tut);
    std::string name() { return "write test"; }
};

#endif /* TESTS_H */
