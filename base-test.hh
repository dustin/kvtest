#ifndef BASE_TEST_H
#define BASE_TEST_H 1

#include <assert.h>
#include <stdbool.h>

#include <list>

namespace kvtest {

    class ThingUnderTest {
    public:
        ThingUnderTest() {};
        virtual void reset() = 0;
        virtual bool set(std::string &key, std::string &val) = 0;
        virtual std::string get(std::string &key) = 0;
    };

    class Test {
    public:
        virtual bool run(ThingUnderTest *tut) = 0;
        virtual std::string name() = 0;
        friend std::ostream& operator<<(std::ostream& s, Test &t) {
            return s << t.name();
        }
    };
    class TestSuite {
    public:
        TestSuite(ThingUnderTest *t);

        bool run() {
            std::list<Test*>::iterator it;
            for (it=tests.begin() ; it != tests.end(); it++ ) {
                Test *t = *it;
                std::cout << "Running test ``" << *t << "''" << std::endl;
                t->run(tut);
                tut->reset();
            }
        }

        void addTest(Test *test) {
            tests.push_back(test);
        }

    private:
        ThingUnderTest   *tut;
        std::list<Test*>  tests;
    };

}

#endif /* BASE_TEST_H */
