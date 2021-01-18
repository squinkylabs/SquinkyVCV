#pragma once

#include <assert.h>

#include "LowpassFilter.h"
#include "SchmidtTrigger.h"


class ACDetector {
public:
    ACDetector() : trig(.1f, .1f) {}
   // bool step(SqInput& input, int numChannels);
    bool step(float combinedInput);

private:
    // let's turn on at 200 hz
    const int thresholdPeriod = 44100 / 200;
    int counter = 0;
    bool lastValue = false;
    bool isACInput = false;

    SchmidtTrigger trig;
    LowpassFilterParams<float> dcBlockParams;
};

inline bool ACDetector::step(float combinedInput) {

    //bool detector = false;



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
        isACInput = change;
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