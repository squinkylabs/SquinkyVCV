
#include "DiatonicUtils.h"
#include "PitchUtils.h"
#include "SqMidiEvent.h"
#include "asserts.h"

#include <sstream>

// simple reality check
static void assertTransposeValidC_Quantized(const std::vector<int> _xpose, int amount)
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

          //  printf("\nin loop, i=%d\n", i);
            // first compute what it should be
            const int originalDegree = DiatonicUtils::getScaleDegreeInC(i);

            int expectedDegreeAfterXpose = originalDegree + numDegreesToTranspose;
            while (expectedDegreeAfterXpose > 6) {
                expectedDegreeAfterXpose -= 7;
            }
         //   printf("orig degree = %d, xpose deg = %d expected deg after = %d\n", originalDegree, numDegreesToTranspose, expectedDegreeAfterXpose);

            // then look at what it really did
            const int xp = _xpose[i];
            const int x = i + _xpose[i];
            const int actualDgreeAferXpose = DiatonicUtils::getScaleDegreeInC(i + _xpose[i]);
          //  printf("xpose amt from array = %d, makes chromatic pitch after xpose %d degree = %d\n",  xp, x, actualDgreeAferXpose);


            assertEQ(actualDgreeAferXpose, expectedDegreeAfterXpose);
        }
    }
}

static void assertTransposeValidC_Informed(const std::vector<int> _xpose, int amount)
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

        if (DiatonicUtils::isNoteInC(i)) {
            // notes in pitch must stay in pitch
            assert(DiatonicUtils::isNoteInC(thisPitch));
        }
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
         //   printf("orig degree = %d, xpose deg = %d expected deg after = %d\n",              originalDegree, numDegreesToTranspose, expectedDegreeAfterXpose);

            // then look at what it really did
            const int xp = _xpose[i];
            const int x = i + _xpose[i];
            const int actualDgreeAferXpose = DiatonicUtils::getScaleDegreeInC(i + _xpose[i]);
         //   printf("xpose amt from array = %d, makes chromatic pitch after xpose %d degree = %d\n",          xp, x, actualDgreeAferXpose);


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

static void testTransposeC_Quantized(int amount)
{
    auto x = DiatonicUtils::getTransposeInC(amount, true);

    std::stringstream str;
    str << "test c1-quant (amt=" << amount << ")";
 
    //DiatonicUtils::_dumpTransposes(str.str().c_str(), x);
    assertTransposeValidC_Quantized(x, amount);
}


static void testTransposeC_Informed(int amount)
{
    auto x = DiatonicUtils::getTransposeInC(amount, false);

    std::stringstream str;
    str << "test c1-inf (amt=" << amount << ")";

   // DiatonicUtils::_dumpTransposes(str.str().c_str(), x);
    assertTransposeValidC_Informed(x, amount);
}

static void testTransposeC_Informed()
{
    testTransposeC_Informed(0);
    testTransposeC_Informed(2);
    testTransposeC_Informed(1);
    for (int i = 0; i < 12; ++i) {
        testTransposeC_Informed(i);
    }

     // There is a bug that 2 and 26 go to different scale degrees
    testTransposeC_Informed(24);
    testTransposeC_Informed(2);

    // TODO: put this back.
    //testTransposeC_Informed(-24);
    printf("transpose -24 test removed\n");
    testTransposeC_Informed(26);
    testTransposeC_Informed(50);
    printf("whew!\n");
}

static void testTransposeC_Quantized()
{
    testTransposeC_Quantized(0);
    testTransposeC_Quantized(2);
    testTransposeC_Quantized(1);
    for (int i = 0; i < 12; ++i) {
        testTransposeC_Quantized(i);
    }

    // There is a bug that 2 and 26 go to different scale degrees
    testTransposeC_Quantized(24);
    testTransposeC_Quantized(2);

    // TODO: put this back.
    //testTransposeC_Quantized(-24);
    printf("transpose -24 test removed\n");
    testTransposeC_Quantized(26);
    testTransposeC_Quantized(50);

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
    std::vector<int> xpose =  DiatonicUtils::getTranspose(amt , DiatonicUtils::c, DiatonicUtils::Modes::Major, true);
    //DiatonicUtils::_dumpTransposes("just generated", xpose);
    assertTransposeValidC_Quantized(xpose, amt);
}

