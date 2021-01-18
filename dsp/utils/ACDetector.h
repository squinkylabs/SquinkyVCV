#pragma once

#include <assert.h>

#include "SqPort.h"

class ACDetector {
public:
    bool step(SqInput& input, int numChannels);

private:
    // let's turn on at 200 hz
    const int thresholdPeriod = 44100 / 200;
    int counter = 0;
    bool lastValue = false;
    bool isACInput = false;
};

inline bool ACDetector::step(SqInput& input, int numChannels) {
    assert(numChannels == 1);  // todo: implement poly
    bool detector = false;
    if (!input.isConnected()) {
        return false;
    }

    bool inputV = input.getVoltage(0) > .5;  // todo: schmidt
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