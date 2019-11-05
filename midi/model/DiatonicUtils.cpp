#include "DiatonicUtils.h"
#include "SqMidiEvent.h"
#include "PitchUtils.h"
#include <assert.h>
#include <sstream>


std::pair<int, int> DiatonicUtils::normalizePitch(int pitch)
{
    int octave = pitch / 12;
    pitch -= octave * 12;
    if (pitch < 0) {
        pitch += 12;
        octave -= 1;
    }
    assert(pitch >= 0 && pitch < 12);
    return std::make_pair(octave, pitch);
}

bool DiatonicUtils::isNoteInC(int _pitch)
{
    auto normPitch = normalizePitch(_pitch);
    bool ret = true;
    assert(normPitch.second >= DiatonicUtils::c);
    assert(normPitch.second <= DiatonicUtils::b);
    switch (normPitch.second) {
        case DiatonicUtils::c:
        case DiatonicUtils::d:
        case DiatonicUtils::e:
        case DiatonicUtils::f:
        case DiatonicUtils::g:
        case DiatonicUtils::a:
        case DiatonicUtils::b:
            ret = true;
            break;
        default:
            ret = false;
    }
    return ret;
}

static std::vector<std::string> pitchNames = 
{
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

std::string DiatonicUtils::getPitchString(int _pitch)
{
    auto pitch = normalizePitch(_pitch);
    if (_pitch < -100) {
        return ("x");
    }
   
    std::stringstream s;
    s << pitch.first << ":" << pitchNames[pitch.second];
    return s.str();
}

std::vector<int> DiatonicUtils::getTransposedPitchesInC(const std::vector<int>& transposes)
{
    std::vector<int> ret(transposes.size());
    for (int i = 0; i < (int) transposes.size(); ++i) {
        const int j = transposes[i];
        ret[i] = (j < -1000) ? j : i + transposes[i];
    }

    return ret;
}

void DiatonicUtils::_dumpTransposes(const char* msg, const std::vector<int>& transposes)
{
    printf("\n*****dump1: %s\n", msg);
    std::vector<int> pitches = getTransposedPitchesInC(transposes);
#if 0   
    for (size_t i = 0; i < transposes.size(); ++i) {
        printf("[%d] transpose = %d, pitch=%d\n", (int)i, transposes[i], pitches[i]);
    }
#endif
   
    for (size_t i = 0; i < transposes.size(); ++i) {
        printf("t[%zd]=%d.  %s => %s\n", 
            i, transposes[i],
            getPitchString(int(i)).c_str(),
            getPitchString(pitches[i]).c_str());
    }
    printf("\n");
}


std::vector<int> DiatonicUtils::getTransposeInC(int amount, bool quantized)
{
    if (quantized) {
        return getTransposeInCQuantized(amount);
    } else {
        return getTransposeInCInformed(amount);
    }
}

std::vector<int> DiatonicUtils::getTransposeInCInformed(int _transposeAmount)
{
    auto normalizedTransposeAmount = normalizePitch(_transposeAmount);
    const int transposeSemis = normalizedTransposeAmount.second;
    const int transposeOctaves = normalizedTransposeAmount.first;

    assert(transposeSemis >= 0);
    assert(transposeSemis < 12);       // callers should normalize out the octaves. Or we should support it?
    std::vector<int> ret(12);

    // initialize to absurd value
    for (int i = 0; i < 12; ++i) {
        ret[i] = -24;
    }

    const int scaleDegreesOfTranspose = quantizeXposeToScaleDegreeInC(transposeSemis);
    // first do all the ones that are already in key
    for (int i = 0; i < 12; ++i) {
        int chromaticTransposePitch = i + transposeSemis;
        if (chromaticTransposePitch > DiatonicUtils::b) {
            chromaticTransposePitch -= 12;
        }
        assert(chromaticTransposePitch <= DiatonicUtils::b);

        const bool isInC = DiatonicUtils::isNoteInC(i);
        if (isInC) {
            // scale by number of degrees, not by chromatic value
            const int initialScaleDegree = getScaleDegreeInC(i);
            const int scaleDegreeAfterQuantize = initialScaleDegree + scaleDegreesOfTranspose;
            const int pitchAfterQuantize = getPitchFromScaleDegreeInC(scaleDegreeAfterQuantize);
            if (pitchAfterQuantize > 11) {
                auto norm = normalizePitch(pitchAfterQuantize);
            }
            const int delta = pitchAfterQuantize - i;
            ret[i] = delta;
        }
    }

    for (int i = 0; i < 12; ++i) {
        int chromaticTransposePitch = i + transposeSemis;
        if (chromaticTransposePitch > DiatonicUtils::b) {
            chromaticTransposePitch -= 12;
        }

        const bool isInC = DiatonicUtils::isNoteInC(i);
        const bool xposeInC = DiatonicUtils::isNoteInC(chromaticTransposePitch);

        if (!isInC) {
            ret[i] = transposeSemis;
        }
    }

    const int shift = transposeOctaves * 12;
    if (shift) {
        for (int i = 0; i < 12; ++i) {
            ret[i] += shift;
        }
    }
    return ret;
}

std::vector<int> DiatonicUtils::getTransposeInCQuantized(int _transposeAmount)
{
    auto normalizedTransposeAmount = normalizePitch(_transposeAmount);
    const int transposeSemis = normalizedTransposeAmount.second;
    const int transposeOctaves = normalizedTransposeAmount.first;

    assert(transposeSemis >= 0);
    assert(transposeSemis < 12);       // callers should normalize out the octaves. Or we should support it?
    std::vector<int> ret(12);


    // initialize to absurd value
    for (int i = 0; i < 12; ++i) {
        ret[i] = -24;
    }

    const int scaleDegreesOfTranspose = quantizeXposeToScaleDegreeInC(transposeSemis);
    // first do all the ones that are already in key
    for (int i = 0; i < 12; ++i) {
        int chromaticTransposePitch = i + transposeSemis;
        if (chromaticTransposePitch > DiatonicUtils::b) {
            chromaticTransposePitch -= 12;
           // printf("do we need to account for this octave at index %d?\n", i);
        }
        assert(chromaticTransposePitch <= DiatonicUtils::b);

        const bool isInC = DiatonicUtils::isNoteInC(i);
        if (isInC) {
            // scale by number of degrees, not by chromatic value
            const int initialScaleDegree = getScaleDegreeInC(i);
            const int scaleDegreeAfterQuantize = initialScaleDegree + scaleDegreesOfTranspose;
            const int pitchAfterQuantize = getPitchFromScaleDegreeInC(scaleDegreeAfterQuantize);

            if (pitchAfterQuantize > 11) {
                auto norm = normalizePitch(pitchAfterQuantize);
            }
            const int delta = pitchAfterQuantize - i;
            ret[i] = delta;
        }
    }

    // now do all the ones that are not in key
    for (int i = 0; i < 12; ++i) {
        int chromaticTransposePitch = i + transposeSemis;
        if (chromaticTransposePitch > DiatonicUtils::b) {
            chromaticTransposePitch -= 12;
           // printf("do we need to account for this octave?\n");
        }

        const bool isInC = DiatonicUtils::isNoteInC(i);
        const bool xposeInC = DiatonicUtils::isNoteInC(chromaticTransposePitch);

        if (!isInC) {
            assert(ret[i] < -12);                 // we haven't filled these in yet
        }

         // if chromatic xpose keeps in key, use that (for now)
        if (!isInC && xposeInC) {
            ret[i] = transposeSemis;

            assert(ret[i] >= transposeSemis - 1);
            assert(ret[i] <= transposeSemis + 1);
        }
        if (!isInC && !xposeInC) {
            assert(i > 0);

            // let's just go to the same pitch as prev guy (won't always work);
            const int prevXpose = ret[i - 1];
            const int prevPitch = prevXpose + (i - 1);
            const int thisPitch = prevPitch;
            const int thisXpose = thisPitch - i;
            ret[i] = thisXpose;

            assert(ret[i] >= transposeSemis - 1);
            assert(ret[i] <= transposeSemis + 1);
        }
    }

    const int shift = transposeOctaves * 12;
    if (shift) {
        for (int i = 0; i < 12; ++i) {
            ret[i] += shift;
        }
    }

    return ret;
}

int DiatonicUtils::getOffsetToRelativeMaj(Modes mode)
{
    int ret = 0;
    switch (mode) {
        case Modes::Major:
            ret = 0;
            break;
        case Modes::Dorian:
            ret = 10;
            break;
        case Modes::Phrygian:
            ret = 8;
            break;
        case Modes::Lydian:
            ret = 7;
            break;
        case Modes::Mixolydian:
            ret = 5;
            break;
        case Modes::Minor:
            ret = 3;
            break;
        case Modes::Locrian:
            ret = 1;
            break;
        default:
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInC(int degree)
{
    if (degree > 6) {
        const int x = getPitchFromScaleDegreeInC(degree - 7);
        return 12 + x;
    }
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;            // unison
            break;
        case 1:
            ret = 2;            // maj 2nd
            break;
        case 2:
            ret = 4;            // maj 3rd
            break;
        case 3:
            ret = 5;            // 4th
            break;
        case 4:
            ret = 7;            // 5th
            break;
        case 5:
            ret = 9;            // maj 6
            break;
        case 6:
            ret = 11;           // maj 7
            break;
        default:
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInCMinor(int degree)
{
    if (degree > 6) {
        const int x = getPitchFromScaleDegreeInCMinor(degree - 7);
        return 12 + x;
    }
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;            // unison
            break;
        case 1:
            ret = 2;            // maj 2nd
            break;
        case 2:
            ret = 3;            // min 3rd
            break;
        case 3:
            ret = 5;            // 4th
            break;
        case 4:
            ret = 7;            // 5th
            break;
        case 5:
            ret = 8;            // min 6
            break;
        case 6:
            ret = 10;           // min 7
            break;
        default:
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInCDorian(int degree)
{
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;
            break;
        case 1:
            ret = 2;
            break;
        case 2:         // flat 3rd
            ret = 3;
            break;
        case 3:
            ret = 5;
            break;
        case 4:
            ret = 7;
            break;
        case 5:
            ret = 9;        // maj 6
            break;
        case 6:
            ret = 10;       // flat 7
            break;
        default: 
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInCPhrygian(int degree)
{
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;
            break;
        case 1:
            ret = 1;
            break;
        case 2:
            ret = 3;
            break;
        case 3:
            ret = 5;
            break;
        case 4:
            ret = 7;
            break;
        case 5:
            ret = 8;            // flat 6
            break;
        case 6:
            ret = 10;
            break;
        default:
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInCLydian(int degree)
{
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;
            break;
        case 1:
            ret = 2;
            break;
        case 2:
            ret = 4;
            break;
        case 3:
            ret = 6;        // no 4th, tritone
            break;
        case 4:
            ret = 7;
            break;
        case 5:
            ret = 9;
            break;
        case 6:
            ret = 11;
            break;

    
        default:
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInCMixolydian(int degree)
{
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;
            break;
        case 1:
            ret = 2;
            break;
        case 2:
            ret = 4;
            break;
        case 3:
            ret = 5;
            break;
        case 4:
            ret = 7;
            break;
        case 5: 
            ret = 9;
            break;
        case 6:
            ret = 10;           // falt 7th
            break;
        default:
            assert(false);
    }
    return ret;
}

int DiatonicUtils::getPitchFromScaleDegreeInCLocrian(int degree)
{
    int ret = 0;
    switch (degree) {
        case 0:
            ret = 0;
            break;
        case 1:
            ret = 1;        // flat 2
            break;
        case 2:
            ret = 3;        // flat 3;
            break;
        case 3:
            ret = 5;
            break;
        case 4:
            ret = 6;        // no fifth, tritone
            break;
        case 5:
            ret = 8;
            break;
        case 6:
            ret = 10;
            break;
        default:
            assert(false);
    }
    return ret;
}



int DiatonicUtils::getPitchOffsetRelativeToCMaj(int keyRoot, Modes mode)
{
    int offsetToRelative = getOffsetToRelativeMaj(mode);
    int ret =  offsetToRelative + keyRoot;
    if (ret >= 12) {
        ret -= 12;
    }
    return ret;
}

std::vector<int> DiatonicUtils::getTranspose(int transposeAmount, int keyRoot, Modes mode, bool quantize)
{
    const int offset = getPitchOffsetRelativeToCMaj(keyRoot, mode);
    const std::vector<int> xpose = getTransposeInC(transposeAmount, quantize);
    std::vector<int> ret(12);

    for (int sourceIndex = 0; sourceIndex < 12; ++sourceIndex) {
        int destIndex = sourceIndex + offset;
        if (destIndex > 11) {
            destIndex -= 12;
        }

        ret[destIndex] = xpose[sourceIndex];
    }

    return ret;
}

std::function<void(MidiEventPtr)> DiatonicUtils::makeInvertLambda(
    int invertAxisSemitones, bool constrainToKeysig, int keyRoot, Modes mode)
{
   // printf("making lambda, invert semi = %d constrain = %d\n", invertAxisSemitones, constrainToKeysig);
    if (!constrainToKeysig) {
        const float axis = PitchUtils::semitoneToCV(invertAxisSemitones);
       // printf("in chromatic, axis (cv) = %.2f\n", axis);
        return [axis](MidiEventPtr event) {
            MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
            if (note) {
              //  printf("  note in pitch = %.2f ", note->pitchCV);
                note->pitchCV =  2 * axis - note->pitchCV;
              //  printf("inverted to %.2f\n", note->pitchCV); fflush(stdout);
            }
        };
    } else {
        printf("makeInvertLambda about to call getInvert\n");
        assert(false);
        return[](MidiEventPtr event) {};

#if 0
        auto inverts = getInvert(invertAxisSemitones, keyRoot, mode);
        for (int i = 0; i < 12; ++i) {
            printf("c inverts[%d] = %d\n", i, inverts[i]);
        }
        DiatonicUtils::_dumpTransposes("make lambda, again", inverts);
        fflush(stdout);
        return[inverts, invertAxisSemitones](MidiEventPtr event) {
            MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
            const int axisOctave = normalizePitch(invertAxisSemitones).first;
            if (note) {
                const int semi = PitchUtils::cvToSemitone(note->pitchCV);
                const auto normalizedPitch = normalizePitch(semi);
                const int octaveCorrection = normalizedPitch.first - axisOctave;
               // const int debug = inverts[normalizedPitch.second];
                int invertedSemi = inverts[normalizedPitch.second] + semi;
                note->pitchCV = PitchUtils::semitoneToCV(invertedSemi) - 2 * octaveCorrection;
            }
        };
#endif
    }
}

std::function<void(MidiEventPtr)> DiatonicUtils::makeTransposeLambda(
    int transposeSemitones, bool constrainToKeysig, int keyRoot, Modes mode)
{
//    MidiEventPtr note = safe_cast<MidiNoteEvent>(event);
    if (!constrainToKeysig) {
        const float delta = transposeSemitones * PitchUtils::semitone;
        return [delta](MidiEventPtr event) {
            MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
            if (note) {
                note->pitchCV += delta;
            }
        };
    } else {
        // for now, always make the non-quantized version (since it works)
        auto xposes = getTranspose(transposeSemitones, keyRoot, mode, false);
        return[xposes](MidiEventPtr event)
        {
            MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
            if (note) {
                const int semi = PitchUtils::cvToSemitone(note->pitchCV);
                const auto normalizedPitch = normalizePitch(semi);
                const int xposedSemi = xposes[normalizedPitch.second] + semi;

                note->pitchCV = PitchUtils::semitoneToCV(xposedSemi);
            }
        };
    }
}

int DiatonicUtils::quantizeXposeToScaleDegreeInC(int xpose)
{
    int ret = 0;
    assert(xpose >= 0);
    assert(xpose <= 11);
    switch (xpose) {
        case 0:
            ret = 0;
            break;
        case 1:
        case 2:
            ret = 1;
            break;
        case 3:
        case 4:
            ret = 2;
            break;
        case 5:
            ret = 3;
            break;
        case 6:         // I know - a tritone is not a 5th, but??
        case 7:
            ret = 4;
            break;
        case 8:
        case 9:
            ret = 5;
            break;
        case 10:
        case 11:
            ret = 6;
            break;
        default:
            assert(false);

    }
    return ret;
}


int DiatonicUtils::getScaleDegreeInC(int _pitch)
{
    // let it work from 0.. two octaves
    assert(_pitch >= 0);    // never considered neg

  //  if (pitch > 11) {
  //      return 7 + getScaleDegreeInC(pitch - 12);
  //  }
    const int normalizedPitch = normalizePitch(_pitch).second;
    int ret = 0;
    switch (normalizedPitch) {
        case 0:     //c
            ret = 0;
            break;
        case 1:     // c#
            assert(false);
            break;
        case 2:     // D
            ret = 1;
            break;
        case 3:     // D#
            assert(false);
            break;
        case 4:     // E
            ret = 2;
            break;
        case 5:     // F
            ret = 3;
            break;
        case 6:     // F#
            assert(false);
            break;
        case 7:      // G
            ret = 4;
            break;
        case 8:     // G#
            assert(false);
            break;
        case 9:     // A
            ret = 5;
            break;
        case 10: // A#
            assert(false);
            break;
        case 11: // B
            ret = 6;
            break;
        default:
            assert(false);
    }
    return ret;
}

int normalize(int input, int end)
{
    int ret = input;
    while (ret < 0) {
        ret += end;
    }
    while (ret >= end) {
        ret -= end;
    }
    assert(ret >= 0 && ret < end);
    return ret;
}

/*

  Major,
        Dorian,
        Phrygian,
        Lydian,
        Mixolydian,
        Minor,
        Locrian*/

int DiatonicUtils::getPitchFromScaleDegree(int degree, int key, Modes mode)
{
    int shiftedPitch = 0;
    switch (mode) {
        case Modes::Major:
            shiftedPitch = getPitchFromScaleDegreeInC(degree);
            break;
        case Modes::Dorian:
            shiftedPitch = getPitchFromScaleDegreeInCDorian(degree);
            break;
        case Modes::Lydian:
            shiftedPitch = getPitchFromScaleDegreeInCLydian(degree);
            break;
        case Modes::Mixolydian:
            shiftedPitch = getPitchFromScaleDegreeInCMixolydian(degree);
            break;
        case Modes::Minor:
            shiftedPitch = getPitchFromScaleDegreeInCMinor(degree);
            break;
        case Modes::Locrian:
            shiftedPitch = getPitchFromScaleDegreeInCLocrian(degree);
            break;
        case Modes::Phrygian:
            shiftedPitch = getPitchFromScaleDegreeInCPhrygian(degree);
            break;
        default:
            assert(false);
    }
    const int pitch = normalize(shiftedPitch + key, 12);
    return pitch;
}







// Pitch to degree:
// offset pitch to C
// get the degree in c
// shift degree by scaleDegree Offset
int DiatonicUtils::getScaleDegree(int pitch, int key, Modes mode)
{
    // offset due to mode note being C
    const int modeOffset = getOffsetToRelativeMaj(mode);

    // extra offset for root C not being the relative major of 'mode'
    const int extraOffset = (normalize(modeOffset + key, 12));


    // modeOffset is semitones we need shift to get to cmaj
    // well, what if we go down by modeOffset from C? That would be degees shift to get to scale
    const int offsetToOtherFromC = normalize(-modeOffset, 12);
    const int scaleDegreeOffset = getScaleDegreeInC(offsetToOtherFromC);

    // from simple major one
    // in that one, offsetToC was -key
    const int offsetToC = -extraOffset;
    const int pitchInC = normalize(pitch + offsetToC, 12);

    if (!isNoteInC(pitchInC)) {
        return -1;
    }
    const int scaleDegreeInC = getScaleDegreeInC(pitchInC);

    int degree = scaleDegreeInC - scaleDegreeOffset;

    degree = normalize(degree, 7);
    return degree;
}

// try to invert getScaleDegree
// first shift by scaledegree offset
#if 0
int DiatonicUtils::getPitchFromScaleDegree(int degree, int key, Modes mode)
{
    // this all bs
    // Step 1: computer scaleDegree Offset
        // offset due to mode note being C
    const int modeOffset = getOffsetToRelativeMaj(mode);

    // extra offset for root C not being the relative major of 'mode'
    const int extraOffset = (normalize(modeOffset + key, 12));
    const int offsetToC = -extraOffset;


    // modeOffset is semitones we need shift to get to cmaj
    // well, what if we go down by modeOffset from C? That would be degees shift to get to scale
    const int offsetToOtherFromC = normalize(-modeOffset, 12);
    const int scaleDegreeOffset = getScaleDegreeInC(offsetToOtherFromC);

    // WE aren't using extra offset anywhere
    //

    // this worked for simple cases
   // int scaleDegreeInC = degree - scaleDegreeOffset;
    // this is totally wrong, can't invert degee. maybe add offet?
   // int scaleDegreeInC = scaleDegreeOffset - degree;

    int scaleDegreeInC = degree + scaleDegreeOffset;
    scaleDegreeInC = normalize(scaleDegreeInC, 7);
   // assert(scaleDegreeInC >= 0);

    const int pitchInC = getPitchFromScaleDegreeInC(scaleDegreeInC);

    const int pitch = pitchInC - offsetToC;

    return pitch;

}
#endif

// work in relative modes from c maj
// a) we need to know how many degrees we need to shift to get to CMaj
// then we fine degree of (pitch) in cmaj, and shift it back down by a
#if 0
int DiatonicUtils::getScaleDegree(int pitch, int key, Modes mode)
{
    const int modeOffset = getOffsetToRelativeMaj(mode);
    assertEQ (normalize(modeOffset + key, 12), 0);

    // modeOffset is semitones we need shift to get to cmaj
    // what is the scale degrees?
    // well, what if we go down by modeOffset from C? That would be degees shift to get to scale
    const int offsetToOtherFromC = normalize(-modeOffset, 12);
    const int scaleDegreeOffset = getScaleDegreeInC(offsetToOtherFromC);

    const int scaleDegreeInC = getScaleDegreeInC(pitch);    // it better be in C, but it will, since it's a relative
    int degree = scaleDegreeInC - scaleDegreeOffset;
    //assert(degree >= 0);
    degree = normalize(degree, 7);
    return degree;
}
#endif

// this one works in major keys
#if 0
int DiatonicUtils::getScaleDegree(int pitch, int key, Modes mode)
{
    assert(mode == Modes::Major);

    // how much we need to move up to get into Major
    //const int modeOffset = getOffsetToRelativeMaj(mode);


    // if in mode, move the pitch into cmaj (by adding mode offset),
    // find degree of (note + mode offset) in C,
    // that's it

    // how much we need to shift to get to C
    const int offsetToC = -key;

    const int pitchInC = normalize(pitch + offsetToC, 12);
    const int ret = getScaleDegreeInC(pitchInC);
    return ret;
}
#endif

// old attempts

#if 0
int DiatonicUtils::getScaleDegree(int pitch, int key, Modes mode)
{
    const int modeOffset = getOffsetToRelativeMaj(mode);

    // how much you need to offset pitch to get it into c
    const int rootOffset = key;
    const int totalOffset = modeOffset + rootOffset;
    int offsetPitch = pitch +totalOffset;
    while (offsetPitch > 11) {
        offsetPitch -= 12;
    }

    const int degreesInOffset = getScaleDegreeInC(totalOffset);

    const int offsetDegree = getScaleDegreeInC(offsetPitch);
    const int finalDegree = offsetDegree - degreesInOffset;

 //   int rootOffset = key;
    return getScaleDegreeInC(pitch - rootOffset);
}
#endif

#if 0
int DiatonicUtils::getScaleDegree(int pitch, int key, Modes mode)
{
    const int modeOffset = getOffsetToRelativeMaj(mode);
    const int rootOffset = key;
    const int offsetPitch = pitch + offset;
    const int degreesInOffset = getScaleDegreeInC(offset);

    const int offsetDegree = getScaleDegreeInC(offsetPitch);
    const int finalDegree = offsetDegree - degreesInOffset;

    int rootOffset = key;
    return getScaleDegreeInC(pitch - rootOffset);
}
#endif


