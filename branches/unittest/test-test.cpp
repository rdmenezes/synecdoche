#include <iostream>

#include <cppunit/TestCase.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

class Test: public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testFoo);
    CPPUNIT_TEST(testNonEq);
    CPPUNIT_TEST(testEq);
    CPPUNIT_TEST_SUITE_END();

public:
    void testFoo() {CPPUNIT_FAIL("not");}
    void testNonEq() {CPPUNIT_ASSERT_EQUAL(2, 1);}
    void testEq() {CPPUNIT_ASSERT_EQUAL(2, 2);}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

int main(int ac, char **av)
{
    CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);

    return runner.run() ? 0 : 1;
}
