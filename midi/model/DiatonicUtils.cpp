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

std::vector<int> DiatonicUtils::getTransposeInC(int transposeAmount)
{
    std::vector<int> ret(12);
    for (int i = 0; i < 12; ++i) {
        ret[i] = -1;                    // init to absurd value
    }

    // first do all the ones that are already in key
    for (int i = 0; i < 12; ++i) {
        const bool isInC = DiatonicUtils::isNoteInC(i);
        const bool xposeInC = DiatonicUtils::isNoteInC(i + transposeAmount);

        // if chromatic xpose keeps in key, use that.
        if (isInC && xposeInC) {
            ret[i] = i + transposeAmount;
        }

        if (isInC && !xposeInC) {
            assert(false);      // can't handle yet
        }
    }

    // now do all the ones that are not in key
    for (int i = 0; i < 12; ++i) {
        const bool isInC = DiatonicUtils::isNoteInC(i);
        const bool xposeInC = DiatonicUtils::isNoteInC(i + transposeAmount);

        if (!isInC) {
            assert(ret[i] < 0);                 // we haven't filled these in yet
        }

         // if chromatic xpose keeps in key, use that (for now)
        if (!isInC && xposeInC) {
            ret[i] = i + transposeAmount;       
        }
        if (!isInC && !xposeInC) {
            assert(i > 0);

            // let's just go to the same pitch as prev guy (won't always work);
            int prevXpose = ret[i - 1];
            int thisXpose = prevXpose;
            ret[i] = thisXpose;
        }
    }



    _dump("step 1", ret);
    return ret;
}

void DiatonicUtils::_dump(const char* msg, const std::vector<int>& data)
{
    printf("dump: %s\n", msg);
    for (size_t i = 0; i < data.size(); ++i) {
        printf("[%zd]=%d, ", i, data[i]);
    }
    printf("\n");
}