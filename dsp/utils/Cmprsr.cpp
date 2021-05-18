
#include "Cmprsr.h"
/**
 * This static needs somewhere to live. 
 * So I put him here.
 */


CompCurves::CompCurveLookupPtr Cmprsr::ratioCurves2[int(Ratios::NUM_RATIOS)];
CompCurves::LookupPtr Cmprsr::ratioCurves[int(Ratios::NUM_RATIOS)];


void Cmprsr::_reset() {
    for (int i = 0; i < int(Ratios::NUM_RATIOS); ++i) {
        ratioCurves2[i].reset();
    }
}