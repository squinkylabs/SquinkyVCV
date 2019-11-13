
#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"
#include "SqMidiEvent.h"

#include "asserts.h"

static void testGetScaleRelativeNote1()
{
    // C Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    assert(p->getScaleRelativeNote(0)->valid);  // C
    assert(!p->getScaleRelativeNote(1)->valid);  // C#
    assert(p->getScaleRelativeNote(2)->valid);  // D
    assert(!p->getScaleRelativeNote(3)->valid);  // D#
    assert(p->getScaleRelativeNote(4)->valid);  // E
    assert(p->getScaleRelativeNote(5)->valid);  // F
    assert(!p->getScaleRelativeNote(6)->valid);  // F#
    assert(p->getScaleRelativeNote(7)->valid);  // G
    assert(!p->getScaleRelativeNote(8)->valid);  // G#
    assert(p->getScaleRelativeNote(9)->valid);  // A
    assert(!p->getScaleRelativeNote(10)->valid);  // A#
    assert(p->getScaleRelativeNote(11)->valid);  // B
}


static void testGetScaleRelativeNote2()
{
    // G Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);
    assert(p->getScaleRelativeNote(PitchUtils::g)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::g_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::a)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::a_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::b)->valid);

    // I'm pretty sure this C case will be in wrong octave
    assert(p->getScaleRelativeNote(PitchUtils::c)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::c_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::d)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::d_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::e)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::f)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::f_)->valid);
}


static void testGetScaleRelativeNote4()
{
    // G Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);

    auto srn = p->getScaleRelativeNote(PitchUtils::g);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 0);

    srn = p->getScaleRelativeNote(PitchUtils::g_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::a);
    assert(srn->valid);
    assertEQ(srn->degree, 1);
    assertEQ(srn->octave, 0);

    srn = p->getScaleRelativeNote(PitchUtils::a_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::b);
    assert(srn->valid);
    assertEQ(srn->degree, 2);
    assertEQ(srn->octave, 0);

    // wrap around
    srn = p->getScaleRelativeNote(PitchUtils::c);
    assert(srn->valid);
    assertEQ(srn->degree, 3);
    assertEQ(srn->octave, -1);

    srn = p->getScaleRelativeNote(PitchUtils::c_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::d);
    assert(srn->valid);
    assertEQ(srn->degree, 4);
    assertEQ(srn->octave, -1);

    srn = p->getScaleRelativeNote(PitchUtils::d_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::e);
    assert(srn->valid);
    assertEQ(srn->degree, 5);
    assertEQ(srn->octave, -1);

    srn = p->getScaleRelativeNote(PitchUtils::f);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::f_);
    assert(srn->valid);
    assertEQ(srn->degree, 6);
    assertEQ(srn->octave, -1);
}


static void testGetScaleRelativeNote3()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    auto srn = p->getScaleRelativeNote(0);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 0);

    srn = p->getScaleRelativeNote(12);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 1);

    srn = p->getScaleRelativeNote(48);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 4);
}

static void testGetSemitone1()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    assertEQ(p->getSemitone(ScaleRelativeNote(0, 0, p)), 0);
    assertEQ(p->getSemitone(ScaleRelativeNote(1, 0, p)), 2);
    assertEQ(p->getSemitone(ScaleRelativeNote(2, 0, p)), 4);
    assertEQ(p->getSemitone(ScaleRelativeNote(3, 0, p)), 5);
    assertEQ(p->getSemitone(ScaleRelativeNote(4, 0, p)), 7);
    assertEQ(p->getSemitone(ScaleRelativeNote(5, 0, p)), 9);
    assertEQ(p->getSemitone(ScaleRelativeNote(6, 0, p)), 11);
}

static void testRoundTrip(ScalePtr scale, int semitone)
{
    ScaleRelativeNotePtr srn = scale->getScaleRelativeNote(semitone);
    if (srn->valid) {
        // if the note is in key
        int semi2 = scale->getSemitone(*srn);
        assertEQ(semi2, semitone);
    }
}

