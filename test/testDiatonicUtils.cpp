
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

static void testInvertLambdaOctaves(bool constrain)
{
    const int axisSemitones = PitchUtils::cvToSemitone(0);
    auto lambda = DiatonicUtils::makeInvertLambda(
        axisSemitones,      //int invert axis semitions,
        constrain,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    int i = 0;
    note->pitchCV = float(i);
    lambda(note);
    assertEQ(note->pitchCV, -i);

    i = 1;
    note->pitchCV = float(i);
    lambda(note);
    assertEQ(note->pitchCV, -i);

    for (int i = -3; i < 4; ++i) {
        note->pitchCV = float(i);
        lambda(note);
        assertEQ(note->pitchCV, -i);
    }
}

static void testInvertLambdaOctaves()
{
    testInvertLambdaOctaves(false);
    testInvertLambdaOctaves(true);
}

static void testTransposeLambdaOctaves(bool constrain)
{
    auto lambda = DiatonicUtils::makeTransposeLambda(
        0,      //int transposeSemitones,
        constrain,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    for (int i = -3; i < 4; ++i) {
        note->pitchCV = float(i);
        lambda(note);
        assertEQ(note->pitchCV, i);
    }
}

static void testTransposeLambdaOctaves()
{
    testTransposeLambdaOctaves(false);
    testTransposeLambdaOctaves(true);
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


static void testGetScaleDegree()
{

    // C major 
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::c, DiatonicUtils::c, DiatonicUtils::Modes::Major), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::c_, DiatonicUtils::c, DiatonicUtils::Modes::Major), -1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d, DiatonicUtils::c, DiatonicUtils::Modes::Major), 1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d_, DiatonicUtils::c, DiatonicUtils::Modes::Major), -1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::e, DiatonicUtils::c, DiatonicUtils::Modes::Major), 2);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::f, DiatonicUtils::c, DiatonicUtils::Modes::Major), 3);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::f_, DiatonicUtils::c, DiatonicUtils::Modes::Major), -1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g, DiatonicUtils::c, DiatonicUtils::Modes::Major), 4);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g_, DiatonicUtils::c, DiatonicUtils::Modes::Major), -1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a, DiatonicUtils::c, DiatonicUtils::Modes::Major), 5);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a_, DiatonicUtils::c, DiatonicUtils::Modes::Major), -1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::b, DiatonicUtils::c, DiatonicUtils::Modes::Major), 6);

    // first note in other major scales
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d, DiatonicUtils::d, DiatonicUtils::Modes::Major), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d_, DiatonicUtils::d, DiatonicUtils::Modes::Major), -1);

    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::b, DiatonicUtils::b, DiatonicUtils::Modes::Major), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::c, DiatonicUtils::b, DiatonicUtils::Modes::Major), -1);

    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a_, DiatonicUtils::a_, DiatonicUtils::Modes::Major), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::b, DiatonicUtils::a_, DiatonicUtils::Modes::Major), -1);


    // a few other minor scale things
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::e, DiatonicUtils::d, DiatonicUtils::Modes::Major), 1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::f_, DiatonicUtils::d, DiatonicUtils::Modes::Major), 2);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g, DiatonicUtils::d, DiatonicUtils::Modes::Major), 3);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a, DiatonicUtils::d, DiatonicUtils::Modes::Major), 4);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::b, DiatonicUtils::d, DiatonicUtils::Modes::Major), 5);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::c_, DiatonicUtils::d, DiatonicUtils::Modes::Major), 6);


    // simple modes (whose relative major is C
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d, DiatonicUtils::d, DiatonicUtils::Modes::Dorian), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::e, DiatonicUtils::e, DiatonicUtils::Modes::Phrygian), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::f, DiatonicUtils::f, DiatonicUtils::Modes::Lydian), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g, DiatonicUtils::g, DiatonicUtils::Modes::Mixolydian), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::b, DiatonicUtils::b, DiatonicUtils::Modes::Locrian), 0);

    // AMinor (relative minor of c)
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::b, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::c, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 2);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 3);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::e, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 4);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::f, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 5);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g, DiatonicUtils::a, DiatonicUtils::Modes::Minor), 6);

    // C minor
