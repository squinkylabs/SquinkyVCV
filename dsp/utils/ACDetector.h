#pragma once

#include <assert.h>

#include "LowpassFilter.h"
#include "SchmidtTrigger.h"

class ACDetector {
public:
    ACDetector();
    ACDetector(const ACDetector&) = delete;

    bool step(float combinedInput);
    const float threshold = .1f;

    // let's turn off at 200 Hz.
    const int thresholdPeriod = 44100 / 200;

private:
    int counter = 0;
    bool lastValue = false;
    bool isACInput = false;

    SchmidtTrigger trig;
    LowpassFilterParams<float> dcBlockParams;
    LowpassFilterState<float> dcBlockState;
};

inline ACDetector::ACDetector() : trig(-threshold, threshold) {
    LowpassFilter<float>::setCutoff(dcBlockParams, 1.f / thresholdPeriod);
}

inline bool ACDetector::step(float _combinedInput) {
    //bool detector = false;

    const float lpRout =  LowpassFilter<float>::run(_combinedInput, dcBlockState, dcBlockParams); 
    const float combinedInput = _combinedInput - lpRout;

    //  bool inputV = input.getVoltage(0) > .5;  // todo: schmidt
    bool inputV = trig.go(combinedInput);
    bool change = inputV != lastValue;
    lastValue = inputV;
    if (!isACInput) {
        // if we are inactive, and we see a transion,
        // go active immediately.
        isACInput = change;

        counter = 0;
    } else if (change) {
        // we active, but we got a change
        isACInput = true;
        counter = 0;
    } else {
        // are active, but no input
        counter++;
        if (counter > thresholdPeriod) {
            isACInput = false;
            counter = 0;
        }
    }

    return isACInput;
}