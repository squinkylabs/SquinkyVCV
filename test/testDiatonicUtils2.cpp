#include "DiatonicUtils.h"
#include "SqMidiEvent.h"

#include "asserts.h"

static void test0()
{
    auto lambda = DiatonicUtils::makeInvertLambdaDiatonic(0, 0, DiatonicUtils::Modes::Major);
}

static void testCMajAxis0()
{
   const int axisSemitones = PitchUtils::cvToSemitone(0);
   auto lambda = DiatonicUtils::makeInvertLambdaDiatonic(axisSemitones, DiatonicUtils::c, DiatonicUtils::Modes::Major);

   MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

   // C -> C
   note->pitchCV = 0;
   lambda(note);
   assertEQ(note->pitchCV, 0);

   // D -> B
   note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

   // E -> A
   note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::a * PitchUtils::semitone - 1, .0001);

    // F -> g
   note->pitchCV = DiatonicUtils::f * PitchUtils::semitone;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone - 1, .0001);

    // g->f
   note->pitchCV = DiatonicUtils::g * PitchUtils::semitone;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::f * PitchUtils::semitone - 1, .0001);

    // a->e
   note->pitchCV = DiatonicUtils::a * PitchUtils::semitone;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::e * PitchUtils::semitone - 1, .0001);

    // b -> d
   note->pitchCV = DiatonicUtils::b * PitchUtils::semitone;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone - 1, .0001);

   // c2 -> c2   note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
   note->pitchCV = DiatonicUtils::c * PitchUtils::semitone + 1;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::c * PitchUtils::semitone - 1, .0001);

    // c3 -> c3   note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
   note->pitchCV = DiatonicUtils::c * PitchUtils::semitone + 2;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::c * PitchUtils::semitone - 2, .0001);

    // c-1 -> c-1   note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
   note->pitchCV = DiatonicUtils::c * PitchUtils::semitone - 1;
   lambda(note);
   assertClose(note->pitchCV, DiatonicUtils::c * PitchUtils::semitone + 1, .0001);
}


static void testCMajAxis2()
{
    const int axisSemitones = PitchUtils::cvToSemitone(0) + DiatonicUtils::d;
    auto lambda = DiatonicUtils::makeInvertLambdaDiatonic(axisSemitones, DiatonicUtils::c, DiatonicUtils::Modes::Major);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    // C -> E
    note->pitchCV = DiatonicUtils::c * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::e * PitchUtils::semitone, .0001);

    // D -> D
    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone, .0001);

    // E -> C
    note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::c * PitchUtils::semitone, .0001);

     // F -> B
    note->pitchCV = DiatonicUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone - 1, .0001);

     // g->a
    note->pitchCV = DiatonicUtils::g * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a * PitchUtils::semitone - 1, .0001);

     // a->g
    note->pitchCV = DiatonicUtils::a * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone - 1, .0001);

     // b -> f
    note->pitchCV = DiatonicUtils::b * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::f * PitchUtils::semitone - 1, .0001);

    // d2 ->d02   
    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone + 1;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone - 1, .0001);
}

static void testAMinAxis0()
{
    const int axisSemitones = PitchUtils::cvToSemitone(0) + DiatonicUtils::a;
    auto lambda = DiatonicUtils::makeInvertLambdaDiatonic(axisSemitones, DiatonicUtils::a, DiatonicUtils::Modes::Minor);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    // A -> A
    note->pitchCV = DiatonicUtils::a * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a * PitchUtils::semitone, .0001);

    // B -> G
    note->pitchCV = DiatonicUtils::b * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone, .0001);

    // C -> F
    note->pitchCV = DiatonicUtils::c * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::f * PitchUtils::semitone + 1, .0001);

     // D -> E
    note->pitchCV = DiatonicUtils::d * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::e * PitchUtils::semitone + 1, .0001);

     // E->D
    note->pitchCV = DiatonicUtils::e * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::d * PitchUtils::semitone + 1, .0001);

     // f->c
    note->pitchCV = DiatonicUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::c * PitchUtils::semitone + 1, .0001);

     // g -> b
    note->pitchCV = DiatonicUtils::g * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::b * PitchUtils::semitone , .0001);

    // a2 ->d02   
    note->pitchCV = DiatonicUtils::a * PitchUtils::semitone + 1;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::a * PitchUtils::semitone - 1, .0001);

}


static void testCminorAxis0()
{

    const int axisSemitones = PitchUtils::cvToSemitone(0);
    auto lambda = DiatonicUtils::makeInvertLambdaDiatonic(axisSemitones, DiatonicUtils::c, DiatonicUtils::Modes::Minor);


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

     // e flat -> A flat
    note->pitchCV = DiatonicUtils::d_ * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g_ * PitchUtils::semitone - 1, .0001);

      // f -> g
    note->pitchCV = DiatonicUtils::f * PitchUtils::semitone;
    lambda(note);
    assertClose(note->pitchCV, DiatonicUtils::g * PitchUtils::semitone - 1, .0001);
   

}

void testDiatonicUtils2()
{
    test0();
    testCMajAxis0();
    testCMajAxis2();
    testAMinAxis0();
    testCminorAxis0();
}