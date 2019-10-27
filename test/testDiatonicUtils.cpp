
#include "DiatonicUtils.h"
#include "PitchUtils.h"
#include "asserts.h"

#include <sstream>

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

    // verify that scale degree go to scale degrees, and the correct ones
    for (int i = 0; i < 12; ++i) {
        const int x = _xpose[i];
        if (DiatonicUtils::isNoteInC(i)) {
            const int originalDegree = DiatonicUtils::getScaleDegreeInC(i);
            const int xposeDegrees = DiatonicUtils::quantizeXposeToScaleDegreeInC(_xpose[i]);
            const int degreeAfterTranspose = DiatonicUtils::getScaleDegreeInC(i + _xpose[i]);

            const int degreesXposed = degreeAfterTranspose - originalDegree;
            assertEQ(degreesXposed, degreeAfterTranspose);
        }
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

    std::stringstream str;
    str << "test c1 (" << amount << ")";
 
    DiatonicUtils::_dumpTransposes(str.str().c_str(), x);
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

    // There is a bug that 2 and 26 go to different scale degrees
    testTransposeC_1(24);
    testTransposeC_1(2);

    // TODO: put this bacd.
 //   testTransposeC_1(-24);
    testTransposeC_1(26);
    testTransposeC_1(50);

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



static void testTransposeLambdaSemi()
{
    // chromatic, one semi
    auto lambdaSemi = DiatonicUtils::makeTransposeLambda(
        1,      //int transposeSemitones,
        false,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    float x = lambdaSemi(0);
    assertClose(x, PitchUtils::semitone, .00001);
    x = lambdaSemi(1);
    assertClose(x, 1 + PitchUtils::semitone, .00001);
}

static void testTransposeLambdaFifth()
{
    // chromatic, one semi
    auto lambdaSemi = DiatonicUtils::makeTransposeLambda(
        7,      //int transposeSemitones,
        false,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    float x = lambdaSemi(0);
    assertClose(x, 7 * PitchUtils::semitone, .00001);
    x = lambdaSemi(1);
    assertClose(x, 1 + 7 * PitchUtils::semitone, .00001);
}

static void testTransposeLambdaDiatonicWhole()
{
    auto lambdaDiatonicWhole = DiatonicUtils::makeTransposeLambda(
        2,      //int transposeSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);        // c major

    //c -> d
    float x = lambdaDiatonicWhole(0);
    assertClose(x, DiatonicUtils::d * PitchUtils::semitone, .00001);

    // e -> f
    x = lambdaDiatonicWhole(DiatonicUtils::e * PitchUtils::semitone);
    assertClose(x, DiatonicUtils::f * PitchUtils::semitone, .00001);
}


static void testTransposeLambdaDiatonicWholeOct()
{
    auto lambdaDiatonicWhole = DiatonicUtils::makeTransposeLambda(
        2 + 2 * 12,   
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);        // c major

    //c -> d
    float x = lambdaDiatonicWhole(0);
    assertClose(x, 2 + DiatonicUtils::d * PitchUtils::semitone, .00001);

    // e -> f
    x = lambdaDiatonicWhole(DiatonicUtils::e * PitchUtils::semitone);
    assertClose(x, 2 + DiatonicUtils::f * PitchUtils::semitone, .00001);
}

static void testGetScaleDegreeInC()
{
    assertEQ(DiatonicUtils::getScaleDegreeInC(0), 0);
    assertEQ(DiatonicUtils::getScaleDegreeInC(2), 1);
    assertEQ(DiatonicUtils::getScaleDegreeInC(4), 2);
    assertEQ(DiatonicUtils::getScaleDegreeInC(5), 3);
    assertEQ(DiatonicUtils::getScaleDegreeInC(7), 4);
    assertEQ(DiatonicUtils::getScaleDegreeInC(9), 5);
    assertEQ(DiatonicUtils::getScaleDegreeInC(11), 6);
}

static void testqQuantizeXposeToScaleDegreeInC()
{
    assert(false);
}

void testDiatonicUtils()
{
    testIsNoteInC();
    testGetScaleDegreeInC();
    testqQuantizeXposeToScaleDegreeInC();
    testTransposeC_1();
    testRelativeMajor();
    testTransposeC2();
    testTransposeLambdaSemi();
    testTransposeLambdaFifth();
    testTransposeLambdaDiatonicWhole();
    testTransposeLambdaDiatonicWholeOct();
}