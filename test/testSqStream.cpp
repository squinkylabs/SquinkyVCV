
#include "SqStream.h"
#include "asserts.h"

static void test0()
{
    SqStream s;
    std::string a = s.str();
    assertEQ(a, "");
}

static void test1()
{
    SqStream s;
    s.add("foo");
    std::string a = s.str();
    assertEQ(a, "foo");
}

static void test2()
{
    SqStream s;
    s.add(std::string("bar"));
    std::string a = s.str();
    assertEQ(a, "bar");
}

static void test3()
{
    SqStream s;
    s.add(12.342f);
    std::string a = s.str();
    assertEQ(a, "12.34");
}


static void test4()
{
    SqStream s;
    s.add("def");
    s.add(12.3f);
    std::string a = s.str();
    assertEQ(a, "def12.30");
}


static void test5()
{
    SqStream s;
    s.precision(1);
    s.add(12.342f);
    std::string a = s.str();
    assertEQ(a, "12.3");
}

static void test6()
{
    SqStream s;
    s.precision(1);
    s.add(12.342);
    std::string a = s.str();
    assertEQ(a, "12.3");
}

static void test7()
{
    SqStream s;
    s.add('x');
    std::string a = s.str();
    assertEQ(a, "x");
}

void testSqStream()
{
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
}