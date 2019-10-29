#include "DiatonicUtils.h"
#include "SqMidiEvent.h"
#include "PitchUtils.h"
#include <assert.h>
#include <sstream>


std::pair<int, int> DiatonicUtils::normalizePitch(int pitch)
{
    int octave = pitch / 12;
    pitch -= octave * 12;
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
    if (_pitch < 0) {
       // printf("convert below zero: %d\n", pitch);
        return ("x");
    }
   
   // return pitchNames[pitch];
    std::stringstream s;
    s << pitch.first << ":" << pitchNames[pitch.second];
    return s.str();
}

std::vector<int> DiatonicUtils::getTransposedPitchesInC(const std::vector<int>& transposes)
{
   // printf("in getTransposedPitchesInC\n");
    std::vector<int> ret(transposes.size());
    for (int i = 0; i < (int) transposes.size(); ++i) {
        const int j = transposes[i];
        ret[i] = (j < -12) ? j : i + transposes[i];
    }

    return ret;
}

void DiatonicUtils::_dumpTransposes(const char* msg, const std::vector<int>& transposes)
{
    printf("\n*****dump1: %s\n", msg);
  
    std::vector<int> pitches = getTransposedPitchesInC(transposes);
   
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



std::vector<int> DiatonicUtils::getInvertInCInformed(int invertAxis)
{
    std::vector<int> ret(12);


 // initialize to absurd value
    for (int i = 0; i < 12; ++i) {
        ret[i] = -1000;
    }

    auto normalizedAxis = normalizePitch(invertAxis);
    const int axisSemis = normalizedAxis.second;
    const int axisOctaves = normalizedAxis.first;

    assert(axisSemis >= 0);
    assert(axisSemis < 12);       // callers should normalize out the octaves. Or we should support it?

    const int scaleDegreesOfAxis = quantizeXposeToScaleDegreeInC(axisSemis);

    // First do the notes in C
    for (int i = 0; i < 12; ++i) {
        const bool isInC = DiatonicUtils::isNoteInC(i);
        if (isInC) {
            const int initialScaleDegree = getScaleDegreeInC(i);
            const int scaleDegreeAfterInvert = scaleDegreesOfAxis - initialScaleDegree;
            const int pitchAfterInvert = getPitchFromScaleDegree(scaleDegreeAfterInvert);

            const int delta = pitchAfterInvert - i;
            ret[i] = delta;
        }
    }


    return ret;

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

   //  _dumpTransposes("init", ret);
  //  int lastScaleTone = -1;

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
            const int pitchAfterQuantize = getPitchFromScaleDegree(scaleDegreeAfterQuantize);
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
            const int pitchAfterQuantize = getPitchFromScaleDegree(scaleDegreeAfterQuantize);

            if (pitchAfterQuantize > 11) {
                auto norm = normalizePitch(pitchAfterQuantize);
            }
            const int delta = pitchAfterQuantize - i;
            ret[i] = delta;
        }
    }

    //_dumpTransposes("step 1", ret);

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

    //_dumpTransposes("final", ret);
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

int DiatonicUtils::getPitchFromScaleDegree(int degree)
{
    if (degree > 6) {
        const int x = getPitchFromScaleDegree(degree - 7);
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
    for (int i = 0; i <= 11; ++i) {
        int pitchInRelMajor = i + offset;
        auto norm = normalizePitch(pitchInRelMajor);
        int normP = norm.second;
        int xposed = xpose[normP];
        int final = xposed - offset;
        ret[i] = final;
    }
    return ret;
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
        // for now, always make the quantized version (since it works)
        auto xposes = getTranspose(transposeSemitones, keyRoot, mode, true);
        return[xposes](MidiEventPtr event)
        {
            MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
            if (note) {
                int semi = PitchUtils::cvToSemitone(note->pitchCV);
                auto normalizedPitch = normalizePitch(semi);
                const int debug = xposes[normalizedPitch.second];
                int xposedSemi = xposes[normalizedPitch.second] + semi;

                note->pitchCV = PitchUtils::semitoneToCV(xposedSemi);
            }
        };
    }
#if 0
        auto xposes =  getTranspose(transposeSemitones, keyRoot, mode);
        _dumpTransposes("making lambda", xposes);
        return [xposes](float input) {
            int semi = PitchUtils::cvToSemitone(input);
            auto normalizedPitch = normalizePitch(semi);
            const int debug = xposes[normalizedPitch.second];
            int xposedSemi = xposes[normalizedPitch.second] + semi;

            return PitchUtils::semitoneToCV(xposedSemi);
        };
    }
#endif
}

std::function<void(MidiEventPtr)> DiatonicUtils::makeInvertLambda(
    int invertAxisSemitones, bool constrainToKeysig, int keyRoot, Modes mode)
{
    assert(false);
    return[](MidiEventPtr event) {
        assert(false);
    };
   
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