static void testTransposeAMinor()
{
    int amt = 2;        // xpose a whole step
    std::vector<int> xpose = DiatonicUtils::getTranspose(amt, DiatonicUtils::a, DiatonicUtils::Modes::Minor, false);
 //   DiatonicUtils::_dumpTransposes("just generated a minor xpose", xpose);

    // a -> b
    assertEQ(xpose[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::b);
    assertEQ(xpose[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::c + 12);
    assertEQ(xpose[DiatonicUtils::c] + DiatonicUtils::c, DiatonicUtils::d);
    assertEQ(xpose[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::e);
    assertEQ(xpose[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::f);
    assertEQ(xpose[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::g);

    assertEQ(xpose[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::a);

}

static void testTransposeDMajor()
{
    int amt = 2;        // xpose a whole step
    std::vector<int> xpose = DiatonicUtils::getTranspose(amt, DiatonicUtils::d, DiatonicUtils::Modes::Major, false);
    assertEQ(xpose[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::e);
    assertEQ(xpose[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::f_);
    assertEQ(xpose[DiatonicUtils::f_] + DiatonicUtils::f_, DiatonicUtils::g);
    assertEQ(xpose[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::a);
    assertEQ(xpose[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::b);
    assertEQ(xpose[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::c_ + 12);
    assertEQ(xpose[DiatonicUtils::c_] + DiatonicUtils::c_, DiatonicUtils::d);
}

static void testTransposeEPhrygian()
{
    int amt = 2;        // xpose a whole step
    std::vector<int> xpose = DiatonicUtils::getTranspose(amt, DiatonicUtils::e, DiatonicUtils::Modes::Phrygian, false);
    //DiatonicUtils::_dumpTransposes("just generated e phrygian xpose", xpose);

    // a -> b
    assertEQ(xpose[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::b);
    assertEQ(xpose[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::c + 12);
    assertEQ(xpose[DiatonicUtils::c] + DiatonicUtils::c, DiatonicUtils::d);
    assertEQ(xpose[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::e);
    assertEQ(xpose[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::f);
    assertEQ(xpose[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::g);

    assertEQ(xpose[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::a);
}


static void testTransposeEFlat()
{
    int amt = 3;        // xpose a third
    std::vector<int> xpose = DiatonicUtils::getTranspose(amt, DiatonicUtils::d_, DiatonicUtils::Modes::Minor, false);
    
    //eflat -> gflat
    assertEQ(xpose[DiatonicUtils::d_] + DiatonicUtils::d_, DiatonicUtils::f_);
    assertEQ(xpose[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::g_);
    assertEQ(xpose[DiatonicUtils::f_] + DiatonicUtils::f_, DiatonicUtils::a_);

    assertEQ(xpose[DiatonicUtils::g_] + DiatonicUtils::g_, DiatonicUtils::b);
    assertEQ(xpose[DiatonicUtils::a_] + DiatonicUtils::a_, DiatonicUtils::c_+12);
    assertEQ(xpose[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::d_+12);

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

static void assertInvertValidInC(const std::vector<int> invert, int axis)
{
    assertEQ(invert.size(), 12);
    for (int i = 0; i < 12; ++i) {
        assert(invert[i] > -100);
        const int inverted = i + invert[i];
        if (DiatonicUtils::isNoteInC(i))  {
            assert(DiatonicUtils::isNoteInC(i));
        }
    }
}


static void testInvertInC(int axis)
{
    auto x = DiatonicUtils::getInvertInCInformed(axis);

    std::stringstream str;
    str << "test c1-inv (axis=" << axis << ")";

   // DiatonicUtils::_dumpTransposes(str.str().c_str(), x);
    assertInvertValidInC(x, axis);
}

static void testInvertInC()
{
    testInvertInC(0);
    for (int i = 0; i < 11; ++i) {
        testInvertInC(i);
    }
}

static void testInvertLambdaDirection()
{
    int axis = 0;
    auto lambda = DiatonicUtils::makeInvertLambda(
        axis,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);        // c major

    float lastPitch = 10000;
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    for (int i = -40; i < 40; ++i) {
        note->pitchCV = PitchUtils::semitoneToCV(i);
        lambda(note);
        assert(note->pitchCV > lastPitch);
        lastPitch = note->pitchCV;
    }
}

static void testInvert()
{
    int axis = 0;
    std::vector<int> invert = DiatonicUtils::getInvert(axis, DiatonicUtils::c, DiatonicUtils::Modes::Major);
    DiatonicUtils::_dumpTransposes("just generated", invert);
    assertInvertValidInC(invert, axis);

    assertEQ(invert[DiatonicUtils::c] + DiatonicUtils::c, DiatonicUtils::c);
    assertEQ(invert[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::b - 12);
    assertEQ(invert[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::a - 12);
    assertEQ(invert[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::g - 12);
    assertEQ(invert[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::f - 12);
    assertEQ(invert[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::e - 12);
    assertEQ(invert[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::d - 12);
}

static void testInvertC()
{
    // try in C Major
    // rotate on the c
    int axis = 0;
    std::vector<int> invert = DiatonicUtils::getInvert(axis, DiatonicUtils::c, DiatonicUtils::Modes::Major);
    DiatonicUtils::_dumpTransposes("just generated C maj", invert);
    assertInvertValidInC(invert, axis);

    assertEQ(invert[DiatonicUtils::c] + DiatonicUtils::c, DiatonicUtils::c);
    assertEQ(invert[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::b - 12);
    assertEQ(invert[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::a - 12);
    assertEQ(invert[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::g - 12);
    assertEQ(invert[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::f - 12);
    assertEQ(invert[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::e - 12);
    assertEQ(invert[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::d - 12);
}


static void testInvertC2()
{
    // try in C Major
    // rotate on the D
    int axis = 2;
    std::vector<int> invert = DiatonicUtils::getInvert(axis, DiatonicUtils::c, DiatonicUtils::Modes::Major);
    DiatonicUtils::_dumpTransposes("just generated C maj", invert);
    assertInvertValidInC(invert, axis);

    assertEQ(invert[DiatonicUtils::c] + DiatonicUtils::c, DiatonicUtils::e);
    assertEQ(invert[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::d);
    assertEQ(invert[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::c);
    assertEQ(invert[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::b-12);
    assertEQ(invert[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::a-12);
    assertEQ(invert[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::g-12);
    assertEQ(invert[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::f-12);
}

static void testInvertD()
{
    // try in D Major
    // rotate on the d
    int axis = 1;
    std::vector<int> invert = DiatonicUtils::getInvert(axis, DiatonicUtils::d, DiatonicUtils::Modes::Major);
    DiatonicUtils::_dumpTransposes("just generated D maj", invert);
    assertInvertValidInC(invert, axis);

    assertEQ(invert[DiatonicUtils::d] + 0 , DiatonicUtils::d);
#if 0
    assertEQ(invert[DiatonicUtils::d] + DiatonicUtils::d, DiatonicUtils::b - 12);
    assertEQ(invert[DiatonicUtils::e] + DiatonicUtils::e, DiatonicUtils::a - 12);
    assertEQ(invert[DiatonicUtils::f] + DiatonicUtils::f, DiatonicUtils::g - 12);
    assertEQ(invert[DiatonicUtils::g] + DiatonicUtils::g, DiatonicUtils::f - 12);
    assertEQ(invert[DiatonicUtils::a] + DiatonicUtils::a, DiatonicUtils::e - 12);
    assertEQ(invert[DiatonicUtils::b] + DiatonicUtils::b, DiatonicUtils::d - 12);
#endif
}

void testDiatonicUtils()
{
    testIsNoteInC();
    testGetScaleDegreeInC();
    testqQuantizeXposeToScaleDegreeInC();
    testGetPitchFromScaleDegree();
    testTransposeC_Quantized();
    testTransposeC_Informed();
   
    testRelativeMajor();
    testTransposeC2();
    testTransposeAMinor();
    testTransposeEPhrygian();
    testTransposeDMajor();
    testTransposeEFlat();
    testTransposeLambdaSemi();

    testTransposeLambdaFifth();
    testTransposeLambdaDiatonicWhole();
    testTransposeLambdaDiatonicWholeOct();

    testInvertInC();
    testInvert();
    printf("put invert tests back\n");
    testInvertC();
    testInvertC2();
    testInvertD();
  //  testInvertLambdaDirection();
    
    

}