#if 0
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::c, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 0);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 1);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::d_, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 2);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::f, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 3);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 4);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::g_, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 5);
    assertEQ(DiatonicUtils::getScaleDegree(DiatonicUtils::a_, DiatonicUtils::c, DiatonicUtils::Modes::Minor), 6);
#endif





   

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

static void testgetPitchFromScaleDegreeInC()
{
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(0), 0);         // unison
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(1), 2);         // maj 2
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(2), 4);         //maj 3
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(3), 5);         // 4th
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(4), 7);
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(5), 9);         // maj 6

    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(6), 11);
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(7), 12);
    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(8), 14);

    assertEQ(DiatonicUtils::getPitchFromScaleDegreeInC(9), 16);
}

static void testPitchFromScaleDegree()
{
    // CMaj
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, 0, DiatonicUtils::Modes::Major), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, 0, DiatonicUtils::Modes::Major), DiatonicUtils::d);

    // Other Maj
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::d, DiatonicUtils::Modes::Major), DiatonicUtils::d);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::d, DiatonicUtils::Modes::Major), DiatonicUtils::a);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::g, DiatonicUtils::Modes::Major), DiatonicUtils::g);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::g, DiatonicUtils::Modes::Major), DiatonicUtils::b);

    // Non-major related
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::a, DiatonicUtils::Modes::Minor), DiatonicUtils::a);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::a, DiatonicUtils::Modes::Minor), DiatonicUtils::b);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::a, DiatonicUtils::Modes::Minor), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::a, DiatonicUtils::Modes::Minor), DiatonicUtils::d);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::a, DiatonicUtils::Modes::Minor), DiatonicUtils::e);

    //
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::d);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::d_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::f);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::g);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::g_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6, DiatonicUtils::c, DiatonicUtils::Modes::Minor), DiatonicUtils::a_);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::d);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::e);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::f);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::g);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::a);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6, DiatonicUtils::c, DiatonicUtils::Modes::Mixolydian), DiatonicUtils::a_);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::d);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::d_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::f);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::g);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::a);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6, DiatonicUtils::c, DiatonicUtils::Modes::Dorian), DiatonicUtils::a_);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::c_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::d_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::f);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::g);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::g_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6, DiatonicUtils::c, DiatonicUtils::Modes::Phrygian), DiatonicUtils::a_);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::d);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::e);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::f_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::g);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::a);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6, DiatonicUtils::c, DiatonicUtils::Modes::Lydian), DiatonicUtils::b);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(0, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::c);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(1, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::c_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(2, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::d_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(3, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::f);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(4, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::f_);

    assertEQ(DiatonicUtils::getPitchFromScaleDegree(5, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::g_);
    assertEQ(DiatonicUtils::getPitchFromScaleDegree(6, DiatonicUtils::c, DiatonicUtils::Modes::Locrian), DiatonicUtils::a_);

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

static void testInvertLambdaChromatic()
{
    // let axis be zero volts
    int axis = PitchUtils::cvToSemitone(0);
    auto lambda = DiatonicUtils::makeInvertLambda(
        axis,
        false,  //bool constrainToKeysig,
        5, DiatonicUtils::Modes::Major);     

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertEQ(note->pitchCV, 0);

    note->pitchCV = 0 + PitchUtils::semitone;
    lambda(note);
    assertEQ(note->pitchCV, -PitchUtils::semitone);

    note->pitchCV = 1 + 3 * PitchUtils::semitone;
    lambda(note);
    assertEQ(note->pitchCV, -(1 + 3 * PitchUtils::semitone));
}

static void testInvertLambdaChromatic2()
{
    // let axis be zero volts
    float axisCV = 4 * PitchUtils::semitone;
    int axis = PitchUtils::cvToSemitone(axisCV);
    auto lambda = DiatonicUtils::makeInvertLambda(
        axis,
        false,  //bool constrainToKeysig,
        5, DiatonicUtils::Modes::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = axisCV;
    lambda(note);
    assertClose(note->pitchCV, axisCV, .001);


    note->pitchCV = axisCV + PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, axisCV -PitchUtils::semitone, .001);

    note->pitchCV = axisCV + 1 + 3 * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV,  axisCV -(1 + 3 * PitchUtils::semitone), .001);
}

static void testInvertLambdaDirection(bool constrain)
{
    int axis = 0;
    auto lambda = DiatonicUtils::makeInvertLambda(
        axis,
        constrain,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);        // c major

    float lastPitch = 10000;
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    for (int i = -40; i < 40; ++i) {
        note->pitchCV = PitchUtils::semitoneToCV(i);
        lambda(note);
        if (constrain) {
            assert(note->pitchCV <= (lastPitch + .0001));         // can be equal in constrain keysig
        } else {
            assert(note->pitchCV < lastPitch);
        }
        lastPitch = note->pitchCV;
    }
}

static void testInvertLambdaDirection()
{
    testInvertLambdaDirection(true);
    testInvertLambdaDirection(false);
}

static void testInvertLambdaSanity(bool constrain, int semitoneAxis, int rootKey, DiatonicUtils::Modes mode)
{
    auto lambda = DiatonicUtils::makeInvertLambda(
        semitoneAxis,
        constrain,
        rootKey, mode);

    float lastPitch = 10000;
    bool firstNote = true;
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    printf("put this back to -40\n !!!!!!!!!!!!");
    for (int i = 0; i < 40; ++i) {

        // Is this a strange offset?
        note->pitchCV = PitchUtils::semitoneToCV(i);

        lambda(note);
        if (!firstNote) {
            if (constrain) {
                bool isOk = false;
                const float deltaFromOneSemitoneDown = note->pitchCV - (lastPitch - PitchUtils::semitone);
                if (std::abs(deltaFromOneSemitoneDown) < .001) {
                    printf("is one semi down\n");
                    isOk = true;
                }
                const float deltaFromTwoSemitoneDown = note->pitchCV - (lastPitch - 2 * PitchUtils::semitone);
                if (std::abs(deltaFromTwoSemitoneDown) < .001) {
                    printf("is two semi down\n");
                    isOk = true;
                }
                const float deltaFromSamePitch = note->pitchCV - lastPitch;
                if (std::abs(deltaFromSamePitch) < .001) {
                    printf("is same\n");            // not sure I'm crazy about this, but for now I'll take it.
                    isOk = true;
                }

                // this is failing at axis 0, c maj (the simplest case!);
                assert(isOk);

               //assertClose(note->pitchCV, lastPitch - PitchUtils::semitone, .001);
                //assert()
               // assert(note->pitchCV <= lastPitch);         // can be equal in constrain keysig
            } else {
               // assert(note->pitchCV < lastPitch);
                assertClose(note->pitchCV, lastPitch - PitchUtils::semitone, .001);
            }
        }
        firstNote = false;
        lastPitch = note->pitchCV;
    }
}

static void testInvertLambdaSanity(bool constrain, int semitoneAxis)
{
    // spot check some keysigs
    testInvertLambdaSanity(constrain, semitoneAxis, DiatonicUtils::c, DiatonicUtils::Modes::Major);
    testInvertLambdaSanity(constrain, semitoneAxis, DiatonicUtils::c, DiatonicUtils::Modes::Minor);
    testInvertLambdaSanity(constrain, semitoneAxis, DiatonicUtils::f_, DiatonicUtils::Modes::Mixolydian);
}

static void testInvertLambdaSanity(bool constrain)
{
    printf("put this back to -10 !!!!!\n");
    for (int i = 0; i < 100; ++i) {
        testInvertLambdaSanity(constrain, i);
    }
}

static void testInvertLambdaSanity()
{
    testInvertLambdaSanity(false);
    testInvertLambdaSanity(true);
}


static void testInvertLambdaC()
{
    // let axis be zero volts (C4)
    // invert in c maj
 
    const int axisSemitones = PitchUtils::cvToSemitone(0);
    auto lambda = DiatonicUtils::makeInvertLambda(
        axisSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);


    // C -> C
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertEQ(note->pitchCV, 0);

    // C# -> B
    printf("finish testInvertLambdaC with chromatic ppitches\n");
    note->pitchCV = DiatonicUtils::c_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

    // D -> B
    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

    //new case
    // D# -> A
    note->pitchCV = DiatonicUtils::d_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a * PitchUtils::semitone - 1, .0001);

    // E -> A
    note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::g * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::f* PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::a * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::e * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::b * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone - 1, .0001);

    note->pitchCV = 1;
    lambda(note);
    assertClose(note->pitchCV, -1, .0001);
}