static void testRoundTrip(ScalePtr scale)
{
    for (int i=0; i< 100; ++i) {
        testRoundTrip(scale, i);
    }
}

static void testRoundTrip(Scale::Scales scale)
{
    for (int i=0; i<12; ++i) {
        auto scaleP = Scale::getScale(scale, i);
        testRoundTrip(scaleP);
    }
}
static void testRoundTrip()
{
    testRoundTrip(Scale::Scales::Major);
}

static void testRTBugCases()
{
    {
        auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
        {
            ScaleRelativeNotePtr srn = p->getScaleRelativeNote(0);
            int semi = p->getSemitone(*srn);
            assertEQ(semi, 0);
        }

        {
            ScaleRelativeNotePtr srn2 = p->getScaleRelativeNote(12);
            int semi2 = p->getSemitone(*srn2);
            assertEQ(semi2, 12);
        }
    }

    {
        auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
        ScaleRelativeNotePtr srn = p->getScaleRelativeNote(0);
        int semi = p->getSemitone(*srn);
        assertEQ(semi, 0);
    }

}

static void testGetSemitone2()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);
    assertEQ(p->getSemitone(ScaleRelativeNote(0, 0, p)), PitchUtils::g + 0);
    assertEQ(p->getSemitone(ScaleRelativeNote(1, 0, p)), PitchUtils::g + 2);
    assertEQ(p->getSemitone(ScaleRelativeNote(2, 0, p)), PitchUtils::g + 4);
    assertEQ(p->getSemitone(ScaleRelativeNote(3, 0, p)), PitchUtils::g + 5);
    assertEQ(p->getSemitone(ScaleRelativeNote(4, 0, p)), PitchUtils::g + 7);
    assertEQ(p->getSemitone(ScaleRelativeNote(5, 0, p)), PitchUtils::g + 9);
    assertEQ(p->getSemitone(ScaleRelativeNote(6, 0, p)), PitchUtils::g + 11);
}

