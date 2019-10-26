#include "DiatonicUtils.h"
#include "asserts.h"

// simple reality check
static void assertTransposeValidC(const std::vector<int> _xpose, int amount)
{
    assertEQ(_xpose.size(), 12);

    auto transposedPitches = DiatonicUtils::getTransposedPitchesInC(_xpose);

    // should transpose exact amount +1 1 semi
    for (int i = 0; i < 12; ++i) {
        const int trans = _xpose[i];
        assertGE(trans, amount - 1);
        assertLE(trans, amount + 1);
    }

    int lastPitch = -1;
    for (int i = 0; i < 12; ++i) {
        const int x = _xpose[i];
        assertGE(x, -12);                 // valid entry

        const int thisPitch = i + x;
        assertGE(thisPitch, lastPitch);              // never decreasing
        lastPitch = thisPitch;

        assert(DiatonicUtils::isNoteInC(thisPitch));        // hard code to C for now
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

static void testTransposeC_1(int amount)
{
    auto x = DiatonicUtils::getTransposeInC(amount);
    assertTransposeValidC(x, amount);
}


static void testTransposeC_1()
{
    testTransposeC_1(0);
    testTransposeC_1(2);
    testTransposeC_1(1);
    for (int i = 0; i < 12; ++i) {
        testTransposeC_1(i);
    }
}

static void testRelativeMajor()
{
    int x = DiatonicUtils::getOffsetToRelativeMaj(DiatonicUtils::Modes::Major);
    assertEQ(x, 0);
    x = DiatonicUtils::getOffsetToRelativeMaj(DiatonicUtils::Modes::Minor);
    assertEQ(x, 3);


    // since C is already the relative major to A min, no transpose needed
    x = DiatonicUtils::getPitchOffsetRelativeToCMaj(DiatonicUtils::a, DiatonicUtils::Modes::Minor);
    assertEQ(x, 0);
};

static void testTransposeC2()
{
    int amt = 5;
    std::vector<int> xpose =  DiatonicUtils::getTranspose(amt , DiatonicUtils::c, DiatonicUtils::Modes::Major);
    DiatonicUtils::_dumpTransposes("just generated", xpose);
    assertTransposeValidC(xpose, amt);
}



static void testTransposeLambda()
{
    auto lambda = DiatonicUtils::makeTransposeLambda(
        1,      //int transposeSemitones,
        false,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);
}

void testDiatonicUtils()
{
    testIsNoteInC();
    testTransposeC_1();
    testRelativeMajor();
    testTransposeC2();
    testTransposeLambda();

}