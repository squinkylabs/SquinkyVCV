#include "DiatonicUtils.h"


bool DiatonicUtils::isNoteInC(int pitch)
{
    bool ret = true;
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