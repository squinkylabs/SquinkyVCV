#include "asserts.h"
#include "fVec.h"

static void test0()
{
    fVec<1> v;
    v.zero();
    for (int i = 0; i < 4; ++i) {
        float x = v.get()[i];
        assertEQ(x, 0);
    }
}

static void testAdd()
{
    fVec<2> v1;
    fVec<2> v2;
    for (int i = 0; i < 8; ++i) {
        v1.get()[i] = float(i);
        v2.get()[i] = float(i) + 10.f;
    }

    v1.add_i(v2);
    for (int i = 0; i < 8; ++i) {
        float x = v1.get()[i];
        assertEQ(x, 2 * i + 10);
    }
}


static void testMul()
{
    fVec<2> v1;
    fVec<2> v2;
    for (int i = 0; i < 8; ++i) {
        v1.get()[i] = float(i);
        v2.get()[i] = float(i);
    }

    v1.mul_i(v2);
    for (int i = 0; i < 8; ++i) {
        float x = v1.get()[i];
        assertEQ(x, i * i);
    }
}

void testVec()
{
    test0();
    testAdd();
    testMul();
}