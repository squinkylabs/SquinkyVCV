#include "DiatonicUtils.h"
#include "asserts.h"

// simple reality check
static void assertTransposeValid(const std::vector<int> xpose)
{
    assertEQ(xpose.size(), 12);

    int last = -1;
    for (int i = 0; i < 12; ++i) {
        int x = xpose[i];
        assertGE(x, 0);
        assertGE(x, last);
        last = x;

    }
}


static void testIsNoteInC()
{
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::c));
    assert(!DiatonicUtils::isNoteInC(DiatonicUtils::c_));
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::d));
    assert(!DiatonicUtils::isNoteInC(DiatonicUtils::d_));
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::e));
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::f));
    assert(!DiatonicUtils::isNoteInC(DiatonicUtils::f_));
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::g));
    assert(!DiatonicUtils::isNoteInC(DiatonicUtils::g_));
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::a));
    assert(!DiatonicUtils::isNoteInC(DiatonicUtils::a_));
    assert(DiatonicUtils::isNoteInC(DiatonicUtils::b));
}

static void testTransposeC_1()
{
    auto x = DiatonicUtils::getTransposeInC(0);
    assertTransposeValid(x);

}
void testDiatonicUtils()
{
    testIsNoteInC();
    testTransposeC_1();

}