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

void testVec()
{
    test0();
}