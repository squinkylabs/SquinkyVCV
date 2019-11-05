#include "DiatonicUtils.h"
#include "SqMidiEvent.h"

#include "asserts.h"

std::function<void(MidiEventPtr)> DiatonicUtils::makeInvertLambdaChromatic(int invertAxisSemitones)
{ 
    const float axis = PitchUtils::semitoneToCV(invertAxisSemitones);
    return [axis](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->pitchCV = 2 * axis - note->pitchCV;
        }
    };
}




std::function<void(MidiEventPtr)> DiatonicUtils::makeInvertLambda(int invertAxisSemitones, bool constrainToKeysig, int keyRoot, Modes mode)
{
    if (constrainToKeysig) {
        return makeInvertLambdaDiatonic(invertAxisSemitones, keyRoot, mode);
    } else {
        return makeInvertLambdaChromatic(invertAxisSemitones);
    }
}

std::function<void(MidiEventPtr)> DiatonicUtils::makeInvertLambdaDiatonic(
    int invertAxisSemitones, int keyRoot, DiatonicUtils::Modes mode)
{
    auto axisPitch = DiatonicUtils::normalizePitch(invertAxisSemitones);
    int axisOctave = axisPitch.first;
    int axisSemitone = axisPitch.second;
  //  int axisDegree = DiatonicUtils::getScaleDegreeInC(axisSemitone);
    int axisDegree = DiatonicUtils::getScaleDegree(axisSemitone, keyRoot, mode);

    auto chromaticLambda = makeInvertLambdaChromatic(invertAxisSemitones);

    return [axisDegree, chromaticLambda, keyRoot, mode](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            auto pitch = PitchUtils::cvToPitch(note->pitchCV);
            int octave = pitch.first;
            int semitone = pitch.second;

           // assert(DiatonicUtils::isNoteInC(semitone));
          //  int degree = DiatonicUtils::getScaleDegreeInC(semitone);
            const int degree = DiatonicUtils::getScaleDegree(semitone, keyRoot, mode);
            if ((degree < 0) || (axisDegree < 0)) {
                chromaticLambda(note);
                return;
            }


            int destinationDegree = axisDegree * 2 - degree;
            destinationDegree = DiatonicUtils::normalizeDegree(destinationDegree);      // normalize degrees


           // int destinationPitchSemititones = DiatonicUtils::getPitchFromScaleDegreeInC(destinationDegree);
            int destinationPitchSemititones = DiatonicUtils::getPitchFromScaleDegree(destinationDegree, keyRoot, mode);
          //  assert(destinationPitchSemititones == destinationPitchSemititones2);

            float destinationPitchCV = PitchUtils::semitoneToCV(destinationPitchSemititones);


            MidiNoteEventPtr temp = std::make_shared<MidiNoteEvent>();;
            temp->pitchCV = note->pitchCV;
            chromaticLambda(temp);
            // printf("inverted cv = %.2f chromatic = %.2f\n", destinationPitchCV, temp->pitchCV);
            const float chromaticResult = temp->pitchCV;

            const float lowerBound = (chromaticResult - 3 * PitchUtils::semitone);
            const float upperBound = (chromaticResult + 3 * PitchUtils::semitone);
            while (destinationPitchCV < lowerBound)
            {
                destinationPitchCV += 1.f;
            }
            while (destinationPitchCV > upperBound) {
                destinationPitchCV -= 1.f;
            }

          //  printf("  note in pitch = %.2f ", note->pitchCV);
            note->pitchCV = destinationPitchCV;
          //  printf("inverted to %.2f\n", note->pitchCV); fflush(stdout);
        }
    };
}


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