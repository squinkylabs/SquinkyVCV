
#include "DiatonicUtils.h"
#include "PitchUtils.h"
#include "SqMidiEvent.h"
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

    auto normalizedAmount = DiatonicUtils::normalizePitch(amount);
    const int transposeSemis = normalizedAmount.second;
    const int numDegreesToTranspose = DiatonicUtils::quantizeXposeToScaleDegreeInC(transposeSemis);

    // verify that scale degree go to scale degrees, and the correct ones
    for (int i = 0; i < 12; ++i) {
        const int x = _xpose[i];
        if (DiatonicUtils::isNoteInC(i)) {

            printf("\nin loop, i=%d\n", i);
            // first compute what it should be
            const int originalDegree = DiatonicUtils::getScaleDegreeInC(i);

            int expectedDegreeAfterXpose = originalDegree + numDegreesToTranspose;
            while (expectedDegreeAfterXpose > 6) {
                expectedDegreeAfterXpose -= 7;
            }
            printf("orig degree = %d, xpose deg = %d expected deg after = %d\n",
                originalDegree, numDegreesToTranspose, expectedDegreeAfterXpose);

            // then look at what it really did
            const int xp = _xpose[i];
            const int x = i + _xpose[i];
            const int actualDgreeAferXpose = DiatonicUtils::getScaleDegreeInC(i + _xpose[i]);
            printf("xpose amt from array = %d, makes chromatic pitch after xpose %d degree = %d\n",
                xp, x, actualDgreeAferXpose);


            assertEQ(actualDgreeAferXpose, expectedDegreeAfterXpose);
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
    str << "test c1 (amt=" << amount << ")";
 
    DiatonicUtils::_dumpTransposes(str.str().c_str(), x);
    assertTransposeValidC(x, amount);
}


static void testTransposeC_1()
{
    // for debugging, do problematic first
    testTransposeC_1(2);

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

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;

    lambdaSemi(note);
    float x = note->pitchCV;
    assertClose(x, PitchUtils::semitone, .00001);

    note->pitchCV = 1;
    lambdaSemi(note);
    assertClose(note->pitchCV, 1 + PitchUtils::semitone, .00001);
}

#if 1
static void testTransposeLambdaFifth()
{
    // chromatic, one semi
    auto lambdaSemi = DiatonicUtils::makeTransposeLambda(
        7,      //int transposeSemitones,
        false,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambdaSemi(note);
    assertClose(note->pitchCV, 7 * PitchUtils::semitone, .00001);
    note->pitchCV = 1;
    lambdaSemi(note);
    assertClose(note->pitchCV, 1 + 7 * PitchUtils::semitone, .00001);
}

static void testTransposeLambdaDiatonicWhole()
{
    auto lambdaDiatonicWhole = DiatonicUtils::makeTransposeLambda(
        2,      //int transposeSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);        // c major

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    //c -> d
    note->pitchCV = 0;
    lambdaDiatonicWhole(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone, .00001);

    // e -> f
    note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
    lambdaDiatonicWhole(note);
    assertClose(note->pitchCV, DiatonicUtils::f * PitchUtils::semitone, .00001);
}


static void testTransposeLambdaDiatonicWholeOct()
{
    auto lambdaDiatonicWhole = DiatonicUtils::makeTransposeLambda(
        2 + 2 * 12,   
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);        // c major

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    //c -> d
    note->pitchCV = 0;
    lambdaDiatonicWhole(note);
    assertClose(note->pitchCV, 2 + DiatonicUtils::d * PitchUtils::semitone, .00001);

    // e -> f
    note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
    lambdaDiatonicWhole(note);
    assertClose(note->pitchCV, 2 + DiatonicUtils::f * PitchUtils::semitone, .00001);
}
#endif

static void testGetScaleDegreeInC()
{
    assertEQ(DiatonicUtils::getScaleDegreeInC(0), 0);
    assertEQ(DiatonicUtils::getScaleDegreeInC(2), 1);
    assertEQ(DiatonicUtils::getScaleDegreeInC(4), 2);
    assertEQ(DiatonicUtils::getScaleDegreeInC(5), 3);
    assertEQ(DiatonicUtils::getScaleDegreeInC(7), 4);
    assertEQ(DiatonicUtils::getScaleDegreeInC(9), 5);
    assertEQ(DiatonicUtils::getScaleDegreeInC(11), 6);

    assertEQ(DiatonicUtils::getScaleDegreeInC(12), 0);
    assertEQ(DiatonicUtils::getScaleDegreeInC(24), 0);

}

static void testqQuantizeXposeToScaleDegreeInC()
{
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(0), 0);

    // 2nd
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(1), 1);
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(2), 1);

    // 3 rd
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(3), 2);
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(4), 2);

    // 4 th
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(5), 3);

     // 5 th
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(6), 4);       // make tritone be a 5th
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(7), 4);

     // 6 th
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(8), 5);
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(9), 5);

     // 7 th
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(10), 6)
    assertEQ(DiatonicUtils::quantizeXposeToScaleDegreeInC(11), 6);
}

static void testGetPitchFromScaleDegree()
{
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0), 0);         // unison
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1), 2);         // maj 2
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2), 4);         //maj 3
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3), 5);         // 4th
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4), 7);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5), 9);         // maj 6

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6), 11);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(7), 12);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(8), 14);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(9), 16);
}
void testDiatonicUtils()
{
    testIsNoteInC();
    testGetScaleDegreeInC();
    testqQuantizeXposeToScaleDegreeInC();
    testGetPitchFromScaleDegree();
    testTransposeC_1();
    testRelativeMajor();
    testTransposeC2();
    testTransposeLambdaSemi();
    printf("********* put back the lambda tests\n");
#if 1
    testTransposeLambdaFifth();
    testTransposeLambdaDiatonicWhole();
    testTransposeLambdaDiatonicWholeOct();
#endif
}