static void testMinor()
{
    // E min octave 0
    auto p = Scale::getScale(Scale::Scales::Minor, PitchUtils::e);
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(!p->getScaleRelativeNote(5)->valid);  // f
    assert(p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
}


static void testPhrygian()
{
    auto p = Scale::getScale(Scale::Scales::Phrygian, PitchUtils::e);
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
}

static void testMixo()
{
    auto p = Scale::getScale(Scale::Scales::Mixolydian, PitchUtils::g);
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
    assert(p->getScaleRelativeNote(16)->valid);  // e
    assert(p->getScaleRelativeNote(17)->valid);  // f
    assert(!p->getScaleRelativeNote(18)->valid);  // f#
}


static void testDorian()
{
    auto p = Scale::getScale(Scale::Scales::Dorian, PitchUtils::d);
    assert(p->getScaleRelativeNote(2)->valid);  // D
    assert(!p->getScaleRelativeNote(3)->valid);  // d#
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
}


static void testLydian()
{
    auto p = Scale::getScale(Scale::Scales::Lydian, PitchUtils::f);
    assert(p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
    assert(p->getScaleRelativeNote(16)->valid);  // e
}


static void testLocrian()
{
    auto p = Scale::getScale(Scale::Scales::Locrian, PitchUtils::b);
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
    assert(p->getScaleRelativeNote(16)->valid);  // e
    assert(p->getScaleRelativeNote(17)->valid);  // f
    assert(!p->getScaleRelativeNote(18)->valid);  // f#
    assert(p->getScaleRelativeNote(19)->valid);  // g
    assert(!p->getScaleRelativeNote(20)->valid);  // g#
    assert(p->getScaleRelativeNote(21)->valid);  // a
    assert(!p->getScaleRelativeNote(22)->valid);  // a#
}


static void testPentatonic()
{
    auto p = Scale::getScale(Scale::Scales::MinorPentatonic, PitchUtils::e);
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(!p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(!p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
}

static void testTransposeInScale1()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    int xpose = p->transposeInScale(PitchUtils::c, 1);
    assertEQ(xpose, PitchUtils::d);
    xpose = p->transposeInScale(PitchUtils::b, 1);
    assertEQ(xpose, PitchUtils::c + 12);
}

static void testInvertInScale1()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
  
    // root, in octave 4
    int inversionDegree = 0 + p->degreesInScale() * 4;

    // input in octave 4;
    int semitone = PitchUtils::c + 12 * 4;

    int invert = p->invertInScale(semitone, inversionDegree);
    assertEQ(invert, semitone);

    int expectedInvert = semitone - 1;
    semitone++;     // c# 4
    invert = p->invertInScale(semitone, inversionDegree);
    assertEQ(invert, expectedInvert);
}



static void testInvertInScale15()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    // root, in octave 4
    int inversionDegree = 0 + p->degreesInScale() * 4;

    int semitone = PitchUtils::c + 12 * 4;
    int expectedInvert = semitone;

    // c->c
    int invert = p->invertInScale(semitone, inversionDegree);
    assertEQ(invert, expectedInvert);

    // d->b
    semitone += 2;
    expectedInvert -= 1;
    invert = p->invertInScale(semitone, inversionDegree);
    assertEQ(invert, expectedInvert);


    printf("write more cases for testInvertInScale15\n");
}

static void testInvertInScaleOctaves()
{
    auto scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    // root, in octave 0
    float cvAll = 0;
    int semitonesAll = PitchUtils::cvToSemitone(cvAll);
    
    auto srnAll = scale->getScaleRelativeNote(semitonesAll);

    int inversionDegree = scale->octaveAndDegree(srnAll->octave, srnAll->degree);
    //int semitone = PitchUtils::c;
    int expectedInvert = semitonesAll;

     /**
     * Input and output are regular chromatic semitones,
     * But transpose will be done scale relative
     */
    //   int invertInScale(int semitone, int inversionDegree);
    int invert = scale->invertInScale(semitonesAll, inversionDegree);
    assertEQ(invert, expectedInvert);
  
}

static void testTransposeInScale2()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    int xpose = p->transposeInScale(PitchUtils::c_, 1);
    assertEQ(xpose, PitchUtils::d_);

    xpose = p->transposeInScale(PitchUtils::a_, 1);
    assertEQ(xpose, PitchUtils::b);
  
}

static void testTransposeInScalePentatonic()
{
    auto p = Scale::getScale(Scale::Scales::MinorPentatonic, PitchUtils::e);

    
    int xpose = p->transposeInScale(PitchUtils::e, 1);
    assertEQ(xpose, PitchUtils::g);

    xpose = p->transposeInScale(PitchUtils::g, 1);
    assertEQ(xpose, PitchUtils::a);

    xpose = p->transposeInScale(PitchUtils::f, 1);
    assertEQ(xpose, PitchUtils::g_);

    assert(false);
}

static void testTransposeLambdaSemi()
{
    // chromatic, one semi
    auto lambdaSemi = Scale::makeTransposeLambdaChromatic(1);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;

    lambdaSemi(note);
    float x = note->pitchCV;
    assertClose(x, PitchUtils::semitone, .00001);

    note->pitchCV = 1;
    lambdaSemi(note);
    assertClose(note->pitchCV, 1 + PitchUtils::semitone, .00001);
}



static void testTransposeLambdaFifth()
{
    // chromatic, one fifth
    auto lambda = Scale::makeTransposeLambdaChromatic(7);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertClose(note->pitchCV, 7 * PitchUtils::semitone, .00001);
    note->pitchCV = 1;
    lambda(note);
    assertClose(note->pitchCV, 1 + 7 * PitchUtils::semitone, .00001);
}


