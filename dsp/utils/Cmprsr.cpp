
#include "Cmprsr.h"
/**
 * This static needs somewhere to live. 
 * So I put him here.
 */


#ifdef _USESPLINE
CompCurves::CompCurveLookupPtr Cmprsr::ratioCurves2[int(Ratios::NUM_RATIOS)];
#else
CompCurves::LookupPtr Cmprsr::ratioCurves[int(Ratios::NUM_RATIOS)];
#endif


void Cmprsr::_reset() {
    for (int i = 0; i < int(Ratios::NUM_RATIOS); ++i) {

#ifdef _USESPLINE
        ratioCurves2[i].reset();
#else   
        a b c
#endif
    }
}