static void testInvertLambdaCMinor()
{
    printf("\n\n !!!!! enter testInvertLambdaCMinor\n");

    const int axisSemitones = PitchUtils::cvToSemitone(0);
    auto lambda = DiatonicUtils::makeInvertLambda(
        axisSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Minor);


    // C -> C
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertEQ(note->pitchCV, 0);            // flip around 0

    // C# -> B
    note->pitchCV = DiatonicUtils::c_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

     // D -> B flat
    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a_ * PitchUtils::semitone - 1, .0001);


}

// failing test case from general test.
// key cminor with axis 0.
// that test is failing at note 3.
static void testInvertLambdaCAxis0()
{
    printf("\n\n !!!!! enter testInvertLambdaCAxis0\n");
    // let axis be zero volts (C4)
    // invert in c maj

    const int axisSemitones = 0;
    const float axisCV = PitchUtils::semitoneToCV(axisSemitones);
    auto lambda = DiatonicUtils::makeInvertLambda(
        axisSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Minor);


    assertEQ(axisCV, -4);
    // C -> C
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertEQ(note->pitchCV, -8);            // flip around -4

    // C# -> B
    note->pitchCV = DiatonicUtils::c_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 9, .0001);

    // D -> B
    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a_ * PitchUtils::semitone - 9, .0001);

    //new case
    // D# -> g#
    note->pitchCV = DiatonicUtils::d_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g_ * PitchUtils::semitone - 9, .0001);

    // finish these, if we care to