static void testTransposeLambdaDiatonicWhole()
{
    auto  lambdaDiatonicWhole = Scale::makeTransposeLambdaScale(1, PitchUtils::c,  Scale::Scales::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    //c -> d
    note->pitchCV = 0;
    lambdaDiatonicWhole(note);
    assertClose(note->pitchCV, PitchUtils::d * PitchUtils::semitone, .00001);

    // e -> f
    note->pitchCV = PitchUtils::e * PitchUtils::semitone;
    lambdaDiatonicWhole(note);
    assertClose(note->pitchCV, PitchUtils::f * PitchUtils::semitone, .00001);
}


static void testTransposeLambdaDiatonicWholeOct()
{
    const int degrees = 2 * 7 + 1;      // two octaves and a step
    auto  lambda  = Scale::makeTransposeLambdaScale(degrees, PitchUtils::c, Scale::Scales::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    //c -> d
    note->pitchCV = 0;
    lambda(note);
    assertClose(note->pitchCV, 2 + PitchUtils::d * PitchUtils::semitone, .00001);

    // e -> f
    note->pitchCV = PitchUtils::e * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, 2 + PitchUtils::f * PitchUtils::semitone, .00001);
}


static void testTransposeLambdaOctavesChromatic()
{
    auto  lambda = Scale::makeTransposeLambdaChromatic(0);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    for (int i = -3; i < 4; ++i) {
        note->pitchCV = float(i);
        lambda(note);
        assertEQ(note->pitchCV, i);
    }
}

static void testTransposeLambdaOctaves()
{
    auto  lambda = Scale::makeTransposeLambdaScale(0, PitchUtils::c, Scale::Scales::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    for (int i = -3; i < 4; ++i) {
        note->pitchCV = float(i);
        lambda(note);
        assertEQ(note->pitchCV, i);
    }
}

static void testTransposeLambdaCMinor()
{ 
    auto  lambda = Scale::makeTransposeLambdaScale(2, PitchUtils::c, Scale::Scales::Minor);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;

    note->pitchCV = PitchUtils::c * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::d_ * PitchUtils::semitone, .00001);

    note->pitchCV = PitchUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::f * PitchUtils::semitone, .00001);

    note->pitchCV = PitchUtils::d_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::g * PitchUtils::semitone, .00001);

    note->pitchCV = PitchUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::g_ * PitchUtils::semitone, .00001);

    note->pitchCV = PitchUtils::g * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::a_ * PitchUtils::semitone, .00001);

    note->pitchCV = PitchUtils::g_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::c * PitchUtils::semitone + 1, .00001);

    note->pitchCV = PitchUtils::a_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::d * PitchUtils::semitone + 1, .00001);
}

static void testInvertLambdaChromatic()
{
    // let axis be zero volts
    int axis = PitchUtils::cvToSemitone(0);
    auto lambda = Scale::makeInvertLambdaChromatic(axis);
      //  axis,
      //  false,  //bool constrainToKeysig,
     //   5, DiatonicUtils::Modes::Major);     

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
   // auto lambda = DiatonicUtils::makeInvertLambda(
   //     axis,
   //     false,  //bool constrainToKeysig,
   //     5, DiatonicUtils::Modes::Major);

    auto lambda = Scale::makeInvertLambdaChromatic(axis);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = axisCV;
    lambda(note);
    assertClose(note->pitchCV, axisCV, .001);


    note->pitchCV = axisCV + PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, axisCV - PitchUtils::semitone, .001);

    note->pitchCV = axisCV + 1 + 3 * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, axisCV - (1 + 3 * PitchUtils::semitone), .001);
}


