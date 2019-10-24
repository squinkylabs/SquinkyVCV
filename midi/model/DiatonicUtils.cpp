#include "DiatonicUtils.h"
#include <assert.h>


bool DiatonicUtils::isNoteInC(int pitch)
{
    bool ret = true;
    assert(pitch >= DiatonicUtils::c);
    assert(pitch <= DiatonicUtils::b);
    switch (pitch) {
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

std::string DiatonicUtils::getPitchString(int pitch)
{
    if (pitch < 0) {
        return ("x");
    }
    assert(pitch < 12);
    return pitchNames[pitch];
}

void DiatonicUtils::_dump(const char* msg, const std::vector<int>& data)
{
    printf("dump: %s\n", msg);
    for (size_t i = 0; i < data.size(); ++i) {
        printf("[%s]=%s\n", getPitchString(int(i)).c_str(), getPitchString(data[i]).c_str());
    }
    printf("\n");
}

std::vector<int> DiatonicUtils::getTransposeInC(int transposeAmount)
{
    std::vector<int> ret(12);
    for (int i = 0; i < 12; ++i) {
        ret[i] = -1;                    // init to absurd value
    }

    int lastScaleToneXpose = -1;

   
    // first do all the ones that are already in key
    for (int i = 0; i < 12; ++i) {
        bool chromaticXposeWrapsPitch = false;
        int chromaticTransposePitch = i + transposeAmount;
        if (chromaticTransposePitch > DiatonicUtils::b) {
            chromaticTransposePitch -= 12;
            chromaticXposeWrapsPitch = true;
        }

        const bool isInC = DiatonicUtils::isNoteInC(i);
        const bool xposeInC = DiatonicUtils::isNoteInC(chromaticTransposePitch);

        // if chromatic xpose keeps in key, use that.
        if (isInC && xposeInC) {
            ret[i] = chromaticTransposePitch;
          
        }

        if (isInC && !xposeInC) {
            int guess = chromaticTransposePitch - 1;
            assert(lastScaleToneXpose >= 0);
            if (guess > lastScaleToneXpose) {
                ret[i] = guess;
            } else {
                ret[i] = chromaticTransposePitch + 1;
            }
            assert(DiatonicUtils::isNoteInC(ret[i])); 
        }
        if (isInC) {
            lastScaleToneXpose = ret[i];
        }
    }

    _dump("step 1", ret);

    // now do all the ones that are not in key
    for (int i = 0; i < 12; ++i) {
        bool chromaticXposeWrapsPitch = false;
        int chromaticTransposePitch = i + transposeAmount;
        if (chromaticTransposePitch > DiatonicUtils::b) {
            chromaticTransposePitch -= 12;
            chromaticXposeWrapsPitch = true;
        }

        const bool isInC = DiatonicUtils::isNoteInC(i);
        const bool xposeInC = DiatonicUtils::isNoteInC(chromaticTransposePitch);

        if (!isInC) {
            assert(ret[i] < 0);                 // we haven't filled these in yet
        }

         // if chromatic xpose keeps in key, use that (for now)
        if (!isInC && xposeInC) {
            ret[i] = chromaticTransposePitch;
        }
        if (!isInC && !xposeInC) {
            assert(i > 0);

            // let's just go to the same pitch as prev guy (won't always work);
            int prevXpose = ret[i - 1];
            int thisXpose = prevXpose;
            ret[i] = thisXpose;
        }
    }



    _dump("final", ret);
    return ret;
}

