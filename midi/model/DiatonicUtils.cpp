#include "DiatonicUtils.h"
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

#if 0
void DiatonicUtils::_dump(const char* msg, const std::vector<int>& data)
{
    printf("dump: %s\n", msg);
    for (size_t i = 0; i < data.size(); ++i) {
        printf("[%s]=%s\n", getPitchString(int(i)).c_str(), getPitchString(data[i]).c_str());
    }
    printf("\n");
}
#endif


std::vector<int> DiatonicUtils::getTransposedPitchesInC(const std::vector<int>& transposes)
{
   // printf("in getTransposedPitchesInC\n");
    std::vector<int> ret(transposes.size());
    for (int i = 0; i < (int) transposes.size(); ++i) {
        const int j = transposes[i];
        ret[i] = (j < -12) ? j : i + transposes[i];
       // printf("in get transp, %d -> %d\n", i, ret[i]);
    }

   // for (int i = 0; i < (int) transposes.size(); ++i) {
   //     printf("in getTransposedPitchesInC i=%d trans=%d final=%d\n", i, transposes[i], ret[i]);
   // }
    return ret;
}

void DiatonicUtils::_dumpTransposes(const char* msg, const std::vector<int>& transposes)
{
    printf("\n*****dump1: %s\n", msg);
  
    std::vector<int> pitches = getTransposedPitchesInC(transposes);
  //  printf("dump2: %s\n", msg);
 //   for (size_t i = 0; i < transposes.size(); ++i) {
 //       printf("transposed pitches[%d] = %d\n", i, pitches[i]);
 //   }
 
   
    printf("dump3: %s\n", msg);
    for (size_t i = 0; i < transposes.size(); ++i) {
        printf("t[%zd]=%d.  %s => %s\n", 
            i, transposes[i],
            getPitchString(int(i)).c_str(),
            getPitchString(pitches[i]).c_str());
    }
    printf("\n");
}

std::vector<int> DiatonicUtils::getTransposeInC(int transposeAmount)
{
    std::vector<int> ret(12);
   
    // init to absurd value
    for (int i = 0; i < 12; ++i) {
        ret[i] = -24;                    
    }

    _dumpTransposes("init", ret);
    int lastScaleTone = -1;

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
            ret[i] = transposeAmount;
            printf("setting ret %d to %d\n", i, transposeAmount);
            assert(ret[i] >= transposeAmount - 1);
            assert(ret[i] <= transposeAmount + 1);
          
        }

        // if we were in the key, and chormatic xpose takes us out,
        // Then make sure we don't grab the same scale tone as a different
        // step. 
        // i.e. two separate scale tones must always xpose to different scale tones.
        if (isInC && !xposeInC) {
            int guessPitch = i + chromaticTransposePitch - 1;
            if (lastScaleTone >= 0) {
                if (guessPitch > lastScaleTone) {
                    ret[i] = transposeAmount - 1;
                } else {
                    ret[i] = transposeAmount + 1;
                }
            } else {
                // there was no previous pitch, to just go down
                ret[i] = transposeAmount - 1;
            }
            assert(DiatonicUtils::isNoteInC(i + ret[i])); 

            assert(ret[i] >= transposeAmount - 1);
            assert(ret[i] <= transposeAmount + 1);
        }
        if (isInC) {
            lastScaleTone = i + ret[i];
        }
    }


    _dumpTransposes("step 1", ret);


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
            assert(ret[i] < -12);                 // we haven't filled these in yet
        }

         // if chromatic xpose keeps in key, use that (for now)
        if (!isInC && xposeInC) {
            ret[i] = transposeAmount;

            assert(ret[i] >= transposeAmount - 1);
            assert(ret[i] <= transposeAmount + 1);
        }
        if (!isInC && !xposeInC) {
            assert(i > 0);

            // let's just go to the same pitch as prev guy (won't always work);
            const int prevXpose = ret[i - 1];
            const int prevPitch = prevXpose + (i-1);
            const int thisPitch = prevPitch;
            const int thisXpose = thisPitch - i;
            ret[i] = thisXpose;

            assert(ret[i] >= transposeAmount - 1);
            assert(ret[i] <= transposeAmount + 1);
        }
       
    }

    _dumpTransposes("final", ret);
    return ret;
}