static void testInvertLambdaC()
{
    /**
    int semitonesAll = PitchUtils::cvToSemitone(cvAll);

    auto srnAll = scale->getScaleRelativeNote(semitonesAll);

    int inversionDegree = scale->octaveAndDegree(srnAll->octave, srnAll->degree);
    */


    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    const int semitoneAxis = PitchUtils::cvToSemitone(0);
    auto srnAll = scale->getScaleRelativeNote(semitoneAxis);
    const int inversionDegree = scale->octaveAndDegree(srnAll->octave, srnAll->degree);

    auto lambda = Scale::makeInvertLambdaDiatonic(inversionDegree, PitchUtils::c, Scale::Scales::Major);


    // C -> C
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = 0;
    lambda(note);
    assertEQ(note->pitchCV, 0);

    // C# -> B
    note->pitchCV = PitchUtils::c_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::b * PitchUtils::semitone - 1, .0001);

    // D -> B
    note->pitchCV = PitchUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::b * PitchUtils::semitone - 1, .0001);

    //new case (fails)
    // D# -> A
    note->pitchCV = PitchUtils::d_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::a_ * PitchUtils::semitone - 1, .0001);

    // E -> A
    note->pitchCV = PitchUtils::e * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::a * PitchUtils::semitone - 1, .0001);

    note->pitchCV = PitchUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::g * PitchUtils::semitone - 1, .0001);

    note->pitchCV = PitchUtils::g * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::f* PitchUtils::semitone - 1, .0001);

    note->pitchCV = PitchUtils::a * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::e * PitchUtils::semitone - 1, .0001);

    note->pitchCV = PitchUtils::b * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, PitchUtils::d * PitchUtils::semitone - 1, .0001);

    note->pitchCV = 1;
    lambda(note);
    assertClose(note->pitchCV, -1, .0001);
}


static void testDegreeUtils()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    assertEQ(scale->degreesInScale(), 7);

    auto norm = scale->normalizeDegree(4);
    assertEQ(norm.first, 0);
    assertEQ(norm.second, 4);

    norm = scale->normalizeDegree(0);
    assertEQ(norm.first, 0);
    assertEQ(norm.second, 0);

    norm = scale->normalizeDegree(7);
    assertEQ(norm.first, 1);
    assertEQ(norm.second, 0);

    norm = scale->normalizeDegree(7+5);
    assertEQ(norm.first, 1);
    assertEQ(norm.second, 5);

    // down to b
    norm = scale->normalizeDegree(-1);
    assertEQ(norm.first, -1);
    assertEQ(norm.second, 6);


    assertEQ(scale->octaveAndDegree(0, 0), 0);
    assertEQ(scale->octaveAndDegree(0, 3), 3);
    assertEQ(scale->octaveAndDegree(1, 0), 7);
    assertEQ(scale->octaveAndDegree(1, 1), 8);
    assertEQ(scale->octaveAndDegree(3, 0), 3*7);
}


void testScale()
{
    testDegreeUtils();
    testGetScaleRelativeNote1();
    testGetScaleRelativeNote2();
    testGetScaleRelativeNote3();
    testGetScaleRelativeNote4();
    testGetSemitone1();
    testGetSemitone2();

    testRTBugCases();
    testRoundTrip();

    testMinor();
    testPhrygian();
    testMixo();
    testDorian();
    testLydian();
    testLocrian();
    testPentatonic();

    testTransposeInScale1();
    testTransposeInScale2();
    testTransposeInScalePentatonic();

    testInvertInScale1();
    testInvertInScaleOctaves();

    testInvertInScale15();


    // these tests ported over from diatonic utils tests
    testTransposeLambdaSemi();

    testTransposeLambdaFifth();

    testTransposeLambdaDiatonicWhole();
    testTransposeLambdaDiatonicWholeOct();
    testTransposeLambdaOctaves();
    testTransposeLambdaOctavesChromatic();
    testTransposeLambdaCMinor();


    testInvertLambdaChromatic();
    testInvertLambdaChromatic2();
    testInvertLambdaC();
    //testInvertLambdaCMinor();
    //testInvertLambdaCAxis0();
    //testInvertLambdaOctaves();
    //testInvertLambdaCAllAxis();
    //testInvertLambdaSanity();
    //testInvertLambdaDirection();

}