#if 0

    // E -> G
    note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone - 9, .0001);

    note->pitchCV = DiatonicUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::g * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::f* PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::a * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::e * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::b * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone - 1, .0001);
#endif

    note->pitchCV = 1;
    lambda(note);
    assertClose(note->pitchCV, -9, .0001);
}

static void testInvertLambdaCAllAxis()
{
    int invertAxisSemitones = PitchUtils::cvToSemitone(0) + DiatonicUtils::c;

    // axis C
    auto lambda = DiatonicUtils::makeInvertLambda(
        invertAxisSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertEQ(note->pitchCV, 0);

    printf("PUT BACK C# test\n");
    note->pitchCV = DiatonicUtils::c_ * PitchUtils::semitone;
    lambda(note);
    //assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

    //****************************** axis D

    invertAxisSemitones = PitchUtils::cvToSemitone(0) + DiatonicUtils::d;
    lambda = DiatonicUtils::makeInvertLambda(
        invertAxisSemitones,
        true,  //bool constrainToKeysig,
        0, DiatonicUtils::Modes::Major);

    note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = DiatonicUtils::c * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::e * PitchUtils::semitone, .0001);


    printf("PUT BACK C# test\n");
    note->pitchCV = DiatonicUtils::c_ * PitchUtils::semitone;
    lambda(note);
    //assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone, .0001);

    printf("finish the all axis test\n");
   
}

void testDiatonicUtils()
{
    testIsNoteInC();
    testGetScaleDegreeInC();
    testGetScaleDegree();
    testqQuantizeXposeToScaleDegreeInC();
    testgetPitchFromScaleDegreeInC();
    testPitchFromScaleDegree();
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
    testTransposeLambdaOctaves();


    testInvertLambdaChromatic();
    testInvertLambdaChromatic2();

    printf("put back all the invert tests\n");

    testInvertLambdaC();
    testInvertLambdaCMinor();
    testInvertLambdaCAxis0();
    testInvertLambdaOctaves();
    testInvertLambdaCAllAxis();
    testInvertLambdaSanity();

    printf("put back test testInvertLambdaDirection\n");
    testInvertLambdaDirection();

    
    printf("***** Make some lambda tests for octaves!\n");

}